(ns mal.eval)

(declare eval)

(defn lookup [mal-value env]
  (let [answer (env mal-value)]
    (when (not answer) (throw (Exception. (str "unbound symbol '" mal-value "'"))))
    answer))

(defn eval-sequence [mal-type mal-value env]
  [mal-type (map #(eval % env) mal-value)])

(defn eval-ast [mal env]
  (let [[mal-type mal-value] mal]
    (case mal-type
      :mal-symbol (lookup mal-value env)
      :mal-list (eval-sequence :mal-list mal-value env)
      :mal-map (eval-sequence :mal-map mal-value env)
      :mal-vector (eval-sequence :mal-vector mal-value env)
      mal)))

(defn eval [mal env]
  (let [[mal-type mal-value] mal]
    (if (not= :mal-list mal-type)
      (eval-ast mal env)
      (if (empty? mal-value)
        mal
        (let [[_ [[fn-type fn-value] & args]] (eval-ast mal env)]
          (when (not= :mal-fn fn-type)
            (throw (Exception. (str "attempted call of non-function '" (pr-str fn-value) "'"))))
          (fn-value args))))))

(comment

  (eval [:mal-symbol "abc"] {"abc" [:mal-fn "fn"]})
  (eval [:mal-integer 1] {"abc" [:mal-fn "fn"]})
  (eval [:mal-string "fred"] {"abc" [:mal-fn "fn"]})
  (map #(eval % {"abc" [:mal-fn "fn"]}) [[:mal-symbol "abc"] [:mal-integer 1] [:mal-string "fred"]])

  (eval [:mal-list [[:mal-symbol "abc"] [:mal-integer 1] [:mal-string "fred"]]] {"abc" [:mal-fn "fn"]})

  ;
  )
