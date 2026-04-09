#pragma once

#include <vector>

#include "defs.hpp"
#include "scanner.hpp"
#include "translate.hpp"


class TLL1
{
private:
	TScanner* scanner;
	Translate* translate;
	std::vector <int> stack;
	int currentToken, prevToken;
	TypeLex lex, prevLex;

public:
	TLL1(TScanner* scanner);
	~TLL1();
	void NextToken();
	bool Analyze();
};