#ifndef READER_H_
#define READER_H_

#include <iostream>

#include <functional>
#include <regex>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>
#include "printer.hpp"
#include "types.hpp"

using RE = std::regex;
using TokenIndex = types::I;
using TokenStrings = std::vector<types::S>;

struct Tokens
{
    TokenIndex tokenIndex;
    TokenStrings tokenStrings;
};

static const auto Tokenize = [](const auto& s, const auto& re)
{
    static const auto NO_MORE_TOKENS{std::sregex_iterator()};
    static const auto CurrentToken = [](const auto& reIt) { return (*reIt)[1].str(); }; // [1] only group match
    auto reIt{std::sregex_iterator(s.begin(), s.end(), re)};
    auto tokens{Tokens{}};
    tokens.tokenIndex = 0;
    while (NO_MORE_TOKENS != reIt)
        tokens.tokenStrings.push_back(CurrentToken(reIt++));
    return tokens;
};

static const auto EMPTY_TOKEN{""};
static const auto NoMoreTokens = [](const auto& tokens) { return (tokens.tokenIndex >= tokens.tokenStrings.size()); };
static const auto CurrentToken = [](const auto& tokens) { return tokens.tokenStrings[tokens.tokenIndex]; };
static const auto Peek = [](const auto& tokens) { return NoMoreTokens(tokens) ? EMPTY_TOKEN : CurrentToken(tokens); };

static const auto Next = [](auto& tokens)
{
    auto token{Peek(tokens)};
    if (EMPTY_TOKEN != token)
        ++tokens.tokenIndex;
    return token;
};

static const auto IsInteger = [](const auto& token)
{
    static const auto RE_INTEGER{RE{R"(-?\d+)"}};
    return std::regex_match(token, RE_INTEGER);
};

static const auto RLWSInteger = [](const auto& token)
{ return types::RLWSType{.type = types::RLWSTypes::RLWS_INTEGER, .value = std::stoi(token)}; };

static const auto IsString = [](const auto& token)
{
    static const auto RE_STRING{RE{R"("(?:\\.|[^\\"])*")"}};
    if ((0 == token.length()) or (types::STRING_CHARACTER_DELIMITER != token[0]))
        return false;
    if (std::regex_match(token, RE_STRING))
        return true;
    throw std::invalid_argument("unbalanced string: '" + token + "'");
};

static const auto EscapeGeneric = [](const types::S& s, const types::S& escapeSequence, const types::S& replaceSequence)
{
    auto answer{std::string{""}};
    auto lastIndex{std::size_t{0}};
    auto nextIndex{s.find(escapeSequence, lastIndex)};
    while (std::string::npos != nextIndex)
    {
        auto numberOfCharactersBeforeEscapeSequence{(nextIndex - lastIndex)};
        answer.append(s, lastIndex, numberOfCharactersBeforeEscapeSequence);
        answer.append(replaceSequence);
        lastIndex += numberOfCharactersBeforeEscapeSequence + escapeSequence.length();
        nextIndex = s.find(escapeSequence, lastIndex);
    }
    answer.append(s, lastIndex);
    return answer;
};
static const auto EscapeDoubleQuote = [](const auto& s) { return EscapeGeneric(s, "\\\"", "\""); };
static const auto EscapeNewline = [](const auto& s) { return EscapeGeneric(s, "\\n", "\n"); };
static const auto EscapeBackslash = [](const auto& s) { return EscapeGeneric(s, "\\\\", "\\"); };

static const auto RLWSString = [](const auto& token)
{
    auto stringBody{token.substr(1, token.length() - 2)};
    auto s{EscapeDoubleQuote(EscapeNewline(EscapeBackslash(stringBody)))};
    return types::RLWSType{.type = types::RLWSTypes::RLWS_STRING, .value = s};
};

static const auto RLWSSymbol = [](const auto& token)
{ return types::RLWSType{.type = types::RLWSTypes::RLWS_SYMBOL, .value = token}; };

static const auto ReadAtom = [](auto& tokens)
{
    auto token{Peek(tokens)};
    return IsInteger(token)  ? RLWSInteger(Next(tokens))
           : IsString(token) ? RLWSString(Next(tokens))
                             : RLWSSymbol(Next(tokens));
};

static types::RLWSType ReadForm(Tokens& tokens);
static const auto ReadSequence = [](auto& tokens, const auto& endToken, const types::RLWSTypes type)
{
    Next(tokens); // eat sequence start token
    auto result{types::RLWSType{}};
    result.type = type;
    result.value = types::L{};
    while ((EMPTY_TOKEN != Peek(tokens)) and (endToken != Peek(tokens)))
        std::get<types::L>(result.value).push_back(ReadForm(tokens));
    if (EMPTY_TOKEN == Peek(tokens))
        throw std::invalid_argument("expected '" + endToken + "', got EOF");
    Next(tokens); // eat sequence end token
    return result;
};

static const auto IsSequenceStart = [](const auto& token)
{
    auto Answer = [](const auto& isSequenceStart, const auto& endToken, const auto& type) {
        return std::tuple<bool, types::S, types::RLWSTypes>{isSequenceStart, endToken, type};
    };
    if (types::LIST_TOKEN_START == token)
        return Answer(true, types::LIST_TOKEN_END, types::RLWSTypes::RLWS_LIST);
    if (types::MAP_TOKEN_START == token)
        return Answer(true, types::MAP_TOKEN_END, types::RLWSTypes::RLWS_MAP);
    if (types::VECTOR_TOKEN_START == token)
        return Answer(true, types::VECTOR_TOKEN_END, types::RLWSTypes::RLWS_VECTOR);
    return Answer(false, "", types::RLWSTypes::RLWS_SYMBOL);
};

using ReaderMacroFn = std::function<void(Tokens&)>;

static const auto InsertTokens = [](auto& tokens, const auto& insertTokens)
{
    tokens.tokenStrings.insert(tokens.tokenStrings.begin() + tokens.tokenIndex,
                               insertTokens.tokenStrings.begin(),
                               insertTokens.tokenStrings.end() - 1);
};

static const auto RE_TOKEN{RE{R"([\s,]*(~@|[\[\]{}()'`~^@]|"(?:\\.|[^\\"])*"?|;.*|[^\s\[\]{}('"`,;)]*))"}};
static const auto GenericReaderMacroFn = [](auto& tokens, const auto& errorMessage, const auto& fnName)
{
    Next(tokens); // eat the reader macro token
    auto nextForm{ReadForm(tokens)};
    auto nextFormString{printer::PrintStr(nextForm)};
    if (nextFormString.empty())
        throw std::invalid_argument(errorMessage);
    auto insertString{types::S{"("} + fnName + " " + nextFormString + ")"};
    auto insertTokens{Tokenize(insertString, RE_TOKEN)};
    InsertTokens(tokens, insertTokens);
};

static const auto DerefReaderMacroFn = [](Tokens& tokens)
{ GenericReaderMacroFn(tokens, "expected form after 'deref', got EOF", "deref"); };

static const auto MetaReaderMacroFn = [](Tokens& tokens)
{
    Next(tokens); // eat the deref token
    auto metaForm{ReadForm(tokens)};
    if (types::RLWSTypes::RLWS_MAP != metaForm.type)
        throw std::invalid_argument("expected 'map' form after 'meta'");
    auto metaFormString{printer::PrintStr(metaForm)};
    auto nextForm{ReadForm(tokens)};
    auto nextFormString{printer::PrintStr(nextForm)};
    if (nextFormString.empty())
        throw std::invalid_argument("expected form after 'meta' form, got EOF");
    auto insertString{types::S{"(with-meta "} + nextFormString + " " + metaFormString + ")"};
    auto insertTokens{Tokenize(insertString, RE_TOKEN)};
    InsertTokens(tokens, insertTokens);
};

static const auto QuasiQuoteReaderMacroFn = [](Tokens& tokens)
{ GenericReaderMacroFn(tokens, "expected form after 'quasiquote', got EOF", "quasiquote"); };

static const auto QuoteReaderMacroFn = [](Tokens& tokens)
{ GenericReaderMacroFn(tokens, "expected form after 'quote', got EOF", "quote"); };

static const auto SpliceUnquoteReaderMacroFn = [](Tokens& tokens)
{ GenericReaderMacroFn(tokens, "expected form after 'splice-unquote', got EOF", "splice-unquote"); };

static const auto UnquoteReaderMacroFn = [](Tokens& tokens)
{ GenericReaderMacroFn(tokens, "expected form after 'unquote', got EOF", "unquote"); };

static const auto IsReaderMacro = [](const auto& token)
{
    auto Answer = [](const auto& isReaderMacro, const auto& readerMacroFn) {
        return std::pair<bool, ReaderMacroFn>{isReaderMacro, readerMacroFn};
    };
    if (types::DEREF_TOKEN == token)
        return Answer(true, DerefReaderMacroFn);
    if (types::META_TOKEN == token)
        return Answer(true, MetaReaderMacroFn);
    if (types::QUASI_QUOTE_TOKEN == token)
        return Answer(true, QuasiQuoteReaderMacroFn);
    if (types::QUOTE_TOKEN == token)
        return Answer(true, QuoteReaderMacroFn);
    if (types::SPLICE_UNQUOTE_TOKEN == token)
        return Answer(true, SpliceUnquoteReaderMacroFn);
    if (types::UNQUOTE_TOKEN == token)
        return Answer(true, UnquoteReaderMacroFn);
    return Answer(false, nullptr);
};

static types::RLWSType ReadForm(Tokens& tokens)
{
    auto [isSequenceStart, endToken, type]{IsSequenceStart(Peek(tokens))};
    if (isSequenceStart)
        return ReadSequence(tokens, endToken, type);
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

static const auto ReadStr = [](const auto& s)
{
    auto tokens{Tokenize(s, RE_TOKEN)};
    return ReadForm(tokens);
};

}

#endif // READER_H_
