#include <iostream>
#include <string>

#include "defs.hpp"
#include "scanner.hpp"
#include "ll1.hpp"


int main(int argc, char* argv[])
{
	TScanner* scanner;
	scanner = new TScanner("input.txt");
	int type;
	TypeLex lex;
	if (argc <= 1)
		GetData("input.txt");
	else
		GetData(argv[1]);

}