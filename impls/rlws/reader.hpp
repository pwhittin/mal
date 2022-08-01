#ifndef READER_H_
#define READER_H_

#include <regex>
#include <stdexcept>
#include <tuple>
#include "types.hpp"

using RE = std::regex;
using Tokens = std::sregex_iterator;

static const auto EMPTY_TOKEN{""};
static const auto NO_MORE_TOKENS{std::sregex_iterator()};
static const auto RE_INTEGER{RE{R"(-?\d+)"}};
static const auto RE_STRING{RE{R"("(?:\\.|[^\\"])*")"}};
static const auto RE_TOKEN{RE{R"([\s,]*(~@|[\[\]{}()'`~^@]|"(?:\\.|[^\\"])*"?|;.*|[^\s\[\]{}('"`,;)]*))"}};

static auto NoMoreTokens = [](const auto& tokens) { return (NO_MORE_TOKENS == tokens); };
static auto CurrentToken = [](const auto& tokens) { return (*tokens)[1].str(); }; // [1] looks only at the group match
static auto Tokenize = [](const auto& s) { return std::sregex_iterator(s.begin(), s.end(), RE_TOKEN); };
static auto Peek = [](const auto& tokens) { return NoMoreTokens(tokens) ? EMPTY_TOKEN : CurrentToken(tokens); };

static auto Next = [](auto& tokens)
{
    auto token{Peek(tokens)};
    if (EMPTY_TOKEN != token)
        ++tokens;
    return token;
};

static auto RLWSInteger = [](const auto& token)
{ return types::RLWSType{.type = types::RLWSTypes::RLWS_INTEGER, .value = std::stoi(token)}; };

static auto RLWSString = [](const auto& token)
{ return types::RLWSType{.type = types::RLWSTypes::RLWS_STRING, .value = token.substr(1, token.length() - 2)}; };

static auto RLWSSymbol = [](const auto& token)
{ return types::RLWSType{.type = types::RLWSTypes::RLWS_SYMBOL, .value = token}; };

static auto IsInteger = [](const auto& token) { return std::regex_match(token, RE_INTEGER); };
static auto IsString = [](const auto& token)
{
    if ((0 == token.length()) or (types::STRING_CHARACTER_DELIMITER != token[0]))
        return false;
    if (std::regex_match(token, RE_STRING))
        return true;
    throw std::invalid_argument("unbalanced string: '" + token + "'");
};
static auto ReadAtom = [](auto& tokens)
{
    auto token{Peek(tokens)};
    return IsInteger(token)  ? RLWSInteger(Next(tokens))
           : IsString(token) ? RLWSString(Next(tokens))
                             : RLWSSymbol(Next(tokens));
};

static types::RLWSType ReadForm(Tokens& tokens);
static auto ReadSequence = [](auto& tokens, const auto& endToken, const types::RLWSTypes type)
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

static auto IsSequenceStart = [](const auto& s)
{
    if (types::LIST_TOKEN_START == s)
        return std::tuple<bool, types::S, types::RLWSTypes>{true, types::LIST_TOKEN_END, types::RLWSTypes::RLWS_LIST};
    if (types::MAP_TOKEN_START == s)
        return std::tuple<bool, types::S, types::RLWSTypes>{true, types::MAP_TOKEN_END, types::RLWSTypes::RLWS_MAP};
    if (types::VECTOR_TOKEN_START == s)
        return std::tuple<bool, types::S, types::RLWSTypes>{
            true, types::VECTOR_TOKEN_END, types::RLWSTypes::RLWS_VECTOR};
    return std::tuple<bool, types::S, types::RLWSTypes>{false, "", types::RLWSTypes::RLWS_SYMBOL};
};

static types::RLWSType ReadForm(Tokens& tokens)
{
    auto [isSequenceStart, endToken, type]{IsSequenceStart(Peek(tokens))};
    return isSequenceStart ? ReadSequence(tokens, endToken, type) : ReadAtom(tokens);
};

namespace reader
{

auto ReadStr = [](const auto& s)
{
    auto tokens{Tokenize(s)};
    return ReadForm(tokens);
};

}

#endif // READER_H_
