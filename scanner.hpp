#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

#include "defs.hpp"


class TScanner
{
private:
	TypeMod text;
	int pointer;
	int lineCounter = 1;
	int positionInLine = 0;
	std::vector<int> breakLinePositions;

public:
	TScanner(std::string);
	~TScanner();
	void SetPointer(int);
	int GetPointer();
	void PrintError(std::string, std::string);
	void PrintWarning(std::string, std::string);
	int Scanner(TypeLex);
	void GetData(std::string);
};