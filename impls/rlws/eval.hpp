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

static constexpr auto EvaluateSequence = [](const auto& rlwsType, auto& e, const auto numberToSkip)
{
    auto skipCount{0};
    auto resultList{L{}};
    auto list{T::ValueList(rlwsType)};
    for (const auto& v : list)
        if ((++skipCount) > numberToSkip)
            resultList.push_back(Evaluate(v, e));
    return T::CreateRLWSType(rlwsType.type, resultList);
};

static constexpr auto EvalSequence = [](const auto& rlwsType, auto& e) { return EvaluateSequence(rlwsType, e, 0); };

static constexpr auto EvalRestOfSequence = [](const auto& rlwsType, auto& e)
{ return EvaluateSequence(rlwsType, e, 1); };

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
        throw T::CreateException("Apply: first arg not a function [" + T::RLWSTypeToString(rlwsFunction) + "]");
    auto function{T::ValueFunction(rlwsFunction)};
    return function(rlwsTypeEvaluated);
};

static constexpr auto DefBang = [](const auto& rlwsType, auto& e)
{
    // this function expects rlwsType to have been validated with types::IsDefBang
    // handles '(def! symbol value_form)'
    static constexpr auto SYMBOL_LIST_INDEX{1};
    static constexpr auto VALUE_LIST_INDEX{2};
    auto list{T::ValueList(rlwsType)};
    auto symbol{list[SYMBOL_LIST_INDEX]};
    auto value{list[VALUE_LIST_INDEX]};
    auto evaluatedValue{Evaluate(value, e)};
    return E::Set(symbol, evaluatedValue, e);
};

static constexpr auto Do = [](const auto& rlwsType, auto& e)
{
    // this function expects rlwsType to have been validated with types::IsDo
    // handles '(do)' or '(do form_1 ... form_n)'
    static constexpr auto FORM_1_LIST_INDEX{1};
    auto list{T::ValueList(rlwsType)};
    auto listCount{T::Count(list)};
    auto result{T::NilSymbol};
    if (1 == listCount) // check for '(do)'
        return result;
    for (auto i{FORM_1_LIST_INDEX}; i < listCount; ++i)
        result = Evaluate(list[i], e);
    return result;
};

static constexpr auto FnStarFunction = [](const auto& params, const auto& argList, const auto& form, auto& e)
{
    auto paramsSequence{T::ValueSequence(params)};
    auto paramsCount{T::Count(paramsSequence)};
    auto argsSequence{T::ValueSequence(argList)};
    auto argsCount{T::Count(argsSequence) - 1}; // argsSequence includes the fn* form
    if (argsCount != paramsCount)
        throw T::CreateException("parameter and argument count mismatch");
    auto evaluatedArgList{EvalRestOfSequence(argList, e)};
    auto localEnv{E::CreateWithBindsAndExprs(&e, params, evaluatedArgList)};
    return Evaluate(form, localEnv);
};

static constexpr auto FnStar = [](const auto& rlwsType, auto& e)
{
    // this function expects rlwsType to have been validated with types::IsFnStar
    // handles '(fn* () form)' or '(fn* (param_1 ... param_n) form)' or
    //         '(fn* [] form)' or '(fn* [param_1 ... param_n] form)'
    static constexpr auto PARAMS_LIST_INDEX{1};
    static constexpr auto FORM_LIST_INDEX{2};
    auto list{T::ValueList(rlwsType)};
    auto params{list[PARAMS_LIST_INDEX]};
    auto form{list[FORM_LIST_INDEX]};
    auto function{[params, form, &e](const auto& argList) { return FnStarFunction(params, argList, form, e); }};
    return T::CreateFunction(function);
};

static constexpr auto If = [](const auto& rlwsType, auto& e)
{
    // this function expects rlwsType to have been validated with types::IsIf
    // handles '(if predicate_form true_form)' or '(if predicate_form true_form false_form)'
    static constexpr auto PREDICATE_LIST_INDEX{1};
    static constexpr auto TRUE_FORM_LIST_INDEX{2};
    static constexpr auto FALSE_FORM_LIST_INDEX{3};
    auto list{T::ValueList(rlwsType)};
    auto predicateForm{list[PREDICATE_LIST_INDEX]};
    auto predicateValue{Evaluate(predicateForm, e)};
    if ((not T::EqualSymbols(T::NilSymbol, predicateValue)) and (not T::EqualSymbols(T::FalseSymbol, predicateValue)))
        return Evaluate(list[TRUE_FORM_LIST_INDEX], e);
    return (3 == T::Count(list)) ? T::NilSymbol : Evaluate(list[FALSE_FORM_LIST_INDEX], e);
};

static constexpr auto LetStar = [](const auto& rlwsType, auto& e)
{
    // this function expects rlwsType to have been validated with types::IsLetStar
    // handles '(let* [symbol_1 value_1_form ... symbol_n value_n_form] form)' or
    //         '(let* (symbol_1 value_1_form ... symbol_n value_n_form) form)'
    static constexpr auto PREDICATE_LIST_INDEX{1};
    auto AddSymbolValuePairsToLocalEnv = [](const auto& symbolValuePairSequence, auto& localEnv)
    {
        auto currentSymbolValuePairIndex{0};
        auto maxSymbolValuePairIndex{T::Count(symbolValuePairSequence) - 1};
        while (currentSymbolValuePairIndex <= maxSymbolValuePairIndex)
        {
            E::Set(symbolValuePairSequence[currentSymbolValuePairIndex],
                   Evaluate(symbolValuePairSequence[currentSymbolValuePairIndex + 1], localEnv),
                   localEnv);
            currentSymbolValuePairIndex += 2;
        }
    };
    auto list{T::ValueList(rlwsType)};
    auto symbolValuePairSequence{T::ValueSequence(list[1])};
    auto localEnv{E::Create(&e)};
    AddSymbolValuePairsToLocalEnv(symbolValuePairSequence, localEnv);
    return Evaluate(list[2], localEnv);
};

static constexpr auto Call = [](const auto& rlwsType, auto& e)
{
    return T::IsDefBang(rlwsType)   ? DefBang(rlwsType, e)
           : T::IsDo(rlwsType)      ? Do(rlwsType, e)
           : T::IsFnStar(rlwsType)  ? FnStar(rlwsType, e)
           : T::IsIf(rlwsType)      ? If(rlwsType, e)
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

static constexpr auto Eval = [](const auto& rlwsType) { return Evaluate(rlwsType, E::repl_env); };

} // namespace eval

#endif // EVAL_H_
