#ifndef ENV_H_
#define ENV_H_

#include <iostream>
#include <functional>
#include <span>
#include <unordered_map>
#include "types.hpp"

namespace T = types;

using I = T::I;
using RT = T::RLWSType;
using RTS = T::RLWSTypes;
using S = T::S;

using Env = std::unordered_map<S, RT>;

static constexpr auto GenericInteger = [](const auto& fnName)
{
    return [fnName](const auto& rlwsType)
    {
        if (not T::IsInteger(rlwsType))
            throw std::invalid_argument(S{fnName} + ": non-integer arg [" + T::RLWSTypeToString(rlwsType) + "]");
        auto result{T::ValueInteger(rlwsType)};
        return result;
    };
};

static const auto CreateInteger = [](const auto& value) { return T::CreateRLWSType(RTS::RLWS_INTEGER, value); };

static const auto AddFn = [](const auto& argList)
{
    // the first element of argList is the function value
    static constexpr auto Integer{GenericInteger("+")};
    auto result{I{0}};
    auto args{T::ValueList(argList)};
    auto argCount{T::Count(args)};
    for (auto i{1}; i < argCount; ++i)
        result += Integer(args[i]);
    return CreateInteger(result);
};

static const auto SubtractFn = [](const auto& argList)
{
    // the first element of argList is the function value
    static constexpr auto Integer{GenericInteger("-")};
    auto args{T::ValueList(argList)};
    auto argCount{T::Count(args)};
    if (1 == argCount)
        throw std::invalid_argument("/: Wrong number of args (0)");
    if (2 == argCount)
        return CreateInteger(-Integer(args[1]));
    auto result{Integer(args[1])};
    for (auto i{2}; i < argCount; ++i)
        result -= Integer(args[i]);
    return CreateInteger(result);
};

static const auto MultiplyFn = [](const auto& argList)
{
    // the first element of argList is the function value
    static constexpr auto Integer{GenericInteger("*")};
    auto result{I{1}};
    auto args{T::ValueList(argList)};
    auto argCount{T::Count(args)};
    for (auto i{1}; i < argCount; ++i)
        result *= Integer(args[i]);
    return CreateInteger(result);
};

static const auto DivideFn = [](const auto& argList)
{
    // the first element of argList is the function value
    static constexpr auto Integer{GenericInteger("/")};
    auto args{T::ValueList(argList)};
    auto argCount{T::Count(args)};
    if (1 == argCount)
        throw std::invalid_argument("/: Wrong number of args (0)");
    if (2 == argCount)
        return CreateInteger(0);
    auto result{Integer(args[1])};
    for (auto i{2}; i < argCount; ++i)
    {
        auto divisor{Integer(args[i])};
        if (I{0} == divisor)
            throw std::invalid_argument("/: Division by zero");
        result /= divisor;
    }
    return CreateInteger(result);
};

static constexpr auto CreateFunction = [](const auto fn) { return T::CreateRLWSType(RTS::RLWS_FUNCTION, fn); };

static constexpr auto LookupFailed = [](const auto& env, const auto& envElement) { return (env.end() == envElement); };

namespace env
{

static const auto repl_env{Env{{"+", CreateFunction(AddFn)},
                               {"-", CreateFunction(SubtractFn)},
                               {"*", CreateFunction(MultiplyFn)},
                               {"/", CreateFunction(DivideFn)}}};

static constexpr auto Lookup = [](const auto& rlwsType, const auto& env)
{
    auto symbol{T::ValueSymbol(rlwsType)};
    auto envElement{env.find(symbol)};
    if (LookupFailed(env, envElement))
        throw std::invalid_argument("unbound symbol: '" + symbol + "'");
    return envElement->second;
};

} // namespace env

#endif // ENV_H_
