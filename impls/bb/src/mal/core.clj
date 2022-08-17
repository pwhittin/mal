(ns mal.core
  (:require [mal.env :as en]
            [mal.eval :as ev]
            [mal.printer :as p]
            [mal.reader :as r]))

(defn READ [s] (r/read-str s))
(defn EVAL [mal env] (ev/eval mal env))
(defn PRINT [mal] (p/print-str mal))
(defn rep [s] (-> s READ (EVAL en/repl_env) PRINT))

(defn -main [& args]
  (loop []
    (print "user> ")
    (flush)
    (let [input (read-line)]
      (when input
        (flush)
        (try
          (println (rep input))
          (catch Exception e (println "error:" (.getMessage e))))
        (recur)))))
