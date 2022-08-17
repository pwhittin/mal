(ns mal.reader
  (:require [clojure.string :as s]))

(def re-token-s "[\\s,]*(~@|[\\[\\]{}()'`~^@]|\"(?:\\\\.|[^\\\\\"])*\"?|;.*|[^\\s\\[\\]{}('\"`,;)]*)")
(def re-token (re-pattern re-token-s))

(def re-integer #"\d+")

(def re-string-s "\"(?:\\\\.|[^\\\\\"])*\"?")
(def re-string (re-pattern re-string-s))

(defn tokenize [s]
  (->> s
       (re-seq re-token)
       (map second)
       (filter #(seq %))))

(declare read-form)

(defn beginning-of-list [tokens]
  (= "(" (first tokens)))

(defn read-list [tokens]
  (loop [remaining-tokens (rest tokens)
         list-mals []]
    (when (empty? remaining-tokens)
      (throw (Exception. "unbalanced '('")))
    (if (= ")" (first remaining-tokens))
      [(rest remaining-tokens) [:mal-list list-mals]]
      (let [[rts mal] (read-form remaining-tokens)]
        (recur rts (conj list-mals mal))))))

(defn token-is-false? [token]
  (= "false" token))

(defn token-is-integer? [token]
  (let [match (re-find re-integer token)]
    (= match token)))

(defn token-is-nil? [token]
  (= "nil" token))

(defn token-is-string? [token]
  (let [match (re-find re-string token)]
    (and (= match token)
         (not= "\\\"" (take-last 2 token)))))

(defn token-is-symbol? [token]
  (not= token (re-find #"\d.*" token)))

(defn token-is-true? [token]
  (= "true" token))

(defn token->integer [token]
  (Integer/parseInt token))

(defn token->string [token]
  (s/replace (apply str (->> token (drop 1) (drop-last))) #"\\\"" "\""))

(defn read-atom [tokens]
  (let [token (first tokens)
        mal (cond
              (token-is-false? token) [:mal-false false]
              (token-is-integer? token) [:mal-integer (token->integer token)]
              (token-is-nil? token) [:mal-nil nil]
              (token-is-string? token) [:mal-string (token->string token)]
              (token-is-symbol? token) [:mal-symbol token]
              (token-is-true? token) [:mal-true true]
              :else (throw (Exception. (str "invalid token '" token "'"))))]
    [(rest tokens) mal]))

(defn read-form [tokens]
  ((if (beginning-of-list tokens) read-list read-atom) tokens))

(defn read-str [s]
  (let [tokens (tokenize s)
        [_ mals] (read-form tokens)]
    mals))

(comment

  (def s "abc\\\"def")
  (read-str s)

;
  )

