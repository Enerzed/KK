/* scanner.hpp */
#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <unordered_map>

#include "defs.hpp"

class TScanner
{
private:
	std::string text;
	int pointer = 0;
	int lineCounter = 1;
	int positionInLine = 0;
	std::vector<int> breakLinePositions;

public:
	TScanner(const std::string& fileName);
	~TScanner();

	void SetPointer(int newPointer);
	int GetPointer() const;

	void PrintError(const std::string& error, const std::string& text = "");
	void PrintWarning(const std::string& warning, const std::string& text = "");

	int Scanner(std::string& lex);
	void GetData(const std::string& fileName);
};