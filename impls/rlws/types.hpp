#ifndef TYPES_H_
#define TYPES_H_

#include <cstdint>
#include <functional>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

namespace types
{

static const auto DEFBANG_TOKEN{"def!"};
static const auto DEREF_TOKEN{"@"};
static const auto DO_TOKEN{"do"};
static const auto KEYWORD_TOKEN{"\xFF"};
static const auto LETSTAR_TOKEN{"let*"};
static const auto LIST_TOKEN_END{")"};
static const auto LIST_TOKEN_START{"("};
static const auto MAP_TOKEN_END{"}"};
static const auto MAP_TOKEN_START{"{"};
static const auto META_TOKEN{"^"};
static constexpr auto NOT_FOUND{std::string::npos};
static const auto QUASI_QUOTE_TOKEN{"`"};
static const auto QUOTE_TOKEN{"'"};
static const auto SPLICE_UNQUOTE_TOKEN{"~@"};
static const auto STRING_CHARACTER_DELIMITER{'"'};
static const auto UNQUOTE_TOKEN{"~"};
static const auto VECTOR_TOKEN_END{"]"};
static const auto VECTOR_TOKEN_START{"["};

struct RLWSType;

using F = std::function<RLWSType(const RLWSType&)>;
using I = intmax_t;
using L = std::vector<RLWSType>;
using S = std::string;
using V = std::variant<F, I, L, S>;

enum class RLWSTypes
{
    RLWS_FUNCTION,
    RLWS_INTEGER,
    RLWS_LIST,
    RLWS_MAP,
    RLWS_STRING,
    RLWS_SYMBOL,
    RLWS_VECTOR
};

struct RLWSType
{
    RLWSTypes type;
    V value;
};

static constexpr auto CreateRLWSType = [](const auto type, const auto& value)
{ return RLWSType{.type = type, .value = value}; };

static constexpr auto CreateFunction = [](const auto& value)
{ return CreateRLWSType(RLWSTypes::RLWS_FUNCTION, value); };

static constexpr auto CreateInteger = [](const auto& value) { return CreateRLWSType(RLWSTypes::RLWS_INTEGER, value); };
static constexpr auto CreateList = [](const auto& value) { return CreateRLWSType(RLWSTypes::RLWS_LIST, value); };
static constexpr auto CreateMap = [](const auto& value) { return CreateRLWSType(RLWSTypes::RLWS_MAP, value); };
static constexpr auto CreateString = [](const auto& value) { return CreateRLWSType(RLWSTypes::RLWS_STRING, value); };
static constexpr auto CreateSymbol = [](const auto& value) { return CreateRLWSType(RLWSTypes::RLWS_SYMBOL, value); };
static constexpr auto CreateVector = [](const auto& value) { return CreateRLWSType(RLWSTypes::RLWS_VECTOR, value); };

static constexpr auto IsFunction = [](const auto& rlwsType) { return (RLWSTypes::RLWS_FUNCTION == rlwsType.type); };
static constexpr auto IsInteger = [](const auto& rlwsType) { return (RLWSTypes::RLWS_INTEGER == rlwsType.type); };
static constexpr auto IsList = [](const auto& rlwsType) { return (RLWSTypes::RLWS_LIST == rlwsType.type); };
static constexpr auto IsMap = [](const auto& rlwsType) { return (RLWSTypes::RLWS_MAP == rlwsType.type); };
static constexpr auto IsString = [](const auto& rlwsType) { return (RLWSTypes::RLWS_STRING == rlwsType.type); };
static constexpr auto IsSymbol = [](const auto& rlwsType) { return (RLWSTypes::RLWS_SYMBOL == rlwsType.type); };
static constexpr auto IsVector = [](const auto& rlwsType) { return (RLWSTypes::RLWS_VECTOR == rlwsType.type); };

static constexpr auto ValueSequence = [](const auto& rlwsType) { return std::get<L>(rlwsType.value); };
static constexpr auto ValueFunction = [](const auto& rlwsType) { return std::get<F>(rlwsType.value); };
static constexpr auto ValueInteger = [](const auto& rlwsType) { return std::get<I>(rlwsType.value); };
static constexpr auto ValueList = [](const auto& rlwsType) { return ValueSequence(rlwsType); };
static constexpr auto ValueMap = [](const auto& rlwsType) { return ValueSequence(rlwsType); };
static constexpr auto ValueString = [](const auto& rlwsType) { return std::get<S>(rlwsType.value); };
static constexpr auto ValueSymbol = [](const auto& rlwsType) { return ValueString(rlwsType); };
static constexpr auto ValueVector = [](const auto& rlwsType) { return ValueSequence(rlwsType); };

static constexpr auto Count = [](const auto& sequence) { return sequence.size(); };
static constexpr auto First = [](const auto& sequence) { return sequence[0]; };
static constexpr auto IsEmpty = [](const auto& sequence) { return (0 == Count(sequence)); };

static constexpr auto CreateException = [](const auto& message) { return std::invalid_argument(message); };

static constexpr auto RLWSTypeToString = [](const auto& rlwsType)
{
    switch (rlwsType.type)
    {
    case RLWSTypes::RLWS_FUNCTION:
        return S{"Function"};
    case RLWSTypes::RLWS_INTEGER:
        return S{"Integer: "} + std::to_string(ValueInteger(rlwsType));
    case RLWSTypes::RLWS_LIST:
        return S{"List"};
    case RLWSTypes::RLWS_MAP:
        return S{"Map"};
    case RLWSTypes::RLWS_STRING:
        return S{"String: \""} + ValueString(rlwsType) + "\"";
    case RLWSTypes::RLWS_SYMBOL:
        return S{"Symbol: \""} + ValueString(rlwsType) + "\"";
    case RLWSTypes::RLWS_VECTOR:
        return S{"Vector"};
    }
    return S{"UNKNOWN"};
};

static constexpr auto IsDefBang = [](const auto& rlwsType)
{
    if (not IsList(rlwsType))
        return false;
    auto list{ValueList(rlwsType)};
    if (IsEmpty(list))
        return false;
    auto defBangElement{list[0]};
    if ((not IsSymbol(defBangElement)) or (DEFBANG_TOKEN != ValueSymbol(defBangElement)))
        return false;
    if (3 != Count(list))
        throw CreateException("two arguments required");
    auto symbolElement(list[1]);
    if (not IsSymbol(symbolElement))
        throw CreateException("first argument must be a symbolElement");
    return true;
};

static constexpr auto IsDo = [](const auto& rlwsType)
{
    if (not IsList(rlwsType))
        return false;
    auto list{ValueList(rlwsType)};
    if (IsEmpty(list))
        return false;
    auto doElement{list[0]};
    return (IsSymbol(doElement) and (DO_TOKEN == ValueSymbol(doElement)));
};

static constexpr auto IsLetStar = [](const auto& rlwsType)
{
    auto IsEven = [](const auto n) { return (0 == (n % 2)); };
    auto AllOddFormsAreSymbols = [](const auto& symbolValuePairSequence)
    {
        auto currentPairIndex{0};
        auto maxPairIndex{Count(symbolValuePairSequence) - 1};
        while (currentPairIndex <= maxPairIndex)
        {
            if (not IsSymbol(symbolValuePairSequence[currentPairIndex]))
                throw CreateException("odd forms must be symbols");
            currentPairIndex += 2;
        }
        return true;
    };
    if (not IsList(rlwsType))
        return false;
    auto list{ValueList(rlwsType)};
    if (IsEmpty(list))
        return false;
    auto letStarElement{list[0]};
    if ((not IsSymbol(letStarElement)) or (LETSTAR_TOKEN != ValueSymbol(letStarElement)))
        return false;
    if (3 != Count(list))
        throw CreateException("two arguments required");
    auto localBindingsElement(list[1]);
    if (not(IsList(localBindingsElement) or IsVector(localBindingsElement)))
        throw CreateException("local bindings must be a list or a vector");
    auto symbolValuePairSequence{ValueSequence(localBindingsElement)};
    if (not IsEven(Count(symbolValuePairSequence)))
        throw CreateException("even number of forms required in local bindings");
    if (not AllOddFormsAreSymbols(symbolValuePairSequence))
        throw CreateException("odd forms must be symbols in local bindings");
    return true;
};

} // namespace types

#endif // TYPES_H_
