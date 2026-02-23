#pragma once

#include <vector>

#include "defs.hpp"
#include "scanner.hpp"


class TLL1
{
private:
	TScanner* scanner;
	std::vector <int> stack;
	int currentToken;
	TypeLex lex;

public:
	TLL1(TScanner* scanner);
	~TLL1();
	void NextToken();
	bool Analyze();
};