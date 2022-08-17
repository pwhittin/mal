(ns mal.printer
  (:require [clojure.string :as s]))

(declare print-str)

(defn mal-integer->string [mal-integer]
  (str mal-integer))

(defn mal-sequence->string [start-token end-token mal-sequence]
  (str start-token (->> mal-sequence (map print-str) (interpose " ") (apply str)) end-token))
(def mal-list->string (partial mal-sequence->string "(" ")"))
(def mal-map->string (partial mal-sequence->string "{" "}"))
(def mal-vector->string (partial mal-sequence->string "[" "]"))

(defn mal-string->string [mal-string]
  (str "\"" (s/replace mal-string #"\"" "\\\\\"") "\""))

(defn mal-keyword->string [mal-symbol]
  mal-symbol)

(defn mal-symbol->string [mal-symbol]
  mal-symbol)

(defn print-str [[mal-type mal-value]]
  (case mal-type
    :mal-false "false"
    :mal-integer (mal-integer->string mal-value)
    :mal-keyword (mal-keyword->string mal-value)
    :mal-list (mal-list->string mal-value)
    :mal-map (mal-map->string mal-value)
    :mal-nil "nil"
    :mal-string (mal-string->string mal-value)
    :mal-symbol (mal-symbol->string mal-value)
    :mal-true "true"
    :mal-vector (mal-vector->string mal-value)
    (throw (Exception. "unknown mal type"))))
