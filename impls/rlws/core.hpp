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
    static const auto Integer{GenericInteger(T::ADD_TOKEN)};
    auto result{I{0}};
    auto args{T::ValueList(argList)};
    auto argCount{T::Count(args)};
    for (auto i{1}; i < argCount; ++i)
        result += Integer(args[i]);
    return T::CreateInteger(result);
};

static const auto CountFn = [](const auto& argList)
{
    // the first element of argList is the function value
    static const auto fnName{T::S{T::COUNT_TOKEN}};
    auto args{T::ValueList(argList)};
    auto argCount{T::Count(args)};
    if (2 != argCount)
        throw T::CreateException(fnName + ": Wrong number of args (" + std::to_string(argCount - 1) + ")");
    auto parameter{args[1]};
    if (not T::IsList(parameter))
        throw T::CreateException(fnName + ": Parameter must be a list");
    auto parameterList{T::ValueList(parameter)};
    auto parameterCount{T::Count(parameterList)};
    return T::CreateInteger(parameterCount);
};

static const auto DivideFn = [](const auto& argList)
{
    // the first element of argList is the function value
    static const auto Integer{GenericInteger(T::DIVIDE_TOKEN)};
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

static const auto EmptyQFn = [](const auto& argList)
{
    // the first element of argList is the function value
    static const auto fnName{T::S{T::EMPTY_Q_TOKEN}};
    auto args{T::ValueList(argList)};
    auto argCount{T::Count(args)};
    if (2 != argCount)
        throw T::CreateException(fnName + ": Wrong number of args (" + std::to_string(argCount - 1) + ")");
    auto parameter{args[1]};
    if (not T::IsList(parameter))
        throw T::CreateException(fnName + ": Parameter must be a list");
    auto parameterList{T::ValueList(parameter)};
    auto parameterCount{T::Count(parameterList)};
    return (0 == parameterCount) ? T::TrueSymbol : T::FalseSymbol;
};

static const auto ListFn = [](const auto& argList)
{
    // the first element of argList is the function value
    auto args{T::ValueList(argList)};
    auto argCount{T::Count(args)};
    auto result{T::L{}};
    for (auto i{1}; i < argCount; ++i)
        result.push_back(args[i]);
    return T::CreateList(result);
};

static const auto ListQFn = [](const auto& argList)
{
    // the first element of argList is the function value
    static const auto fnName{T::S{T::LIST_Q_TOKEN}};
    auto args{T::ValueList(argList)};
    auto argCount{T::Count(args)};
    if (2 != argCount)
        throw T::CreateException(fnName + ": Wrong number of args (" + std::to_string(argCount - 1) + ")");
    return T::IsList(args[1]) ? T::TrueSymbol : T::FalseSymbol;
};

static const auto MultiplyFn = [](const auto& argList)
{
    // the first element of argList is the function value
    static const auto Integer{GenericInteger(T::MULTIPLY_TOKEN)};
    auto result{I{1}};
    auto args{T::ValueList(argList)};
    auto argCount{T::Count(args)};
    for (auto i{1}; i < argCount; ++i)
        result *= Integer(args[i]);
    return T::CreateInteger(result);
};

static const auto PrStrFn = [](const auto& argList)
{
    // the first element of argList is the function value
    auto args{T::ValueList(argList)};
    auto argCount{T::Count(args)};
    auto result{T::S{""}};
    for (auto i{1}; i < argCount; ++i)
    {
        auto argS{P::PrintStr(args[i], true)};
        result += argS;
        if (i < (argCount - 1))
            result += " ";
    }
    return T::CreateString(result);
};

static const auto PrintlnFn = [](const auto& argList)
{
    // the first element of argList is the function value
    auto args{T::ValueList(argList)};
    auto argCount{T::Count(args)};
    auto s{T::S{""}};
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

static const auto PrnFn = [](const auto& argList)
{
    // the first element of argList is the function value
    auto args{T::ValueList(argList)};
    auto argCount{T::Count(args)};
    auto s{T::S{""}};
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

static const auto StrFn = [](const auto& argList)
{
    // the first element of argList is the function value
    auto args{T::ValueList(argList)};
    auto argCount{T::Count(args)};
    auto result{T::S{""}};
    for (auto i{1}; i < argCount; ++i)
    {
        auto argS{P::PrintStr(args[i], false)};
        result += argS;
    }
    return T::CreateString(result);
};

static const auto SubtractFn = [](const auto& argList)
{
    // the first element of argList is the function value
    static const auto fnName{T::S{T::SUBTRACT_TOKEN}};
    static const auto Integer{GenericInteger(T::SUBTRACT_TOKEN)};
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
