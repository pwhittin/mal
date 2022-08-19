(ns mal.core
  (:require [mal.env :as en]
            [mal.eval :as ev]
            [mal.printer :as p]
            [mal.reader :as r]))

(defn READ [s] (r/read-str s))
(defn EVAL [mal env] (ev/eval mal env))
(defn PRINT [mal] (p/print-str mal))
(defn rep [s] (-> s READ (EVAL en/repl_env) PRINT))

(def prompt "user> ")
(defn -main [& args]
  (loop []
    (print prompt)
    (flush)
    (let [input (read-line)]
      (when input
        (try
          (println (rep input))
          (flush)
          (catch Exception e (println "error:" (.getMessage e))))
        (recur)))))
