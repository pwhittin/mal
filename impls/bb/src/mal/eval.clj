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

(defn eval-mal-def! [[[symbol-type symbol-value :as symbol] value & others :as args] env]
  (when (zero? (count args))
    (throw (Exception. "def!: wrong number of args (0)")))
  (when (not= :mal-symbol symbol-type)
    (throw (Exception. (str "def!: first arg is not a symbol " (p/print-str symbol)))))
  (when (not value)
    (throw (Exception. (str "def!: '" (p/print-str symbol) "' wrong number of args (1)"))))
  (when (seq others)
    (throw (Exception. (str "def!: '" (p/print-str symbol) "' wrong number of args (" (count args) ") "
                            (pr-str (mapv p/print-str args))))))
  (en/set env symbol (eval value env))
  symbol)

(defn eval-mal-let* [mal env])

(defn eval-fn [mal-value env]
  (let [[_ [[fn-type fn-value] & args]] (eval-ast mal-value env)]
    (when (not= :mal-fn fn-type)
      (throw (Exception. (str "attempted call of non-function '" (pr-str fn-value) "'"))))
    (fn-value args)))

(def mal-def! [:mal-symbol "def!"])
(def mal-let* [:mal-symbol "let*"])

(defn eval-list [[first-mal-value & rest-mal-value :as mal-value] env]
  (condp = first-mal-value
    mal-def! (eval-mal-def! rest-mal-value env)
    mal-let* (eval-mal-let* rest-mal-value env)
    :else (eval-fn mal-value env)))

(defn eval [[mal-type mal-value :as mal] env]
  (if (not= :mal-list mal-type)
    (eval-ast mal env)
    (if (empty? mal-value)
      mal
      (eval-list mal-value env))))

(comment

  (defn doit [x]
    (condp = x
      mal-def! "def!"
      mal-let* "let*"
      :else "fn"))

  (doit [:mal-symbol "def!"])

  (doit [1 2])

  (eval [:mal-symbol "abc"] {"abc" [:mal-fn "fn"]})
  (eval [:mal-integer 1] {"abc" [:mal-fn "fn"]})
  (eval [:mal-string "fred"] {"abc" [:mal-fn "fn"]})
  (map #(eval % {"abc" [:mal-fn "fn"]}) [[:mal-symbol "abc"] [:mal-integer 1] [:mal-string "fred"]])

  (eval [:mal-list [[:mal-symbol "abc"] [:mal-integer 1] [:mal-string "fred"]]] {"abc" [:mal-fn "fn"]})

  ;
  )
