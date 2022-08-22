(ns mal.core
  (:require [mal.printer :as p]))

(defn all-integers?! [fn-symbol mals]
  (let [non-integer-mals (filter #(not= :mal-integer (first %)) mals)]
    (when (seq non-integer-mals)
      (throw (Exception.
              (str fn-symbol ": not all arguments are integers '" (p/print-str [:mal-list non-integer-mals]) "'")))))
  mals)

(defn mal-integer-compare [fn-str fn [[mal-type-1 mal-value-1] [mal-type-2 mal-value-2] & rest-mals :as mals]]
  (when (nil? mal-type-1)
    (throw (Exception. (str fn-str ": wrong number of arguments (0)"))))
  (when (nil? mal-type-2)
    (throw (Exception. (str fn-str ": wrong number of arguments (1) '" (p/print-str [:mal-list mals]) "'"))))
  (when (seq rest-mals)
    (throw (Exception.
            (str fn-str ": wrong number of arguments (" (count mals) ") '" (p/print-str [:mal-list mals]) "'"))))
  (when (not (= :mal-integer mal-type-1 mal-type-2))
    (throw (Exception. (str fn-str ": arguments must be integers '" (p/print-str [:mal-list mals]) "'"))))
  (if (fn mal-value-1 mal-value-2) [:mal-true true] [:mal-false false]))

(defn is-list-or-vector? [[sequence-type _]]
  (#{:mal-list :mal-vector} sequence-type))

(defn is-map? [[sequence-type _]]
  (= :mal-map sequence-type))

(defn is-sequence? [[sequence-type _]]
  (#{:mal-list :mal-map :mal-vector} sequence-type))

(defn mal-add [mals]
  (let [ints (all-integers?! "+" mals)]
    [:mal-integer (apply + (map second ints))]))

(defn mal-count [[[mal-type-1 mal-value-1 :as mal-1] & rest-mals :as mals]]
  (when (nil? mal-type-1)
    (throw (Exception. "count: wrong number of arguments (0)")))
  (when (seq rest-mals)
    (throw (Exception.
            (str "count: wrong number of arguments (" (count mals) ") '" (p/print-str [:mal-list mals]) "'"))))
  (when (not (is-sequence? mal-1))
    (throw (Exception. (str "count: argument must be a list, map, nil or vector '" (p/print-str mal-1) "'"))))
  [:mal-integer (if (= :mal-nil mal-type-1)
                  0
                  (/ (count mal-value-1) (if (= :mal-map mal-type-1) 2 1)))])

(defn mal-divide [mals]
  (let [ints (all-integers?! "/" mals)]
    (when (= 0 (count ints))
      (throw (Exception. "/: wrong number of arguments (0)")))
    [:mal-integer (int (apply / (map second ints)))]))

(defn mal-empty? [[[mal-type-1 mal-value-1 :as mal-1] & rest-mals :as mals]]
  (when (nil? mal-type-1)
    (throw (Exception. "empty?: wrong number of arguments (0)")))
  (when (seq rest-mals)
    (throw (Exception.
            (str "empty?: wrong number of arguments (" (count mals) ") '" (p/print-str [:mal-list mals]) "'"))))
  (when (not (is-sequence? mal-1))
    (throw (Exception. (str "empty?: argument must be a list, map or vector '" (p/print-str mal-1) "'"))))
  (if (zero? (count mal-value-1)) [:mal-true true] [:mal-false false]))

(defn mal-equal [[[mal-type-1 mal-value-1 :as mal-1] [mal-type-2 mal-value-2 :as mal-2] & rest-mals :as mals]]
  (when (nil? mal-type-1)
    (throw (Exception. "=: wrong number of arguments (0)")))
  (when (nil? mal-type-2)
    (throw (Exception. (str "=: wrong number of arguments (1) '" (p/print-str [:mal-list mals]) "'"))))
  (when (seq rest-mals)
    (throw (Exception.
            (str "=: wrong number of arguments (" (count mals) ") '" (p/print-str [:mal-list mals]) "'"))))
  (cond
    (and (is-list-or-vector? mal-1) (is-list-or-vector? mal-2))
    (if (and (= (count mal-value-1) (count mal-value-2))
             (empty? (filter #(= [:mal-false false] %) (map #(mal-equal [%1 %2]) mal-value-1 mal-value-2))))
      [:mal-true true]
      [:mal-false false])

    (and (is-map? mal-1) (is-map? mal-2))
    (if (= (apply hash-map mal-value-1) (apply hash-map mal-value-2)) [:mal-true true] [:mal-false false])

    :else
    (if (= mal-1 mal-2) [:mal-true true] [:mal-false false])))

(def mal-less-than (partial mal-integer-compare "<" <))
(def mal-less-than-or-equal (partial mal-integer-compare "<=" <=))
(def mal-greater-than (partial mal-integer-compare ">" >))
(def mal-greater-than-or-equal (partial mal-integer-compare ">=" >=))

(defn mal-list [mals]
  [:mal-list mals])

(defn mal-list? [[[mal-type-1 _] & rest-mals :as mals]]
  (when (nil? mal-type-1)
    (throw (Exception. "<: wrong number of arguments (0)")))
  (when (seq rest-mals)
    (throw (Exception.
            (str "list?: wrong number of arguments (" (count mals) ") '" (p/print-str [:mal-list mals]) "'"))))
  (if (= :mal-list mal-type-1) [:mal-true true] [:mal-false false]))

(defn mal-multiply [mals]
  (let [ints (all-integers?! "*" mals)]
    [:mal-integer (apply * (map second ints))]))

(defn mal-pr-str [mals]
  [:mal-string (->> mals (map p/print-str) (interpose " ") (apply str))])

(defn mal-subtract [mals]
  (let [ints (all-integers?! "-" mals)]
    (when (= 0 (count ints))
      (throw (Exception. "-: wrong number of arguments (0)")))
    [:mal-integer (int (apply - (map second ints)))]))

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
   [:mal-symbol "count"] [:mal-fn mal-count]
   [:mal-symbol "empty?"] [:mal-fn mal-empty?]
   [:mal-symbol "list"] [:mal-fn mal-list]
   [:mal-symbol "list?"] [:mal-fn mal-list?]
   [:mal-symbol "pr-str"] [:mal-fn mal-pr-str]})