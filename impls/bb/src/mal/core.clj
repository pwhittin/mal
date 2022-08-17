(ns mal.core
  (:require [mal.reader :as r]
            [mal.printer :as p]))

(defn READ [s] (r/read-str s))
(defn EVAL [s] s)
(defn PRINT [mal] (p/print-str mal))
(def rep (comp PRINT EVAL READ))

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
