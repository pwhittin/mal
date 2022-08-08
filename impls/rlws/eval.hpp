#ifndef EVAL_H_
#define EVAL_H_

#include "env.hpp"
#include "reader.hpp"
#include "types.hpp"

namespace E = env;
namespace T = types;

using RT = T::RLWSType;
using RTS = T::RLWSTypes;

static RT Evaluate(const RT& rlwsType, auto& e);

static constexpr auto EvalSequence = [](const auto& rlwsType, auto& e)
{
    auto resultList{L{}};
    auto list{T::ValueList(rlwsType)};
    for (const auto& v : list)
        resultList.push_back(Evaluate(v, e));
    return T::CreateRLWSType(rlwsType.type, resultList);
};

static constexpr auto EvalAst = [](const auto& rlwsType, auto& e)
{
    switch (rlwsType.type)
    {
    case RTS::RLWS_SYMBOL:
        return E::Get(rlwsType, e);
    case RTS::RLWS_LIST:
    case RTS::RLWS_MAP:
    case RTS::RLWS_VECTOR:
        return EvalSequence(rlwsType, e);
    }
    return rlwsType;
};

static constexpr auto Apply = [](const auto& rlwsType, auto& e)
{
    auto rlwsTypeEvaluated{EvalSequence(rlwsType, e)};
    auto listEvaluated{T::ValueList(rlwsTypeEvaluated)};
    auto rlwsFunction{T::First(listEvaluated)};
    if (not T::IsFunction(rlwsFunction))
        throw std::invalid_argument("Call: first arg not a function [" + T::RLWSTypeToString(rlwsFunction) + "]");
    auto function{T::ValueFunction(rlwsFunction)};
    return function(rlwsTypeEvaluated);
};

static constexpr auto DefBang = [](const auto& rlwsType, auto& e)
{
    auto list{T::ValueList(rlwsType)};
    auto symbol{list[1]};
    auto value{list[2]};
    auto evaluatedValue{Evaluate(value, e)};
    return env::Set(symbol, evaluatedValue, e);
};

static constexpr auto LetStar = [](const auto& rlwsType, auto& e)
{
    auto AddSymbolValuePairsToLocalEnv = [](const auto& symbolValuePairSequence, auto& localEnv)
    {
        auto i{0};
        auto n{T::Count(symbolValuePairSequence)};
        while (i < n)
        {
            env::Set(symbolValuePairSequence[i], Evaluate(symbolValuePairSequence[i + 1], localEnv), localEnv);
            i += 2;
        }
    };
    auto list{T::ValueList(rlwsType)};
    auto symbolValuePairSequence{T::ValueSequence(list[1])};
    auto localEnv{env::Create(&e)};
    AddSymbolValuePairsToLocalEnv(symbolValuePairSequence, localEnv);
    return Evaluate(list[2], localEnv);
};

static constexpr auto Call = [](const auto& rlwsType, auto& e)
{
    return T::IsDefBang(rlwsType)   ? DefBang(rlwsType, e)
           : T::IsLetStar(rlwsType) ? LetStar(rlwsType, e)
                                    : Apply(rlwsType, e);
};

static RT Evaluate(const RT& rlwsType, auto& e)
{
    return (not T::IsList(rlwsType))            ? EvalAst(rlwsType, e)
           : T::IsEmpty(T::ValueList(rlwsType)) ? rlwsType
                                                : Call(rlwsType, e);
};

namespace eval
{

static constexpr auto Eval = [](const auto& rlwsType) { return Evaluate(rlwsType, env::repl_env); };

} // namespace eval

#endif // EVAL_H_
