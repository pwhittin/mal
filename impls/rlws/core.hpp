#ifndef CORE_H_
#define CORE_H_

#include <utility>
#include <vector>
#include "env.hpp"
#include "types.hpp"

namespace E = env;
namespace T = types;

using I = T::I;
using RT = T::RLWSType;
using S = T::S;

static constexpr auto GenericInteger = [](const auto& fnName)
{
    return [fnName](const auto& rlwsType)
    {
        if (not T::IsInteger(rlwsType))
            throw T::CreateException(S{fnName} + ": non-integer arg [" + T::RLWSTypeToString(rlwsType) + "]");
        auto result{T::ValueInteger(rlwsType)};
        return result;
    };
};

static const auto AddFn = [](const auto& argList)
{
    // the first element of argList is the function value
    static constexpr auto Integer{GenericInteger("+")};
    auto result{I{0}};
    auto args{T::ValueList(argList)};
    auto argCount{T::Count(args)};
    for (auto i{1}; i < argCount; ++i)
        result += Integer(args[i]);
    return T::CreateInteger(result);
};

static const auto DivideFn = [](const auto& argList)
{
    // the first element of argList is the function value
    static constexpr auto Integer{GenericInteger("/")};
    auto args{T::ValueList(argList)};
    auto argCount{T::Count(args)};
    if (1 == argCount)
        throw T::CreateException("/: Wrong number of args (0)");
    if (2 == argCount)
        return T::CreateInteger(0);
    auto result{Integer(args[1])};
    for (auto i{2}; i < argCount; ++i)
    {
        auto divisor{Integer(args[i])};
        if (I{0} == divisor)
            throw T::CreateException("/: Division by zero");
        result /= divisor;
    }
    return T::CreateInteger(result);
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
    return T::CreateInteger(result);
};

static const auto SubtractFn = [](const auto& argList)
{
    // the first element of argList is the function value
    static constexpr auto Integer{GenericInteger("-")};
    auto args{T::ValueList(argList)};
    auto argCount{T::Count(args)};
    if (1 == argCount)
        throw T::CreateException("/: Wrong number of args (0)");
    if (2 == argCount)
        return T::CreateInteger(-Integer(args[1]));
    auto result{Integer(args[1])};
    for (auto i{2}; i < argCount; ++i)
        result -= Integer(args[i]);
    return T::CreateInteger(result);
};

static constexpr auto NsAddToEnv = [](const auto& ns, auto& e)
{
    for (const auto& [symbol, function] : ns)
        E::Set(symbol, function, e);
};

using SymbolFunctionPair = std::pair<RT, RT>;
using Ns = std::vector<SymbolFunctionPair>;

static constexpr auto NsCreateSymbolFunctionPair = [](const auto& symbol, const auto& function) {
    return SymbolFunctionPair{T::CreateSymbol(symbol), T::CreateFunction(function)};
};

namespace core
{

static constexpr auto SFP{NsCreateSymbolFunctionPair};
static auto ns{Ns{SFP(T::ADD_TOKEN, AddFn),
                  SFP(T::DIVIDE_TOKEN, DivideFn),
                  SFP(T::MULTIPLY_TOKEN, MultiplyFn),
                  SFP(T::SUBTRACT_TOKEN, SubtractFn)}};

static constexpr auto AddToEnv{NsAddToEnv};

} // namespace core

#endif // CORE_H_
