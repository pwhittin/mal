#ifndef PRINTER_H_
#define PRINTER_H_

#include <string>
#include <utility>
#include "types.hpp"

auto SequenceTokens = [](const auto type)
{
    switch (type)
    {
    case types::RLWSTypes::RLWS_LIST:
        return std::pair<types::S, types::S>{types::LIST_TOKEN_START, types::LIST_TOKEN_END};
    case types::RLWSTypes::RLWS_MAP:
        return std::pair<types::S, types::S>{types::MAP_TOKEN_START, types::MAP_TOKEN_END};
    case types::RLWSTypes::RLWS_VECTOR:
        return std::pair<types::S, types::S>{types::VECTOR_TOKEN_START, types::VECTOR_TOKEN_END};
    }
    return std::pair<types::S, types::S>{"", ""}; // quiets compiler warning
};

namespace printer
{

types::S PrintStr(const types::RLWSType& rlwsType)
{
    auto result{types::S{""}};
    switch (rlwsType.type)
    {
    case types::RLWSTypes::RLWS_INTEGER:
        result = std::to_string(std::get<types::I>(rlwsType.value));
        break;
    case types::RLWSTypes::RLWS_STRING:
        result = "\"" + std::get<types::S>(rlwsType.value) + "\"";
        break;
    case types::RLWSTypes::RLWS_SYMBOL:
        result = std::get<types::S>(rlwsType.value);
        break;
    case types::RLWSTypes::RLWS_LIST:
    case types::RLWSTypes::RLWS_MAP:
    case types::RLWSTypes::RLWS_VECTOR:
        auto [startToken, endToken]{SequenceTokens(rlwsType.type)};
        auto sequence{std::get<types::L>(rlwsType.value)};
        result += startToken;
        for (const auto& v : sequence)
            result += PrintStr(v) + " ";
        if (' ' == *result.rbegin())
            *result.rbegin() = endToken[0];
        else
            result += endToken;
        break;
    }
    return result;
}

} // namespace printer

#endif // PRINTER_H_
