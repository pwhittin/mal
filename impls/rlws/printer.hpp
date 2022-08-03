#ifndef PRINTER_H_
#define PRINTER_H_

#include <string>
#include <utility>
#include "types.hpp"

static const auto SequenceTokens = [](const auto type)
{
    auto Answer = [](const auto& startToken, const auto& endToken) {
        return std::pair<types::S, types::S>{startToken, endToken};
    };
    switch (type)
    {
    case types::RLWSTypes::RLWS_LIST:
        return Answer(types::LIST_TOKEN_START, types::LIST_TOKEN_END);
    case types::RLWSTypes::RLWS_MAP:
        return Answer(types::MAP_TOKEN_START, types::MAP_TOKEN_END);
    case types::RLWSTypes::RLWS_VECTOR:
        return Answer(types::VECTOR_TOKEN_START, types::VECTOR_TOKEN_END);
    default:
        return Answer("", "");
    }
};

static const auto ExpandGeneric = [](const types::S& s, const types::S& expandSequence, const types::S& replaceSequence)
{
    auto answer{std::string{""}};
    auto lastIndex{std::size_t{0}};
    while (true)
    {
        auto nextIndex{s.find(expandSequence, lastIndex)};
        auto expandSequenceNotFound{std::string::npos == nextIndex};
        if (expandSequenceNotFound)
        {
            answer.append(s, lastIndex);
            return answer;
        }
        auto numberOfCharactersBeforeExpandSequence{nextIndex - lastIndex};
        answer.append(s, lastIndex, numberOfCharactersBeforeExpandSequence);
        answer.append(replaceSequence);
        lastIndex += numberOfCharactersBeforeExpandSequence + expandSequence.length();
    }
};
static const auto ExpandDoubleQuote = [](const auto& s) { return ExpandGeneric(s, "\"", "\\\""); };
static const auto ExpandNewline = [](const auto& s) { return ExpandGeneric(s, "\n", "\\n"); };
static const auto ExpandBackslash = [](const auto& s) { return ExpandGeneric(s, "\\", "\\\\"); };
static const auto ExpandString = [](const auto& s) { return ExpandDoubleQuote(ExpandNewline(ExpandBackslash(s))); };

auto PrintAnInteger = [](const auto& rlwsType) { return std::to_string(std::get<types::I>(rlwsType.value)); };
auto PrintAString = [](const auto& rlwsType) { return "\"" + ExpandString(std::get<types::S>(rlwsType.value)) + "\""; };
auto PrintASymbol = [](const auto& rlwsType) { return std::get<types::S>(rlwsType.value); };

static types::S PrintString(const types::RLWSType& rlwsType);
auto PrintASequence = [](const auto& rlwsType)
{
    auto [startToken, endToken]{SequenceTokens(rlwsType.type)};
    auto sequence{std::get<types::L>(rlwsType.value)};
    auto result{types::S{startToken}};
    for (const auto& v : sequence)
        result += PrintString(v) + " ";
    if (' ' == result.back())
        result.back() = endToken[0];
    else
        result += endToken;
    return result;
};

static types::S PrintString(const types::RLWSType& rlwsType)
{
    switch (rlwsType.type)
    {
    case types::RLWSTypes::RLWS_INTEGER:
        return PrintAnInteger(rlwsType);
    case types::RLWSTypes::RLWS_STRING:
        return PrintAString(rlwsType);
    case types::RLWSTypes::RLWS_SYMBOL:
        return PrintASymbol(rlwsType);
    case types::RLWSTypes::RLWS_LIST:
    case types::RLWSTypes::RLWS_MAP:
    case types::RLWSTypes::RLWS_VECTOR:
        return PrintASequence(rlwsType);
    }
    return ""; // quite compiler warning
}

namespace printer
{

auto PrintStr = [](const auto& rlwsType) { return PrintString(rlwsType); };

} // namespace printer

#endif // PRINTER_H_
