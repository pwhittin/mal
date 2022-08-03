#ifndef READER_H_
#define READER_H_

#include <iostream>

#include <functional>
#include <regex>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>
#include "printer.hpp"
#include "types.hpp"

namespace T = types;

using I = T::I;
using L = T::L;
using RE = std::regex;
using RT = T::RLWSType;
using RTS = T::RLWSTypes;
using S = T::S;
using TokenIndex = I;
using TokenStrings = std::vector<S>;

struct Tokens
{
    TokenIndex tokenIndex{0};
    TokenStrings tokenStrings{};
};

static constexpr auto Tokenize = [](const auto& s, const auto& re)
{
    static const auto NO_MORE_TOKENS{std::sregex_iterator()};
    static constexpr auto CurrentToken = [](const auto& reIt) { return (*reIt)[1].str(); }; // [1] only group match
    auto reIt{std::sregex_iterator(s.begin(), s.end(), re)};
    auto tokens{Tokens{}};
    while (NO_MORE_TOKENS != reIt)
        tokens.tokenStrings.push_back(CurrentToken(reIt++));
    return tokens;
};

static constexpr auto EMPTY_TOKEN{""};

static constexpr auto NoMoreTokens = [](const auto& tokens)
{ return (tokens.tokenIndex >= tokens.tokenStrings.size()); };

static constexpr auto CurrentToken = [](const auto& tokens) { return tokens.tokenStrings[tokens.tokenIndex]; };

static constexpr auto Peek = [](const auto& tokens)
{ return NoMoreTokens(tokens) ? EMPTY_TOKEN : CurrentToken(tokens); };

static constexpr auto Next = [](auto& tokens)
{
    auto token{Peek(tokens)};
    if (EMPTY_TOKEN != token)
        ++tokens.tokenIndex;
    return token;
};

static constexpr auto IsInteger = [](const auto& token)
{
    static const auto RE_INTEGER{RE{R"(-?\d+)"}};
    return std::regex_match(token, RE_INTEGER);
};

static constexpr auto RLWSInteger = [](const auto& token)
{ return RT{.type = RTS::RLWS_INTEGER, .value = std::stoi(token)}; };

static constexpr auto IsString = [](const auto& token)
{
    static const auto RE_STRING{RE{R"("(?:\\.|[^\\"])*")"}};
    if ((0 == token.length()) or (T::STRING_CHARACTER_DELIMITER != token[0]))
        return false;
    if (std::regex_match(token, RE_STRING))
        return true;
    throw std::invalid_argument("unbalanced string: '" + token + "'");
};

static constexpr auto EscapeGeneric = [](const S& s, const S& escapeSequence, const S& replaceSequence)
{
    auto answer{S{""}};
    auto lastIndex{I{0}};
    while (true)
    {
        auto nextIndex{s.find(escapeSequence, lastIndex)};
        auto escapeSequenceNotFound{T::NOT_FOUND == nextIndex};
        if (escapeSequenceNotFound)
        {
            answer.append(s, lastIndex);
            return answer;
        }
        auto numberOfCharactersBeforeEscapeSequence{nextIndex - lastIndex};
        answer.append(s, lastIndex, numberOfCharactersBeforeEscapeSequence);
        answer.append(replaceSequence);
        lastIndex += numberOfCharactersBeforeEscapeSequence + escapeSequence.length();
    }
};
static constexpr auto EscapeDoubleQuote = [](const auto& s) { return EscapeGeneric(s, "\\\"", "\""); };
static constexpr auto EscapeNewline = [](const auto& s) { return EscapeGeneric(s, "\\n", "\n"); };
static constexpr auto EscapeBackslash = [](const auto& s) { return EscapeGeneric(s, "\\\\", "\\"); };

static constexpr auto ReplaceEscapes = [](const auto& s)
{ return EscapeDoubleQuote(EscapeNewline(EscapeBackslash(s))); };

static constexpr auto RLWSString = [](const auto& token)
{
    auto stringBody{token.substr(1, token.length() - 2)};
    auto value{ReplaceEscapes(stringBody)};
    return RT{.type = RTS::RLWS_STRING, .value = value};
};

static constexpr auto RLWSSymbol = [](const auto& token) { return RT{.type = RTS::RLWS_SYMBOL, .value = token}; };

static constexpr auto ReadAtom = [](auto& tokens)
{
    auto token{Peek(tokens)};
    return IsInteger(token)  ? RLWSInteger(Next(tokens))
           : IsString(token) ? RLWSString(Next(tokens))
                             : RLWSSymbol(Next(tokens));
};

static RT ReadForm(Tokens& tokens);
static constexpr auto ReadSequence = [](auto& tokens, const auto& endToken, const RTS type)
{
    Next(tokens); // eat sequence start token
    auto result{RT{}};
    result.type = type;
    result.value = L{};
    while ((EMPTY_TOKEN != Peek(tokens)) and (endToken != Peek(tokens)))
        std::get<L>(result.value).push_back(ReadForm(tokens));
    if (EMPTY_TOKEN == Peek(tokens))
        throw std::invalid_argument("expected '" + endToken + "', got EOF");
    Next(tokens); // eat sequence end token
    return result;
};

using SequenceValue = std::pair<S, RTS>;
static const auto ListSequenceValue{SequenceValue{T::LIST_TOKEN_END, RTS::RLWS_LIST}};
static const auto MapSequenceValue{SequenceValue{T::MAP_TOKEN_END, RTS::RLWS_MAP}};
static const auto VectorSequenceValue{SequenceValue{T::VECTOR_TOKEN_END, RTS::RLWS_VECTOR}};

using SequenceValueMap = std::unordered_map<S, SequenceValue>;
static const auto TokenToSequenceValue{SequenceValueMap{{T::LIST_TOKEN_START, ListSequenceValue},
                                                        {T::MAP_TOKEN_START, MapSequenceValue},
                                                        {T::VECTOR_TOKEN_START, VectorSequenceValue}}};
static const auto SEQUENCE_VALUE_NOT_FOUND{TokenToSequenceValue.end()};

static const auto NotFoundSequenceValue{VectorSequenceValue};
static constexpr auto IsSequenceStart = [](const auto& token)
{
    static constexpr auto Answer = [](const auto& isSequenceStart, const auto& sequenceValue) {
        return std::pair<bool, SequenceValue>{isSequenceStart, sequenceValue};
    };
    auto sequenceValueEntry{TokenToSequenceValue.find(token)};
    return (SEQUENCE_VALUE_NOT_FOUND == sequenceValueEntry) ? Answer(false, NotFoundSequenceValue)
                                                            : Answer(true, sequenceValueEntry->second);
};

using ReaderMacroFn = std::function<void(Tokens&)>;

static constexpr auto InsertTokens = [](auto& tokens, const auto& insertTokens)
{
    tokens.tokenStrings.insert(tokens.tokenStrings.begin() + tokens.tokenIndex,
                               insertTokens.tokenStrings.begin(),
                               insertTokens.tokenStrings.end() - 1);
};

static const auto RE_TOKEN{RE{R"([\s,]*(~@|[\[\]{}()'`~^@]|"(?:\\.|[^\\"])*"?|;.*|[^\s\[\]{}('"`,;)]*))"}};
static constexpr auto GenericReaderMacroFn = [](auto& tokens, const auto& errorMessage, const auto& fnName)
{
    Next(tokens); // eat the reader macro token
    auto nextForm{ReadForm(tokens)};
    auto nextFormString{printer::PrintStr(nextForm)};
    if (nextFormString.empty())
        throw std::invalid_argument(errorMessage);
    auto insertString{S{"("} + fnName + " " + nextFormString + ")"};
    auto insertTokens{Tokenize(insertString, RE_TOKEN)};
    InsertTokens(tokens, insertTokens);
};

static constexpr auto DerefReaderMacroFn = [](Tokens& tokens)
{ GenericReaderMacroFn(tokens, "expected form after 'deref', got EOF", "deref"); };

static constexpr auto MetaReaderMacroFn = [](Tokens& tokens)
{
    Next(tokens); // eat the deref token
    auto metaForm{ReadForm(tokens)};
    if (RTS::RLWS_MAP != metaForm.type)
        throw std::invalid_argument("expected 'map' form after 'meta'");
    auto metaFormString{printer::PrintStr(metaForm)};
    auto nextForm{ReadForm(tokens)};
    auto nextFormString{printer::PrintStr(nextForm)};
    if (nextFormString.empty())
        throw std::invalid_argument("expected form after 'meta' form, got EOF");
    auto insertString{S{"(with-meta "} + nextFormString + " " + metaFormString + ")"};
    auto insertTokens{Tokenize(insertString, RE_TOKEN)};
    InsertTokens(tokens, insertTokens);
};

static constexpr auto QuasiQuoteReaderMacroFn = [](Tokens& tokens)
{ GenericReaderMacroFn(tokens, "expected form after 'quasiquote', got EOF", "quasiquote"); };

static constexpr auto QuoteReaderMacroFn = [](Tokens& tokens)
{ GenericReaderMacroFn(tokens, "expected form after 'quote', got EOF", "quote"); };

static constexpr auto SpliceUnquoteReaderMacroFn = [](Tokens& tokens)
{ GenericReaderMacroFn(tokens, "expected form after 'splice-unquote', got EOF", "splice-unquote"); };

static constexpr auto UnquoteReaderMacroFn = [](Tokens& tokens)
{ GenericReaderMacroFn(tokens, "expected form after 'unquote', got EOF", "unquote"); };

static constexpr auto IsReaderMacro = [](const auto& token)
{
    static constexpr auto Answer = [](const auto& isReaderMacro, const auto& readerMacroFn) {
        return std::pair<bool, ReaderMacroFn>{isReaderMacro, readerMacroFn};
    };
    if (T::DEREF_TOKEN == token)
        return Answer(true, DerefReaderMacroFn);
    if (T::META_TOKEN == token)
        return Answer(true, MetaReaderMacroFn);
    if (T::QUASI_QUOTE_TOKEN == token)
        return Answer(true, QuasiQuoteReaderMacroFn);
    if (T::QUOTE_TOKEN == token)
        return Answer(true, QuoteReaderMacroFn);
    if (T::SPLICE_UNQUOTE_TOKEN == token)
        return Answer(true, SpliceUnquoteReaderMacroFn);
    if (T::UNQUOTE_TOKEN == token)
        return Answer(true, UnquoteReaderMacroFn);
    return Answer(false, nullptr);
};

static RT ReadForm(Tokens& tokens)
{
    auto [isSequenceStart, sequenceValue]{IsSequenceStart(Peek(tokens))};
    if (isSequenceStart)
    {
        auto [endToken, type]{sequenceValue};
        return ReadSequence(tokens, endToken, type);
    }
    auto [isReaderMacro, readerMacroFn]{IsReaderMacro(Peek(tokens))};
    if (isReaderMacro)
    {
        readerMacroFn(tokens);
        return ReadForm(tokens);
    }
    return ReadAtom(tokens);
};

namespace reader
{

static constexpr auto ReadStr = [](const auto& s)
{
    auto tokens{Tokenize(s, RE_TOKEN)};
    return ReadForm(tokens);
};

}

#endif // READER_H_
