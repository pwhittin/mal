#ifndef CORE_H_
#define CORE_H_

#include <iostream>
#include <utility>
#include <vector>
#include "env.hpp"
#include "printer.hpp"
#include "types.hpp"

namespace E = env;
namespace P = printer;
namespace T = types;

using I = T::I;
using L = T::L;
using RT = T::RLWSType;
using S = T::S;

static constexpr auto TrueOrFalse = [](const bool tOrF) { return tOrF ? T::TrueSymbol : T::FalseSymbol; };

static constexpr auto GenericInteger = [](const auto& fnName)
{
    return [fnName](const auto& rlwsType)
    {
        if (not T::IsInteger(rlwsType))
            throw T::CreateException(fnName + ": non-integer arg [" + T::RLWSTypeToString(rlwsType) + "]");
        auto result{T::ValueInteger(rlwsType)};
        return result;
    };
};

static constexpr auto GenericRelationalIntegerFn = [](const auto& argList, const auto& fnName)
{
    // the first element of argList is the function value
    auto args{T::ValueList(argList)};
    auto argCount{T::Count(args)};
    if (3 != argCount)
        throw T::CreateException(fnName + ": Wrong number of args (" + std::to_string(argCount - 1) + ")");
    auto parameter1{args[1]};
    auto parameter2{args[2]};
    auto Integer{GenericInteger(fnName)};
    return std::pair<RT, RT>{T::CreateInteger(Integer(parameter1)), T::CreateInteger(Integer(parameter2))};
};

static constexpr auto AddFn = [](const auto& argList)
{
    // the first element of argList is the function value
    static const auto Integer{GenericInteger(S{T::ADD_TOKEN})};
    auto result{I{0}};
    auto args{T::ValueList(argList)};
    auto argCount{T::Count(args)};
    for (auto i{1}; i < argCount; ++i)
        result += Integer(args[i]);
    return T::CreateInteger(result);
};

static constexpr auto CountFn = [](const auto& argList)
{
    // the first element of argList is the function value
    static const auto fnName{S{T::COUNT_TOKEN}};
    auto args{T::ValueList(argList)};
    auto argCount{T::Count(args)};
    if (2 != argCount)
        throw T::CreateException(fnName + ": Wrong number of args (" + std::to_string(argCount - 1) + ")");
    auto parameter{args[1]};
    if (T::EqualSymbols(parameter, T::NilSymbol))
        return T::CreateInteger(0);
    if (not T::IsList(parameter))
        throw T::CreateException(fnName + ": Parameter must be a list");
    auto parameterList{T::ValueList(parameter)};
    auto parameterCount{T::Count(parameterList)};
    return T::CreateInteger(parameterCount);
};

static constexpr auto DivideFn = [](const auto& argList)
{
    // the first element of argList is the function value
    static const auto Integer{GenericInteger(S{T::DIVIDE_TOKEN})};
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

static constexpr auto EmptyQFn = [](const auto& argList)
{
    // the first element of argList is the function value
    static const auto fnName{S{T::EMPTY_Q_TOKEN}};
    auto args{T::ValueList(argList)};
    auto argCount{T::Count(args)};
    if (2 != argCount)
        throw T::CreateException(fnName + ": Wrong number of args (" + std::to_string(argCount - 1) + ")");
    auto parameter{args[1]};
    if (not T::IsList(parameter))
        throw T::CreateException(fnName + ": Parameter must be a list");
    auto parameterList{T::ValueList(parameter)};
    auto parameterCount{T::Count(parameterList)};
    return TrueOrFalse(0 == parameterCount);
};

static constexpr auto EqualFn = [](const auto& argList)
{
    // the first element of argList is the function value
    static const auto fnName{S{T::EQUAL_TOKEN}};
    auto args{T::ValueList(argList)};
    auto argCount{T::Count(args)};
    if (3 != argCount)
        throw T::CreateException(fnName + ": Wrong number of args (" + std::to_string(argCount - 1) + ")");
    auto parameter1{args[1]};
    auto parameter2{args[2]};
    return T::EqualRLWSTypes(parameter1, parameter2);
};

static constexpr auto GreaterThanFn = [](const auto& argList)
{
    static const auto fnName{S{T::LESS_THAN_TOKEN}};
    auto [parameter1, parameter2]{GenericRelationalIntegerFn(argList, fnName)};
    return TrueOrFalse(T::ValueInteger(parameter1) > T::ValueInteger(parameter2));
};

static constexpr auto GreaterThanOrEqualFn = [](const auto& argList)
{
    static const auto fnName{S{T::LESS_THAN_TOKEN}};
    auto [parameter1, parameter2]{GenericRelationalIntegerFn(argList, fnName)};
    return TrueOrFalse(T::ValueInteger(parameter1) >= T::ValueInteger(parameter2));
};

static constexpr auto LessThanFn = [](const auto& argList)
{
    static const auto fnName{S{T::LESS_THAN_TOKEN}};
    auto [parameter1, parameter2]{GenericRelationalIntegerFn(argList, fnName)};
    return TrueOrFalse(T::ValueInteger(parameter1) < T::ValueInteger(parameter2));
};

static constexpr auto LessThanOrEqualFn = [](const auto& argList)
{
    static const auto fnName{S{T::LESS_THAN_TOKEN}};
    auto [parameter1, parameter2]{GenericRelationalIntegerFn(argList, fnName)};
    return TrueOrFalse(T::ValueInteger(parameter1) <= T::ValueInteger(parameter2));
};

static constexpr auto ListFn = [](const auto& argList)
{
    // the first element of argList is the function value
    auto args{T::ValueList(argList)};
    auto argCount{T::Count(args)};
    auto result{T::L{}};
    for (auto i{1}; i < argCount; ++i)
        result.push_back(args[i]);
    return T::CreateList(result);
};

static constexpr auto ListQFn = [](const auto& argList)
{
    // the first element of argList is the function value
    static const auto fnName{S{T::LIST_Q_TOKEN}};
    auto args{T::ValueList(argList)};
    auto argCount{T::Count(args)};
    if (2 != argCount)
        throw T::CreateException(fnName + ": Wrong number of args (" + std::to_string(argCount - 1) + ")");
    return T::IsList(args[1]) ? T::TrueSymbol : T::FalseSymbol;
};

static constexpr auto MultiplyFn = [](const auto& argList)
{
    // the first element of argList is the function value
    static const auto Integer{GenericInteger(S{T::MULTIPLY_TOKEN})};
    auto result{I{1}};
    auto args{T::ValueList(argList)};
    auto argCount{T::Count(args)};
    for (auto i{1}; i < argCount; ++i)
        result *= Integer(args[i]);
    return T::CreateInteger(result);
};

static constexpr auto PrStrFn = [](const auto& argList)
{
    // the first element of argList is the function value
    auto args{T::ValueList(argList)};
    auto argCount{T::Count(args)};
    auto result{S{""}};
    for (auto i{1}; i < argCount; ++i)
    {
        auto argS{P::PrintStr(args[i], true)};
        result += argS;
        if (i < (argCount - 1))
            result += " ";
    }
    return T::CreateString(result);
};

static constexpr auto PrintlnFn = [](const auto& argList)
{
    // the first element of argList is the function value
    auto args{T::ValueList(argList)};
    auto argCount{T::Count(args)};
    auto s{S{""}};
    for (auto i{1}; i < argCount; ++i)
    {
        auto argS{P::PrintStr(args[i], false)};
        s += argS;
        if (i < (argCount - 1))
            s += " ";
    }
    std::cout << s << "\n";
    return E::NilSymbol;
};

static constexpr auto PrnFn = [](const auto& argList)
{
    // the first element of argList is the function value
    auto args{T::ValueList(argList)};
    auto argCount{T::Count(args)};
    auto s{S{""}};
    for (auto i{1}; i < argCount; ++i)
    {
        auto argS{P::PrintStr(args[i], true)};
        s += argS;
        if (i < (argCount - 1))
            s += " ";
    }
    std::cout << s << "\n";
    return E::NilSymbol;
};

static constexpr auto StrFn = [](const auto& argList)
{
    // the first element of argList is the function value
    auto args{T::ValueList(argList)};
    auto argCount{T::Count(args)};
    auto result{S{""}};
    for (auto i{1}; i < argCount; ++i)
    {
        auto argS{P::PrintStr(args[i], false)};
        result += argS;
    }
    return T::CreateString(result);
};

static constexpr auto SubtractFn = [](const auto& argList)
{
    // the first element of argList is the function value
    static const auto fnName{S{T::SUBTRACT_TOKEN}};
    static const auto Integer{GenericInteger(S{T::SUBTRACT_TOKEN})};
    auto args{T::ValueList(argList)};
    auto argCount{T::Count(args)};
    if (1 == argCount)
        throw T::CreateException(fnName + ": Wrong number of args (0)");
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
                  SFP(T::COUNT_TOKEN, CountFn),
                  SFP(T::DIVIDE_TOKEN, DivideFn),
                  SFP(T::EMPTY_Q_TOKEN, EmptyQFn),
                  SFP(T::EQUAL_TOKEN, EqualFn),
                  SFP(T::GREATER_THAN_TOKEN, GreaterThanFn),
                  SFP(T::GREATER_THAN_OR_EQUAL_TOKEN, GreaterThanOrEqualFn),
                  SFP(T::LESS_THAN_TOKEN, LessThanFn),
                  SFP(T::LESS_THAN_OR_EQUAL_TOKEN, LessThanOrEqualFn),
                  SFP(T::LIST_TOKEN, ListFn),
                  SFP(T::LIST_Q_TOKEN, ListQFn),
                  SFP(T::MULTIPLY_TOKEN, MultiplyFn),
                  SFP(T::PR_STR_TOKEN, PrStrFn),
                  SFP(T::PRINTLN_TOKEN, PrintlnFn),
                  SFP(T::PRN_TOKEN, PrnFn),
                  SFP(T::STR_TOKEN, StrFn),
                  SFP(T::SUBTRACT_TOKEN, SubtractFn)}};

static constexpr auto AddToEnv{NsAddToEnv};

} // namespace core

#endif // CORE_H_
