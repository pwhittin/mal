(ns mal.main
  (:require [mal.core :as c]
            [mal.env :as en]
            [mal.eval :as ev]
            [mal.printer :as p]
            [mal.reader :as r]))

(def prompt "user> ")

(en/set-entries en/repl_env c/ns)

(defn READ [s] (r/read-str s))
(defn EVAL [mal env] (ev/eval mal env))
(defn PRINT [mal] (p/print-str mal))
(defn rep [s] (-> s READ (EVAL en/repl_env) PRINT))

(rep "(def! not (fn* (a) (if a false true)))")
(rep "(def! load-file (fn* (f) (eval (read-string (str \"(do \" (slurp f) \"\nnil)\")))))")

(defn execute-file [[file-spec & args]]
  (try
    (rep (str "(load-file " (pr-str file-spec) ")"))
    (println "nil")
    (catch Exception e (println "error:" (.getMessage e)))))

(defn repl []
  (loop []
    (print prompt)
    (flush)
    (let [input (read-line)]
      (when input
        (try
          (println (rep input))
          (flush)
          (catch Exception e
            (println "error:" (.getMessage e))))
        (recur)))))

(defn -main [& args]
  (let [[file-spec & mal-args] args]
    (try
      (rep (str "(def! *ARGV* (list " (apply str (interpose " " (map pr-str mal-args))) "))"))
      (if (seq args)
        (execute-file args)
        (repl))
      (catch Exception e (println "error:" (.getMessage e))))))
