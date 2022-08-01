#include <iostream>
#include <stdexcept>
#include <string>

#include "printer.hpp"
#include "reader.hpp"
#include "types.hpp"

auto READ = [](const auto& s) { return reader::ReadStr(s); };
auto EVAL = [](const auto& rlwsType) { return rlwsType; };
auto PRINT = [](const auto& rlwsType) { return printer::PrintStr(rlwsType); };
auto rep = [](const auto& s) { return PRINT(EVAL(READ(s))); };

static const auto PROMPT{types::S{"user> "}};

const auto ReadLine = [](auto& in) noexcept
{
    auto s{types::S{""}};
    std::getline(in, s);
    return s;
};

const auto IsEOF = [](const auto& in) noexcept { return in.eof(); };

int main()
{
    while (not IsEOF(std::cin))
    {
        std::cout << PROMPT;
        try
        {
            std::cout << rep(ReadLine(std::cin)) << "\n";
        }
        catch (const std::invalid_argument& ia)
        {
            std::cout << ia.what() << "\n";
        }
    }

    return 0;
}
