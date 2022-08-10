#ifndef ENV_H_
#define ENV_H_

#include <functional>
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

static constexpr auto EnvCreate = [](Env* const outerEnv)
{
    auto result{Env{}};
    result.outer = outerEnv;
    return result;
};

static constexpr auto EnvSet = [](const auto& rlwsSymbol, const auto& rlwsType, Env& e)
{
    T::S symbol{T::ValueSymbol(rlwsSymbol)};
    e.data.insert_or_assign(symbol, rlwsType);
    return rlwsType;
};

static constexpr auto EnvCreateWithBindsAndExprs =
    [](Env* const outerEnv, const auto& bindsSequence, const auto& exprsSequence)
{
    auto result{EnvCreate(outerEnv)};
    auto binds{T::ValueSequence(bindsSequence)};
    auto exprs{T::ValueSequence(exprsSequence)};
    auto n{T::Count(binds)};
    for (auto i{0}; i < n; ++i)
        EnvSet(binds[i], exprs[i], result);
    return result;
};

static Env EnvFind(const auto& rlwsSymbol, const auto& e)
{
    auto symbol{T::ValueSymbol(rlwsSymbol)};
    auto ePair{e.data.find(symbol)};
    auto outermostEnv{nullptr == e.outer};
    auto symbolFound{e.data.end() != ePair};
    return (outermostEnv or symbolFound) ? e : EnvFind(rlwsSymbol, *e.outer);
};

static constexpr auto EnvGet = [](const auto& rlwsSymbol, const auto& e)
{
    auto eFound{EnvFind(rlwsSymbol, e)};
    auto symbol{T::ValueSymbol(rlwsSymbol)};
    auto eFoundPair{eFound.data.find(symbol)};
    auto symbolNotFound{eFound.data.end() == eFoundPair};
    if (symbolNotFound)
        throw T::CreateException("Symbol '" + symbol + "' not found");
    return eFoundPair->second;
};

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

static const auto DivideFn = [](const auto& argList)
{
    // the first element of argList is the function value
    static constexpr auto Integer{GenericInteger("/")};
    auto args{T::ValueList(argList)};
    auto argCount{T::Count(args)};
    if (1 == argCount)
        throw T::CreateException("/: Wrong number of args (0)");
    if (2 == argCount)
        return CreateInteger(0);
    auto result{Integer(args[1])};
    for (auto i{2}; i < argCount; ++i)
    {
        auto divisor{Integer(args[i])};
        if (I{0} == divisor)
            throw T::CreateException("/: Division by zero");
        result /= divisor;
    }
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

static const auto SubtractFn = [](const auto& argList)
{
    // the first element of argList is the function value
    static constexpr auto Integer{GenericInteger("-")};
    auto args{T::ValueList(argList)};
    auto argCount{T::Count(args)};
    if (1 == argCount)
        throw T::CreateException("/: Wrong number of args (0)");
    if (2 == argCount)
        return CreateInteger(-Integer(args[1]));
    auto result{Integer(args[1])};
    for (auto i{2}; i < argCount; ++i)
        result -= Integer(args[i]);
    return CreateInteger(result);
};

namespace env
{

static auto repl_env{Env{}};
static constexpr auto Init = []()
{
    EnvSet(T::CreateSymbol(T::ADD_TOKEN), T::CreateFunction(AddFn), repl_env);
    EnvSet(T::CreateSymbol(T::FALSE_TOKEN), T::FalseSymbol, repl_env);
    EnvSet(T::CreateSymbol(T::DIVIDE_TOKEN), T::CreateFunction(DivideFn), repl_env);
    EnvSet(T::CreateSymbol(T::MULTIPLY_TOKEN), T::CreateFunction(MultiplyFn), repl_env);
    EnvSet(T::CreateSymbol(T::NIL_TOKEN), T::NilSymbol, repl_env);
    EnvSet(T::CreateSymbol(T::SUBTRACT_TOKEN), T::CreateFunction(SubtractFn), repl_env);
    EnvSet(T::CreateSymbol(T::TRUE_TOKEN), T::TrueSymbol, repl_env);
};

static constexpr auto Create{EnvCreate};
static constexpr auto CreateWithBindsAndExprs{EnvCreateWithBindsAndExprs};
static constexpr auto Get{EnvGet};
static constexpr auto Set{EnvSet};

} // namespace env

#endif // ENV_H_
