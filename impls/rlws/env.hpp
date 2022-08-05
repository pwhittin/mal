#ifndef ENV_H_
#define ENV_H_

#include <functional>
#include <span>
#include <unordered_map>
#include "types.hpp"

namespace T = types;

using I = T::I;
using RT = T::RLWSType;
using RTS = T::RLWSTypes;
using S = T::S;

using EnvData = std::unordered_map<S, RT>;
struct Env
{
    Env* outer{nullptr};
    EnvData data{};
};

static constexpr auto EnvCreate = [](Env* const o)
{
    auto result{Env{}};
    result.outer = o;
    return result;
};

static constexpr auto EnvSet = [](const auto& rlwsSymbol, const auto& rlwsType, auto& e)
{
    auto symbol{T::ValueSymbol(rlwsSymbol)};
    e.data.insert_or_assign(symbol, rlwsType);
};

static Env EnvFind(const auto& rlwsSymbol, const auto& e)
{
    auto symbol{T::ValueSymbol(rlwsSymbol)};
    auto ePair{e.data.find(symbol)};
    auto symbolNotFound{e.data.end() == ePair};
    auto outermostEnv{nullptr == e.outer};
    return (symbolNotFound or outermostEnv) ? e : EnvFind(rlwsSymbol, *e.outer);
};

static constexpr auto EnvGet = [](const auto& rlwsSymbol, const auto& e)
{
    auto eFound{EnvFind(rlwsSymbol, e)};
    auto symbol{T::ValueSymbol(rlwsSymbol)};
    auto eFoundPair{eFound.data.find(symbol)};
    auto symbolNotFound{eFound.data.end() == eFoundPair};
    if (symbolNotFound)
        throw std::invalid_argument("Symbol \"" + symbol + "\" not found");
    return eFoundPair->second;
};

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

namespace env
{

static constexpr auto Create{EnvCreate};

static auto repl_env{Env{}};
static constexpr auto Init = []()
{
    EnvSet(T::CreateSymbol("+"), T::CreateFunction(AddFn), repl_env);
    EnvSet(T::CreateSymbol("-"), T::CreateFunction(SubtractFn), repl_env);
    EnvSet(T::CreateSymbol("*"), T::CreateFunction(MultiplyFn), repl_env);
    EnvSet(T::CreateSymbol("/"), T::CreateFunction(DivideFn), repl_env);
};

static constexpr auto Get{EnvGet};
static constexpr auto Set{EnvSet};

} // namespace env

#endif // ENV_H_
