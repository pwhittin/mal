#ifndef TYPES_H_
#define TYPES_H_

#include <cstdint>
#include <functional>
#include <span>
#include <string>
#include <variant>
#include <vector>

namespace types
{

static const auto DEREF_TOKEN{"@"};
static const auto KEYWORD_TOKEN{"\xFF"};
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

} // namespace types

#endif // TYPES_H_
