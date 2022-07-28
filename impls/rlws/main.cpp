#include <iostream>
#include <string>

using S = std::string;

auto READ = [](const auto& s) { return s; };
auto EVAL = [](const auto& s) { return s; };
auto PRINT = [](const auto& s) { return s; };
auto rep = [](const auto& s) { return PRINT(EVAL(READ(s))); };

static const auto PROMPT{S{"user> "}};

const auto ReadLine = [](auto& in) noexcept
{
    auto s{S{""}};
    std::getline(in, s);
    return s;
};

const auto IsEOF = [](const auto& in) noexcept { return in.eof(); };

int main()
{
    while (not IsEOF(std::cin))
    {
        std::cout << PROMPT;
        std::cout << rep(ReadLine(std::cin)) << "\n";
    }

    return 0;
}
