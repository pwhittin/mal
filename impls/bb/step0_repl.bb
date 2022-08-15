(ns mal)

(defn READ [s] s)
(defn EVAL [s] s)
(defn PRINT [s] s)
(defn rep [s] (-> s READ EVAL PRINT))

(loop []
  (print "user> ")
  (flush)
  (let [input (read-line)]
    (when input
      (println (rep input))
      (recur))))
