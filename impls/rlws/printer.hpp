#ifndef PRINTER_H_
#define PRINTER_H_

#include <functional>
#include <string>
#include <unordered_map>
#include <utility>
#include "types.hpp"

namespace T = types;

using RT = T::RLWSType;
using RTS = T::RLWSTypes;
using S = T::S;

using SequenceTokenPair = std::pair<S, S>;
using SequenceTokenPairMap = std::unordered_map<RTS, SequenceTokenPair>;
static const auto RLWSTypeToSequenceTokens{
    SequenceTokenPairMap{{RTS::RLWS_LIST, {T::LIST_TOKEN_START, T::LIST_TOKEN_END}},
                         {RTS::RLWS_MAP, {T::MAP_TOKEN_START, T::MAP_TOKEN_END}},
                         {RTS::RLWS_VECTOR, {T::VECTOR_TOKEN_START, T::VECTOR_TOKEN_END}}}};
static constexpr auto SequenceTokens = [](const auto type) { return RLWSTypeToSequenceTokens.find(type)->second; };

static constexpr auto ExpandGeneric = [](const S& s, const S& expandSequence, const S& replaceSequence)
{
    auto answer{std::string{""}};
    auto lastIndex{std::size_t{0}};
    while (true)
    {
        auto nextIndex{s.find(expandSequence, lastIndex)};
        auto expandSequenceNotFound{T::NOT_FOUND == nextIndex};
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
static constexpr auto ExpandDoubleQuote = [](const auto& s) { return ExpandGeneric(s, "\"", "\\\""); };
static constexpr auto ExpandNewline = [](const auto& s) { return ExpandGeneric(s, "\n", "\\n"); };
static constexpr auto ExpandBackslash = [](const auto& s) { return ExpandGeneric(s, "\\", "\\\\"); };
static constexpr auto ExpandString = [](const auto& s) { return ExpandDoubleQuote(ExpandNewline(ExpandBackslash(s))); };

static S PrintString(const RT& rlwsType, const bool printReadably);

static constexpr auto PrintAFunction = [](const auto& rlwsType, const auto printReadably) { return S{"#<function>"}; };

static constexpr auto PrintAnInteger = [](const auto& rlwsType, const auto printReadably)
{ return std::to_string(T::ValueInteger(rlwsType)); };

static constexpr auto PrintAString = [](const auto& rlwsType, const auto printReadably)
{
    auto s{T::ValueString(rlwsType)};
    if (s.starts_with(T::KEYWORD_TOKEN))
        return ":" + s.substr(1);
    if (printReadably)
        return "\"" + ExpandString(s) + "\"";
    return s;
};

static constexpr auto PrintASymbol = [](const auto& rlwsType, const auto printReadably)
{ return T::ValueSymbol(rlwsType); };

static constexpr auto PrintASequence = [](const auto& rlwsType, const auto printReadably)
{
    auto [startToken, endToken]{SequenceTokens(rlwsType.type)};
    auto sequence{T::ValueSequence(rlwsType)};
    auto result{S{startToken}};
    for (const auto& v : sequence)
        result += PrintString(v, true) + " ";
    if (' ' == result.back())
        result.back() = endToken[0];
    else
        result += endToken;
    return result;
};

using PrintFn = std::function<S(const RT&, const bool printReadably)>;
using PrintFnMap = std::unordered_map<RTS, PrintFn>;
static const auto RLWSTypeToPrintFn{PrintFnMap{{RTS::RLWS_FUNCTION, PrintAFunction},
                                               {RTS::RLWS_INTEGER, PrintAnInteger},
                                               {RTS::RLWS_LIST, PrintASequence},
                                               {RTS::RLWS_MAP, PrintASequence},
                                               {RTS::RLWS_STRING, PrintAString},
                                               {RTS::RLWS_SYMBOL, PrintASymbol},
                                               {RTS::RLWS_VECTOR, PrintASequence}}};
static S PrintString(const RT& rlwsType, const bool printReadably)
{
    return RLWSTypeToPrintFn.find(rlwsType.type)->second(rlwsType, printReadably);
}

namespace printer
{

static constexpr auto PrintStr = [](const auto& rlwsType, const auto printReadably)
{ return PrintString(rlwsType, printReadably); };

} // namespace printer

#endif // PRINTER_H_
