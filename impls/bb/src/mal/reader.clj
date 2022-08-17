(ns mal.reader
  (:require [clojure.string :as s]
            [mal.printer :as p]))

(def re-token-s "[\\s,]*(~@|[\\[\\]{}()'`~^@]|\"(?:\\\\.|[^\\\\\"])*\"?|;.*|[^\\s\\[\\]{}('\"`,;)]*)")
(def re-token (re-pattern re-token-s))

(def re-integer #"[-+]?\d+")

(defn tokenize [s]
  (->> s
       (re-seq re-token)
       (map second)
       (filter #(seq %))))

(declare read-form)

(defn beginning-of-sequence [tokens]
  (contains? #{"(" "[" "{"} (first tokens)))

(def start-token->end-token {"(" ")", "[" "]", "{" "}"})
(def start-token->sequence-mal-type {"(" :mal-list, "[" :mal-vector, "{" :mal-map})

(defn read-sequence [tokens]
  (let [start-token (first tokens)
        end-token (start-token->end-token start-token)]
    (loop [remaining-tokens (rest tokens)
           list-mals []]
      (when (empty? remaining-tokens)
        (throw (Exception. "unbalanced '('")))
      (if (= end-token (first remaining-tokens))
        [(rest remaining-tokens) [(start-token->sequence-mal-type start-token) list-mals]]
        (let [[rts mal] (read-form remaining-tokens)]
          (recur rts (conj list-mals mal)))))))

(defn token-is-false? [token]
  (= "false" token))

(defn token-is-integer? [token]
  (let [match (re-find re-integer token)]
    (= match token)))

(defn token-is-nil? [token]
  (= "nil" token))

(defn remove-escaped-backslash [s]
  (s/replace s #"\\\\" ""))

(defn remove-escaped-double-quote [s]
  (s/replace s #"\\\"" ""))

(defn token-is-string? [token]
  (let [s (-> token remove-escaped-backslash remove-escaped-double-quote)
        at-least-two-characters-long (>= (count s) 2)
        starts-with-double-quote (s/starts-with? s "\"")
        ends-with-double-quote (s/ends-with? s "\"")]
    (and at-least-two-characters-long starts-with-double-quote ends-with-double-quote)))

(defn token-is-symbol? [token]
  (let [does-not-start-with-digit (not= token (re-find #"\d.*" token))
        does-not-contain-double-quote (not (s/includes? token "\""))]
    (and does-not-start-with-digit does-not-contain-double-quote)))

(defn token-is-true? [token]
  (= "true" token))

(defn token-is-keyword? [token]
  (s/starts-with? token ":"))

(defn token->integer [token]
  (Integer/parseInt token))

(defn token->string [token]
  (s/replace (apply str (->> token (drop 1) (drop-last))) #"\\\"" "\""))

(defn read-atom [tokens]
  (let [token (first tokens)
        mal (cond
              (token-is-true? token) [:mal-true true]
              (token-is-false? token) [:mal-false false]
              (token-is-nil? token) [:mal-nil nil]
              (token-is-integer? token) [:mal-integer (token->integer token)]
              (token-is-string? token) [:mal-string (token->string token)]
              (token-is-keyword? token) [:mal-keyword token]
              (token-is-symbol? token) [:mal-symbol token]
              :else (throw (Exception. (str "end of input '" token "'"))))]
    [(rest tokens) mal]))

(def token-is-reader-macro? #{"'" "`" "~" "~@" "@" "^"})

(defn expand-reader-macro-generic [fn-name tokens]
  (let [[remaining-tokens mal] (read-form tokens)
        mal-s (p/print-str mal)
        expanded-form (str "(" fn-name " " mal-s ")")
        expanded-form-tokens (tokenize expanded-form)]
    (concat expanded-form-tokens remaining-tokens)))

(def expand-reader-macro-quote (partial expand-reader-macro-generic "quote"))
(def expand-reader-macro-quasiquote (partial expand-reader-macro-generic "quasiquote"))
(def expand-reader-macro-unquote (partial expand-reader-macro-generic "unquote"))
(def expand-reader-macro-splice-unquote (partial expand-reader-macro-generic "splice-unquote"))
(def expand-reader-macro-deref (partial expand-reader-macro-generic "deref"))

(defn expand-reader-macro-with-meta [tokens]
  (let [[remaining-tokens-meta mal-meta] (read-form tokens)
        _ (when (not= :mal-map (first mal-meta)) (throw (Exception. "meta form must be a map")))
        [remaining-tokens mal] (read-form remaining-tokens-meta)
        mal-meta-s (p/print-str mal-meta)
        mal-s (p/print-str mal)
        expanded-form (str "(with-meta " mal-s " " mal-meta-s ")")
        expanded-form-tokens (tokenize expanded-form)]
    (concat expanded-form-tokens remaining-tokens)))

(defn expand-reader-macro [tokens]
  (case (first tokens)
    "'" (expand-reader-macro-quote (rest tokens))
    "`" (expand-reader-macro-quasiquote (rest tokens))
    "~" (expand-reader-macro-unquote (rest tokens))
    "~@" (expand-reader-macro-splice-unquote (rest tokens))
    "@" (expand-reader-macro-deref (rest tokens))
    "^" (expand-reader-macro-with-meta (rest tokens))))

(defn read-form [tokens]
  (let [expanded-tokens (if (token-is-reader-macro? (first tokens)) (expand-reader-macro tokens) tokens)]
    ((if (beginning-of-sequence expanded-tokens) read-sequence read-atom) expanded-tokens)))

(defn read-str [s]
  (let [tokens (tokenize s)
        [_ mals] (read-form tokens)]
    mals))

(comment

  (read-str ":one")

;
  )
