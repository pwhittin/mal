(ns mal.core
  (:require [mal.printer :as p]))

(defn all-integers?! [fn-symbol mals]
  (let [non-integer-mals (filter #(not= :mal-integer (first %)) mals)]
    (when (seq non-integer-mals)
      (throw (Exception.
              (str fn-symbol ": not all arguments are integers '" (p/print-str [:mal-list non-integer-mals]) "'")))))
  mals)

(defn mal-add [mals]
  (let [ints (all-integers?! "+" mals)]
    [:mal-integer (apply + (map second ints))]))

(defn mal-count [[[mal-type-1 mal-value-1 :as mal-1] & rest-mals :as mals]]
  (when (nil? mal-type-1)
    (throw (Exception. "count: wrong number of arguments (0)")))
  (when (seq rest-mals)
    (throw (Exception.
            (str "count: wrong number of arguments (" (count mals) ") '" (p/print-str [:mal-list mals]) "'"))))
  (when (not (#{:mal-list :mal-map :mal-nil :mal-vector} mal-type-1))
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
  (when (not (#{:mal-list :mal-map :mal-vector} mal-type-1))
    (throw (Exception. (str "empty?: argument must be a list, map or vector '" (p/print-str mal-1) "'"))))
  (if (zero? (count mal-value-1)) [:mal-true true] [:mal-false false]))

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
   [:mal-symbol "<"] [:mal-fn mal-less-than]
   [:mal-symbol "<="] [:mal-fn mal-less-than-or-equal]
   [:mal-symbol ">"] [:mal-fn mal-greater-than]
   [:mal-symbol ">="] [:mal-fn mal-greater-than-or-equal]
   [:mal-symbol "count"] [:mal-fn mal-count]
   [:mal-symbol "empty?"] [:mal-fn mal-empty?]
   [:mal-symbol "list"] [:mal-fn mal-list]
   [:mal-symbol "list?"] [:mal-fn mal-list?]})