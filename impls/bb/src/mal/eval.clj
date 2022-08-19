(ns mal.eval
  (:require [mal.env :as en]
            [mal.printer :as p]))

(declare eval)

(defn eval-sequence [mal-type mal-value env]
  [mal-type (map #(eval % env) mal-value)])

(defn eval-ast [[mal-type mal-value :as mal] env]
  (case mal-type
    :mal-symbol (en/get env mal)
    :mal-list (eval-sequence :mal-list mal-value env)
    :mal-map (eval-sequence :mal-map mal-value env)
    :mal-vector (eval-sequence :mal-vector mal-value env)
    mal))

(defn eval-mal-def!-validate! [[[symbol-type symbol-value :as symbol] value & others :as args] env]
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
  (eval-mal-def!-validate! args env)
  (let [v (eval value env)]
    (en/set env symbol v)
    v))

(defn eval-mal-do [forms env]
  (loop [fs forms
         answer nil]
    (if (empty? fs)
      answer
      (recur (rest fs) (eval (first fs) env)))))

(defn eval-mal-let*-validate! [[[sequence-type sequence-value :as sequence] value & others :as args] env]
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

(defn add-bindings! [env bindings]
  (doseq [[symbol value] (partition 2 bindings)]
    (en/set env symbol (eval value env))))

(defn eval-mal-let* [[[sequence-type sequence-value :as sequence] value & others :as args] env]
  (eval-mal-let*-validate! args env)
  (let [let-env (en/create env)]
    (add-bindings! let-env sequence-value)
    (eval value let-env)))

(defn eval-fn [mal-value env]
  (let [[_ [[fn-type fn-value] & args]] (eval-ast mal-value env)]
    (when (not= :mal-fn fn-type)
      (throw (Exception. (str "attempted call of non-function '" (pr-str fn-value) "'"))))
    (fn-value args)))

(def mal-def! [:mal-symbol "def!"])
(def mal-do [:mal-symbol "do"])
(def mal-let* [:mal-symbol "let*"])

(defn eval-list [[_ [first-mal-value & rest-mal-value :as mal-value] :as mal] env]
  (condp = first-mal-value
    mal-def! (eval-mal-def! rest-mal-value env)
    mal-do (eval-mal-do rest-mal-value env)
    mal-let* (eval-mal-let* rest-mal-value env)
    (eval-fn mal env)))

(defn eval [[mal-type mal-value :as mal] env]
  (if (not= :mal-list mal-type)
    (eval-ast mal env)
    (if (empty? mal-value)
      mal
      (eval-list mal env))))

(comment

  (defn go [n]
    (loop [ns (range n)
           answer nil]
      (if (empty? ns)
        answer
        (recur (rest ns) (* 2 (first ns))))))

  (go 10)

;
  )
