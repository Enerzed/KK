/* gen.hpp */

#pragma once

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <set>
#include <vector>
#include <cstdlib>
#include <string>
#include <sstream>

#include "tree.hpp"
#include "triad.hpp"
#include "scanner.hpp"

class AsmGenerator
{
private:
	TScanner* scanner;
	Tree* tree;
	TriadGenerator* triadGen;
	std::ostream* out;
	std::string mainAsmName;
	std::string raxContent;
	std::string rbxContent;

	std::unordered_map<int, int> tempOffsets;
	std::set<int> indexTriads;

	void GenerateDataSegment();
	void GenerateDataSegmentInit();

	void GenerateFunction(int funcStart, int funcEnd, const std::string& funcName, int frameSize);

	void EmitAssignment(const TriadGenerator::Triad& t, int i, const std::vector<int>& refCount);
	void EmitBinaryOp(const TriadGenerator::Triad& t, int i, const std::vector<int>& refCount);
	void EmitUnaryOp(const TriadGenerator::Triad& t, int i, const std::vector<int>& refCount);
	void EmitCall(const TriadGenerator::Triad& t, int i, const std::vector<int>& refCount);
	void EmitIndex(const TriadGenerator::Triad& t, int i, const std::vector<int>& refCount);
	void SaveResult(int i, const std::vector<int>& refCount);

	std::string OperandToStr(const std::string& op, bool asAddress = false);
	Symbol* FindSymbolForOperand(const std::string& op);
	bool IsImmediate(const std::string& s);

	void LoadToRAX(const std::string& operand);
	void StoreFromRAX(const std::string& operand);
	void LoadEffectiveAddress(const std::string& operand);

	int ParseIndex(const std::string& s);
	bool IsTempIndex(const std::string& operand, int& idx);

public:
	AsmGenerator(TScanner* scanner, Tree* tree, TriadGenerator* triadGen);
	void Generate(const std::string& filename);
};