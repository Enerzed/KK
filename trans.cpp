/* trans.cpp */

#include <iostream>
#include "scanner.hpp"
#include "ll1.hpp"
#include "gen.hpp"

int main(int argc, char* argv[])
{
	std::string filename = (argc > 1) ? argv[1] : "input.txt";
	TScanner scanner(filename);
	TLL1 analyzer(&scanner);
	if (analyzer.Analyze())
	{
		std::cout << "\nSyntax is correct\n";
		AsmGenerator asmGen(&scanner, analyzer.GetTranslate()->GetTree(),
			analyzer.GetTranslate()->GetTriadGenerator());
		asmGen.Generate("output.asm");
	}
	return 0;
}