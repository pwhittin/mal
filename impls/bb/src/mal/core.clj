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
   [:mal-symbol "-"] [:mal-fn mal-subtract]})