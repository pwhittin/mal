(ns mal.env
  (:require [mal.printer :as p]))

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

(defn set-entries [env entries]
  (doseq [[symbol mal] entries]
    (set env symbol mal)))

(def repl_env (create nil))
