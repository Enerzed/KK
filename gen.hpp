/* gen.hpp */

#pragma once

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <set>
#include <vector>
#include <cstdlib>
#include "tree.hpp"
#include "triad.hpp"


class AsmGenerator
{
private:
	Tree* tree;
	TriadGenerator* triadGen;
	std::ostream* out;
	std::string mainAsmName;

	std::unordered_map<int, int> tempOffsets;
	std::set<int> indexTriads;

	void GenerateDataSegment();
	void GenerateFunction(int funcStart, int funcEnd, const std::string& funcName, int frameSize);
	void GenerateCode(int start, int end);

	std::string OperandToStr(const std::string& op, bool asAddress = false);
	void LoadToEAX(const std::string& operand);
	void StoreFromEAX(const std::string& operand);
	void LoadEffectiveAddress(const std::string& operand);

	int ParseIndex(const std::string& s);
	bool IsTempIndex(const std::string& operand, int& idx);

public:
	AsmGenerator(Tree* t, TriadGenerator* tg) : tree(t), triadGen(tg), out(nullptr) {}
	void Generate(const std::string& filename);
};