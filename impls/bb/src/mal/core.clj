(ns mal.core
  (:require [mal.env :as en]
            [mal.eval :as ev]
            [mal.printer :as p]
            [mal.reader :as r]))

(defn all-integers?! [fn-symbol mals]
  (let [non-integer-mals (filter #(not= :mal-integer (first %)) mals)]
    (when (seq non-integer-mals)
      (throw (Exception.
              (str fn-symbol ": not all arguments are integers '" (p/print-str [:mal-list non-integer-mals]) "'")))))
  mals)

(defn mal-tf [b]
  (if b [:mal-true true] [:mal-false false]))

(defn validate-mals-count! [fn-str required-mals-count mals]
  (let [mals-count (count mals)]
    (cond
      (zero? mals-count) (throw (Exception. (str fn-str ": wrong number of arguments (0)")))
      (not= mals-count required-mals-count) (throw (Exception.
                                                    (str fn-str ": wrong number of arguments (" (count mals) ") '"
                                                         (p/print-str [:mal-list mals]) "'"))))))

(defn mal-integer-compare [fn-str fn [[mal-type-1 mal-value-1] [mal-type-2 mal-value-2] & _ :as mals]]
  (validate-mals-count! fn-str 2 mals)
  (when (not (= :mal-integer mal-type-1 mal-type-2))
    (throw (Exception. (str fn-str ": arguments must be integers '" (p/print-str [:mal-list mals]) "'"))))
  (mal-tf (fn mal-value-1 mal-value-2)))

(defn is-list-or-vector? [[sequence-type _]]
  (#{:mal-list :mal-vector} sequence-type))

(defn is-map? [[sequence-type _]]
  (= :mal-map sequence-type))

(defn is-sequence? [[sequence-type _]]
  (#{:mal-list :mal-map :mal-vector} sequence-type))

(defn mal-add [mals]
  (let [ints (all-integers?! "+" mals)]
    [:mal-integer (apply + (map second ints))]))

(defn mal-atom [[mal-1 & _ :as mals]]
  (validate-mals-count! "atom" 1 mals)
  [:mal-atom (atom mal-1)])

(defn mal-atom? [[[mal-type-1 _] & _ :as mals]]
  (validate-mals-count! "atom?" 1 mals)
  (mal-tf (= :mal-atom mal-type-1)))

(defn mal-count [[[mal-type-1 mal-value-1 :as mal-1] & _ :as mals]]
  (validate-mals-count! "count" 1 mals)
  (if (= [:mal-nil nil] mal-1)
    [:mal-integer 0]
    (do
      (when (not (is-sequence? mal-1))
        (throw (Exception. (str "count: argument must be a list, map, or vector '" (p/print-str mal-1) "'"))))
      [:mal-integer (/ (count mal-value-1) (if (= :mal-map mal-type-1) 2 1))])))

(defn mal-deref [[[mal-type-1 mal-value-1 :as mal-1] & _ :as mals]]
  (validate-mals-count! "deref" 1 mals)
  (when (not (= :mal-atom mal-type-1))
    (throw (Exception. (str "deref: argument must be an atom '" (p/print-str mal-1) "'"))))
  @mal-value-1)

(defn mal-divide [mals]
  (let [ints (all-integers?! "/" mals)]
    (when (= 0 (count ints))
      (throw (Exception. "/: wrong number of arguments (0)")))
    [:mal-integer (int (apply / (map second ints)))]))

(defn mal-empty? [[[mal-type-1 mal-value-1 :as mal-1] & rest-mals :as mals]]
  (validate-mals-count! "empty?" 1 mals)
  (when (not (is-sequence? mal-1))
    (throw (Exception. (str "empty?: argument must be a list, map or vector '" (p/print-str mal-1) "'"))))
  (mal-tf (zero? (count mal-value-1))))

(defn mal-eval [[mal-1 & rest-mals :as mals]]
  (validate-mals-count! "eval" 1 mals)
  (ev/eval mal-1 en/repl_env))

(defn mal-equal [[[_ mal-value-1 :as mal-1] [_ mal-value-2 :as mal-2] & _ :as mals]]
  (validate-mals-count! "=" 2 mals)
  (cond
    (and (is-list-or-vector? mal-1) (is-list-or-vector? mal-2))
    (mal-tf (and (= (count mal-value-1) (count mal-value-2))
                 (empty? (filter #(= [:mal-false false] %) (map #(mal-equal [%1 %2]) mal-value-1 mal-value-2)))))

    (and (is-map? mal-1) (is-map? mal-2))
    (mal-tf (= (apply hash-map mal-value-1) (apply hash-map mal-value-2)))

    :else
    (mal-tf (= mal-1 mal-2))))

(def mal-less-than (partial mal-integer-compare "<" <))
(def mal-less-than-or-equal (partial mal-integer-compare "<=" <=))
(def mal-greater-than (partial mal-integer-compare ">" >))
(def mal-greater-than-or-equal (partial mal-integer-compare ">=" >=))

(defn mal-list [mals]
  [:mal-list mals])

(defn mal-list? [[[mal-type-1 _] & rest-mals :as mals]]
  (validate-mals-count! "list?" 1 mals)
  (mal-tf (= :mal-list mal-type-1)))

(defn mal-multiply [mals]
  (let [ints (all-integers?! "*" mals)]
    [:mal-integer (apply * (map second ints))]))

(defn mal-pr-str [mals]
  [:mal-string (->> mals (map p/print-str) (interpose " ") (apply str))])

(defn mal-println [mals]
  (println (->> mals (map #(p/print-str % false)) (interpose " ") (apply str)))
  [:mal-nil nil])

(defn mal-prn [mals]
  (println (->> mals (map p/print-str) (interpose " ") (apply str)))
  [:mal-nil nil])

(defn mal-read-string [[[mal-type-1 mal-value-1] & _ :as mals]]
  (validate-mals-count! "read-string" 1 mals)
  (when (not= :mal-string mal-type-1)
    (throw (Exception. (str "read-string: argument must be a string '" (p/print-str [:mal-list mals]) "'"))))
  (r/read-str mal-value-1))

(defn mal-reset! [[[mal-type-1 mal-value-1 :as mal-1] mal-2 & _ :as mals]]
  (validate-mals-count! "reset!" 2 mals)
  (when (not= :mal-atom mal-type-1)
    (throw (Exception. (str "reset!: first argument must be an atom '" (p/print-str [:mal-list mals]) "'"))))
  (reset! mal-value-1 mal-2)
  mal-2)

(defn mal-slurp [[[mal-type-1 mal-value-1 :as mal-1] & rest-mals :as mals]]
  (validate-mals-count! "slurp" 1 mals)
  (when (not= :mal-string mal-type-1)
    (throw (Exception. (str "slurp: argument must be a string '" (p/print-str [:mal-list mals]) "'"))))
  [:mal-string (slurp mal-value-1)])

(defn mal-str [mals]
  [:mal-string (->> mals (map #(p/print-str % false)) (apply str))])

(defn mal-subtract [mals]
  (let [ints (all-integers?! "-" mals)]
    (when (= 0 (count ints))
      (throw (Exception. "-: wrong number of arguments (0)")))
    [:mal-integer (int (apply - (map second ints)))]))

(defn mal-swap! [[[mal-type-1 mal-value-1 :as mal-1] [mal-type-2 mal-value-2 :as mal-2] & rest-mals :as mals]]
  (let [mals-count (count mals)]
    (cond
      (zero? mals-count) (throw (Exception. (str "swap!: wrong number of arguments (0)")))
      (< mals-count 2) (throw (Exception. (str "swap!: wrong number of arguments (" (count mals) ") '"
                                               (p/print-str [:mal-list mals]) "'")))
      (not= :mal-atom mal-type-1) (throw (Exception. (str "swap!: first argument must be an atom (" (count mals) ") '"
                                                          (p/print-str [:mal-list mals]) "'")))
      (not (#{:mal-fn :mal-fn*} mal-type-2)) (throw (Exception.
                                                     (str "swap!: second argument must be a function (" (count mals)
                                                          ") '" (p/print-str [:mal-list mals]) "'")))
      :else (let [mal-atom mal-value-1
                  current-mal-atom-value @mal-atom
                  mal-fn mal-2
                  expression [:mal-list (concat [mal-fn current-mal-atom-value] rest-mals)]
                  new-mal-atom-value (ev/eval expression en/repl_env)]
              (reset! mal-atom new-mal-atom-value)
              new-mal-atom-value))))

(def ns
  {[:mal-symbol "+"] [:mal-fn mal-add]
   [:mal-symbol "/"] [:mal-fn mal-divide]
   [:mal-symbol "*"] [:mal-fn mal-multiply]
   [:mal-symbol "-"] [:mal-fn mal-subtract]
   [:mal-symbol "="] [:mal-fn mal-equal]
   [:mal-symbol "<"] [:mal-fn mal-less-than]
   [:mal-symbol "<="] [:mal-fn mal-less-than-or-equal]
   [:mal-symbol ">"] [:mal-fn mal-greater-than]
   [:mal-symbol ">="] [:mal-fn mal-greater-than-or-equal]
   [:mal-symbol "atom"] [:mal-fn mal-atom]
   [:mal-symbol "atom?"] [:mal-fn mal-atom?]
   [:mal-symbol "count"] [:mal-fn mal-count]
   [:mal-symbol "deref"] [:mal-fn mal-deref]
   [:mal-symbol "empty?"] [:mal-fn mal-empty?]
   [:mal-symbol "eval"] [:mal-fn mal-eval]
   [:mal-symbol "list"] [:mal-fn mal-list]
   [:mal-symbol "list?"] [:mal-fn mal-list?]
   [:mal-symbol "pr-str"] [:mal-fn mal-pr-str]
   [:mal-symbol "println"] [:mal-fn mal-println]
   [:mal-symbol "prn"] [:mal-fn mal-prn]
   [:mal-symbol "read-string"] [:mal-fn mal-read-string]
   [:mal-symbol "reset!"] [:mal-fn mal-reset!]
   [:mal-symbol "slurp"] [:mal-fn mal-slurp]
   [:mal-symbol "str"] [:mal-fn mal-str]
   [:mal-symbol "swap!"] [:mal-fn mal-swap!]})
