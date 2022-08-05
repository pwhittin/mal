#ifndef EVAL_H_
#define EVAL_H_

#include "env.hpp"
#include "reader.hpp"
#include "types.hpp"

namespace E = env;
namespace T = types;

using RT = T::RLWSType;
using RTS = T::RLWSTypes;

static RT Evaluate(const RT& rlwsType);

static constexpr auto EvalList = [](const auto& rlwsType, const auto& e)
{
    auto resultList{L{}};
    auto list{T::ValueList(rlwsType)};
    for (const auto& v : list)
        resultList.push_back(Evaluate(v));
    return T::CreateRLWSType(RTS::RLWS_LIST, resultList);
};
static constexpr auto EvalAst = [](const auto& rlwsType, const auto& e) {
    return T::IsSymbol(rlwsType) ? E::Lookup(rlwsType, e) : T::IsList(rlwsType) ? EvalList(rlwsType, e) : rlwsType;
};

static constexpr auto Call = [](const auto& rlwsType)
{
    auto rlwsTypeEvaluated{EvalList(rlwsType, E::repl_env)};
    auto listEvaluated{T::ValueList(rlwsTypeEvaluated)};
    auto rlwsFunction{T::First(listEvaluated)};
    if (not T::IsFunction(rlwsFunction))
        throw std::invalid_argument("Call: first arg not a function [" + T::RLWSTypeToString(rlwsFunction) + "]");
    auto function{T::ValueFunction(rlwsFunction)};
    return function(rlwsTypeEvaluated);
};

static RT Evaluate(const RT& rlwsType)
{
    return (not T::IsList(rlwsType))            ? EvalAst(rlwsType, env::repl_env)
           : T::IsEmpty(T::ValueList(rlwsType)) ? rlwsType
                                                : Call(rlwsType);
};

namespace eval
{

static constexpr auto Eval = [](const auto& rlwsType)
{
    return (not T::IsList(rlwsType))            ? EvalAst(rlwsType, env::repl_env)
           : T::IsEmpty(T::ValueList(rlwsType)) ? rlwsType
                                                : Call(rlwsType);
};

} // namespace eval

#endif // EVAL_H_
