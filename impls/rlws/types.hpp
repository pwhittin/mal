#ifndef TYPES_H_
#define TYPES_H_

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

namespace types
{

static const auto LIST_TOKEN_END{")"};
static const auto LIST_TOKEN_START{"("};
static const auto MAP_TOKEN_END{"}"};
static const auto MAP_TOKEN_START{"{"};
static const auto STRING_CHARACTER_DELIMITER{'"'};
static const auto STRING_TOKEN_DELIMITER{"\""};
static const auto VECTOR_TOKEN_END{"]"};
static const auto VECTOR_TOKEN_START{"["};

struct RLWSType;

using I = intmax_t;
using S = std::string;
using L = std::vector<RLWSType>;
using V = std::variant<I, L, S>;

enum class RLWSTypes
{
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

} // namespace types

#endif // TYPES_H_
