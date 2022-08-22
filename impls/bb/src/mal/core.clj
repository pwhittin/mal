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

(defn mal-divide [mals]
  (let [ints (all-integers?! "/" mals)]
    (when (= 0 (count ints))
      (throw (Exception. "/: wrong number of arguments (0)")))
    [:mal-integer (int (apply / (map second ints)))]))

(defn mal-empty? [[[first-mal-type first-mal-value :as first-mal] & rest-mals :as mals]]
  (when (seq rest-mals)
    (throw (Exception.
            (str "empty?: wrong number of arguments (" (count mals) ") '" (p/print-str [:mal-list mals]) "'"))))
  (when (not (#{:mal-list :mal-map :mal-vector} first-mal-type))
    (throw (Exception.
            (str "empty?: argument must be a list, map or vector '" (p/print-str first-mal) "'"))))
  (if (zero? (count first-mal-value)) [:mal-true true] [:mal-false false]))

(defn mal-list [mals]
  [:mal-list mals])

(defn mal-list? [[[first-mal-type _] & rest-mals :as mals]]
  (when (seq rest-mals)
    (throw (Exception.
            (str "list?: wrong number of arguments (" (count mals) ") '" (p/print-str [:mal-list mals]) "'"))))
  (if (= :mal-list first-mal-type) [:mal-true true] [:mal-false false]))

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
   [:mal-symbol "empty?"] [:mal-fn mal-empty?]
   [:mal-symbol "list"] [:mal-fn mal-list]
   [:mal-symbol "list?"] [:mal-fn mal-list?]})