(ns mal.main
  (:require [mal.core :as c]
            [mal.env :as en]
            [mal.eval :as ev]
            [mal.printer :as p]
            [mal.reader :as r]))

(en/set-entries en/repl_env c/ns)

(defn READ [s] (r/read-str s))
(defn EVAL [mal env] (ev/eval mal env))
(defn PRINT [mal] (p/print-str mal))
(defn rep [s] (-> s READ (EVAL en/repl_env) PRINT))

(rep "(def! not (fn* (a) (if a false true)))")
(rep "(def! load-file (fn* (f) (eval (read-string (str \"(do \" (slurp f) \"\nnil)\")))))")

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
