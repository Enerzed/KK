/* translate.hpp */

#pragma once

#include "tree.hpp"
#include "triad.hpp"
#include "scanner.hpp"


class Translate
{
private:
	Tree* tree;
	TScanner* scanner;
	TriadGenerator* triadGen;
	TypeData currentDataType;
	TypeData currentArrayType;
	bool inDeclaration = false;
	bool inFunction = false;
	int currentArraySize;
	std::string currentArrayName;
	std::string currentDeclId;
	Symbol* currentLHS;
	std::vector<std::string> initList;
	std::set<std::string> localVarNames;

public:
	Translate(TScanner* scanner);
	~Translate();
	void ExecuteAction(int actionCode, const std::string& lex, int token);

	// Actions
	void StartDeclareData();
	void EndDeclareData();
	void SetId(const std::string& lex);
	void SetType(int token);
	void StartFunction(const std::string& lex);
	void EndFunction();
	void NewLevel();
	void ReturnLevel();
	void FindId(const std::string& lex);
	void InitValue();
	void GenIf();
	void GenElse();
	void GenEndIf();
	void GenAssign();
	void GenArrayAssign();
	void GenAdd();
	void GenSub();
	void GenMul();
	void GenDiv();
	void GenMod();
	void GenLt();
	void GenLe();
	void GenGt();
	void GenGe();
	void GenEq();
	void GenNe();
	void GenNeg();
	void PushOperand(const std::string& lex);
	void CallFunc(const std::string& lex);
	void ArrayElem();
	void StartArray();
	void EndArray();
	void SetArraySize(const std::string& lex);

	// Triad generator
	TriadGenerator* GetTriadGenerator() { return triadGen; }
	void OptimizeTriads();
};