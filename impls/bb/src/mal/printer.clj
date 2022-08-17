(ns mal.printer
  (:require [clojure.string :as s]))

(declare print-str)

(defn mal-integer->string [mal-integer]
  (str mal-integer))

(defn mal-list->string [mal-list]
  (str "(" (->> mal-list (map print-str) (interpose " ") (apply str)) ")"))

(defn mal-string->string [mal-string]
  (str "\"" (s/replace mal-string #"\"" "\\\\\"") "\""))

(defn mal-symbol->string [mal-symbol]
  mal-symbol)

(defn print-str [[mal-type mal-value]]
  (case mal-type
    :mal-false "false"
    :mal-integer (mal-integer->string mal-value)
    :mal-list (mal-list->string mal-value)
    :mal-nil "nil"
    :mal-string (mal-string->string mal-value)
    :mal-symbol (mal-symbol->string mal-value)
    :mal-true "true"
    (throw (Exception. "unknown mal type"))))

(comment

  (println (mal-string->string "abc\"def"))
  (println (s/replace "abc\"def" #"\"" "\\\\x"))
  (re-find #"\"" "abc\"def")

;
  )
