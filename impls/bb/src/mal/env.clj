(ns mal.env
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

(defn set [env symbol mal]
  (swap! env assoc-in [:data symbol] mal))

(defn find [env symbol]
  (if (get-in @env [:data symbol])
    env
    (when (:outer @env)
      (recur (:outer @env) symbol))))

(defn get [env symbol]
  (let [found-env (find env symbol)]
    (when (not found-env)
      (throw (Exception. (str "symbol '" (second symbol) "' not found"))))
    (get-in @found-env [:data symbol])))

(defn create [outer]
  (atom {:data {}
         :outer outer}))

(def repl_env (create nil))
(set repl_env [:mal-symbol "+"] [:mal-fn mal-add])
(set repl_env [:mal-symbol "/"] [:mal-fn mal-divide])
(set repl_env [:mal-symbol "*"] [:mal-fn mal-multiply])
(set repl_env [:mal-symbol "-"] [:mal-fn mal-subtract])
