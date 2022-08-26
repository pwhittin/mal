(ns mal.eval
  (:require [clojure.pprint]
            [mal.env :as en]
            [mal.printer :as p]))

(defn env->symbols [env]
  (loop [e env
         symbols []]
    (if (nil? e)
      symbols
      (recur (:outer @e) (conj symbols (map second (keys (:data @e))))))))

(declare eval)

(defn add-bindings! [env bindings]
  (doseq [[symbol value] (partition 2 bindings)]
    (en/set env symbol (eval value env))))

(defn eval-sequence [mal-type mal-value env]
  [mal-type (map #(eval % env) mal-value)])

(defn eval-ast [[mal-type mal-value :as mal] env]
  (case mal-type
    :mal-symbol (en/get env mal)
    :mal-list (eval-sequence :mal-list mal-value env)
    :mal-map (eval-sequence :mal-map mal-value env)
    :mal-vector (eval-sequence :mal-vector mal-value env)
    mal))

(defn eval-mal-def!-validate! [[[symbol-type symbol-value :as symbol] value & others :as args]]
  (when (zero? (count args))
    (throw (Exception. "def!: wrong number of args (0)")))
  (when (not= :mal-symbol symbol-type)
    (throw (Exception. (str "def!: first arg is not a symbol: " (p/print-str symbol)))))
  (when (not value)
    (throw (Exception. (str "def!: '" (p/print-str symbol) "' wrong number of args (1)"))))
  (when (seq others)
    (throw (Exception. (str "def!: '" (p/print-str symbol) "' wrong number of args (" (count args) "): "
                            (pr-str (mapv p/print-str args)))))))

(defn eval-mal-def! [[[symbol-type symbol-value :as symbol] value & others :as args] env]
  (eval-mal-def!-validate! args)
  (let [v (eval value env)]
    (en/set env symbol v)
    [v nil]))

(defn eval-mal-do [forms env]
  (loop [fs (drop-last forms)]
    (if (empty? fs)
      [(last forms) env]
      (do
        (eval (first fs) env)
        (recur (rest fs))))))

(defn params-amper-parse [params]
  (let [amper-params (drop-while #(not= [:mal-symbol "&"] %) params)]
    [(drop-last (count amper-params) params) (drop 1 amper-params)]))

(defn eval-mal-fn*-validate! [[params-type params-value :as params]]
  (when (not params)
    (throw (Exception. (str "fn*: wrong number of args (0)"))))
  (when (not (#{:mal-list :mal-vector} params-type))
    (throw (Exception. (str "fn*: params must be a list or vector"))))
  (when (some (fn [[type _]] (not= :mal-symbol type)) params-value)
    (throw (Exception. (str "fn*: non-symbol param: " (pr-str (mapv p/print-str params-value))))))
  (let [[_ amper-params :as amper-parse] (params-amper-parse params-value)]
    (when (> (count amper-params) 1)
      (throw (Exception. (str "fn*: only one param allowed after '&': " (pr-str (mapv p/print-str params-value))))))
    amper-parse))

(defn eval-mal-fn-validate! [args [regular-params amper-params]]
  (when (< (count args) (count regular-params))
    (throw (Exception. (str "fn: wrong number of args (< " (count args) " " (count regular-params) "): "
                            (p/print-str [:mal-list args])))))
  (let [regular-args (take (count regular-params) args)
        regular-bindings (interleave regular-params regular-args)]
    (if (empty? amper-params)
      regular-bindings
      (let [amper-args (drop (count regular-args) args)
            amper-args-list [:mal-list (concat [[:mal-symbol "list"]] amper-args)]
            amper-symbol (first amper-params)]
        (concat regular-bindings [amper-symbol amper-args-list])))))

(defn fn*-params->fn-args-env->env [params env]
  (fn [args]
    (let [bindings (eval-mal-fn-validate! args params)
          e (en/create env)]
      (add-bindings! e bindings)
      e)))

(defn eval-mal-fn* [[params form] env]
  (let [fn-params (eval-mal-fn*-validate! params)
        fn-args-env->env (fn*-params->fn-args-env->env fn-params env)]
    [[:mal-fn* [form fn-args-env->env]] env]))

(defn eval-mal-if-validate! [[predicate form-true form-false & others :as forms]]
  (when (not predicate)
    (throw (Exception. (str "if: wrong number of args (0)"))))
  (when (not form-true)
    (throw (Exception. (str "if: wrong number of args (1) " (pr-str (mapv p/print-str forms))))))
  (when others
    (throw (Exception. (str "if: wrong number of args (" (+ 3 (count others)) ") " (pr-str (mapv p/print-str forms)))))))

(defn eval-mal-if [[predicate form-true form-false :as forms] env]
  (eval-mal-if-validate! forms)
  (let [[predicate-type _] (eval predicate env)]
    (if (not (#{:mal-false :mal-nil} predicate-type))
      [form-true env]
      (if form-false
        [form-false env]
        [[:mal-nil nil] nil]))))

(defn eval-mal-let*-validate! [[[sequence-type sequence-value :as sequence] value & others :as args]]
  (when (zero? (count args))
    (throw (Exception. "let*: wrong number of args (0)")))
  (when (not (#{:mal-list :mal-vector} sequence-type))
    (throw (Exception. (str "let*: first arg is not a list or vector: " (p/print-str sequence)))))
  (when (not value)
    (throw (Exception. (str "let*: wrong number of args (1)"))))
  (when (seq others)
    (throw (Exception. (str "let*: wrong number of args (" (count args) "): " (pr-str (mapv p/print-str args))))))
  (when (odd? (count sequence-value))
    (throw (Exception. (str "let*: bindings sequence odd forms count (" (count sequence-value) "): "
                            (pr-str (mapv p/print-str sequence-value))))))
  (when (some (fn [[type _]] (not= :mal-symbol type)) (take-nth 2 sequence-value))
    (throw (Exception. (str "let*: non-symbol binding: " (pr-str (mapv p/print-str sequence-value)))))))

(defn eval-mal-let* [[[sequence-type sequence-value :as sequence] value & others :as args] env]
  (eval-mal-let*-validate! args)
  (let [let-env (en/create env)]
    (add-bindings! let-env sequence-value)
    [value let-env]))

(declare eval-mal-quasiquote)

(defn eval-mal-quasiquote-reduce-fn [[mal-list env] [elt-type [elt-1 elt-2 & _] :as elt]]
  (let [is-splice-unquote? (and (= :mal-list elt-type) (= [:mal-symbol "splice-unquote"] elt-1))]
    (if is-splice-unquote?
      [[:mal-list [[:mal-symbol "concat"] elt-2 mal-list]] env]
      (let [[eval-elt-value _] (eval-mal-quasiquote [elt] env)]
        [[:mal-list [[:mal-symbol "cons"] eval-elt-value mal-list]] env]))))

(defn splice-unquote-list [mals env]
  (reduce eval-mal-quasiquote-reduce-fn [[:mal-list []] env] (reverse mals)))

(defn quasiquote-list [[mal-1 mal-2 & _ :as mals] env]
  (if (= [:mal-symbol "unquote"] mal-1)
    [mal-2 env]
    (splice-unquote-list mals env)))

(defn eval-mal-quasiquote [[[mal-type mal-value :as mal] & others :as args] env]
  (when (zero? (count args))
    (throw (Exception. "quasiquote: wrong number of args (0)")))
  (when (seq others)
    (throw (Exception. (str "quasiquote: wrong number of args (" (count args) "): " (pr-str (mapv p/print-str args))))))
  (cond
    (= :mal-list mal-type) (quasiquote-list mal-value env)
    (#{:mal-map :mal-symbol} mal-type) [[:mal-list [[:mal-symbol "quote"] mal]] nil]
    :else [mal nil]))

(defn eval-mal-quote [[mal & others :as args]]
  (when (seq others)
    (throw (Exception. (str "quote: wrong number of args (" (count args) "): " (pr-str (mapv p/print-str args))))))
  [mal nil])

(defn eval-fn [mal env]
  (let [[_ [[fn-type fn-value] & args]] (eval-ast mal env)]
    (when (not (#{:mal-fn :mal-fn*} fn-type))
      (throw (Exception. (str "attempted call of non-function '" (pr-str fn-value) "'"))))
    (if (= :mal-fn fn-type)
      [(fn-value args) nil]
      (let [[form args-env->env] fn-value
            new-env (args-env->env args)]
        [form new-env]))))

(def mal-def! [:mal-symbol "def!"])
(def mal-do [:mal-symbol "do"])
(def mal-fn* [:mal-symbol "fn*"])
(def mal-if [:mal-symbol "if"])
(def mal-let* [:mal-symbol "let*"])
(def mal-quasiquote [:mal-symbol "quasiquote"])
(def mal-quote [:mal-symbol "quote"])

(defn eval-list [[_ [mal-list-fn & mal-list-args] :as mal-list] env]
  (condp = mal-list-fn
    mal-def! (eval-mal-def! mal-list-args env)
    mal-do (eval-mal-do mal-list-args env)
    mal-fn* (eval-mal-fn* mal-list-args env)
    mal-if (eval-mal-if mal-list-args env)
    mal-let* (eval-mal-let* mal-list-args env)
    mal-quasiquote (eval-mal-quasiquote mal-list-args env)
    mal-quote (eval-mal-quote mal-list-args)
    (eval-fn mal-list env)))

(defn eval [mal env]
  (loop [[mal-type mal-value :as m] mal
         e env]
    (if (not= :mal-list mal-type)
      (eval-ast m e)
      (if (empty? mal-value)
        m
        (let [[new-mal new-env] (eval-list m e)]
          (if (not new-env)
            new-mal
            (recur new-mal new-env)))))))
