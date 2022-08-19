(ns mal.eval
  (:require [mal.env :as en]
            [mal.printer :as p]))

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
    v))

(defn eval-mal-do [forms env]
  (loop [fs forms
         answer nil]
    (if (empty? fs)
      answer
      (recur (rest fs) (eval (first fs) env)))))

(defn eval-mal-fn*-validate! [[params-type params-value :as params]]
  (when (not params)
    (throw (Exception. (str "fn*: wrong number of args (0)"))))
  (when (not (#{:mal-list :mal-vector} params-type))
    (throw (Exception. (str "fn*: params must be a list or vector"))))
  (when (some (fn [[type _]] (not= :mal-symbol type)) params-value)
    (throw (Exception. (str "fn*: non-symbol param: " (pr-str (mapv p/print-str params-value)))))))

(defn eval-mal-fn-validate! [args [_ params-value :as params]]
  (when (not= (count args) (count params-value))
    (throw (Exception. (str "fn: wrong number of args (not= " (count args) " " (count params-value) "): "
                            (p/print-str [:mal-list args]))))))

(defn eval-mal-fn* [[[_ params-value :as params] form] env]
  (eval-mal-fn*-validate! params)
  [:mal-fn (fn [args]
             (eval-mal-fn-validate! args params)
             (let [e (en/create env)
                   bindings (interleave params-value args)]
               (add-bindings! e bindings)
               (eval form e)))])

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
      (eval form-true env)
      (if form-false
        (eval form-false env)
        [:mal-nil nil]))))

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
    (eval value let-env)))

(defn eval-fn [mal env]
  (let [[_ [[fn-type fn-value] & args]] (eval-ast mal env)]
    (when (not= :mal-fn fn-type)
      (throw (Exception. (str "attempted call of non-function '" (pr-str fn-value) "'"))))
    (fn-value args)))

(def mal-def! [:mal-symbol "def!"])
(def mal-do [:mal-symbol "do"])
(def mal-fn* [:mal-symbol "fn*"])
(def mal-if [:mal-symbol "if"])
(def mal-let* [:mal-symbol "let*"])

(defn eval-list [[_ [mal-list-fn & mal-list-args] :as mal-list] env]
  (condp = mal-list-fn
    mal-def! (eval-mal-def! mal-list-args env)
    mal-do (eval-mal-do mal-list-args env)
    mal-fn* (eval-mal-fn* mal-list-args env)
    mal-if (eval-mal-if mal-list-args env)
    mal-let* (eval-mal-let* mal-list-args env)
    (eval-fn mal-list env)))

(defn eval [[mal-type mal-value :as mal] env]
  (if (not= :mal-list mal-type)
    (eval-ast mal env)
    (if (empty? mal-value)
      mal
      (eval-list mal env))))
