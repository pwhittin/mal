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

namespace env
{

static const auto FalseSymbol{T::CreateSymbol(T::FALSE_TOKEN)};
static const auto NilSymbol{T::CreateSymbol(T::NIL_TOKEN)};
static const auto TrueSymbol{T::CreateSymbol(T::TRUE_TOKEN)};

static auto repl_env{Env{}};
static constexpr auto Init = []()
{
    EnvSet(FalseSymbol, T::FalseSymbol, repl_env);
    EnvSet(NilSymbol, T::NilSymbol, repl_env);
    EnvSet(TrueSymbol, T::TrueSymbol, repl_env);
};

static constexpr auto Create{EnvCreate};
static constexpr auto CreateWithBindsAndExprs{EnvCreateWithBindsAndExprs};
static constexpr auto Get{EnvGet};
static constexpr auto Set{EnvSet};

} // namespace env

#endif // ENV_H_
