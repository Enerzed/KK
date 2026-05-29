#include <iostream>
#include <string>
#include "defs.hpp"
#include "scanner.hpp"
#include "optimizer.hpp"
#include "ll1.hpp"

int main(int argc, char* argv[])
{
    std::string filename = (argc > 1) ? argv[1] : "input.txt";
    TScanner scanner(filename);
    TLL1 analyzer(&scanner);
    if (analyzer.Analyze())
    {
        std::cout << "\nSyntax is correct\n";
    }
    return 0;
}