/* tree.hpp */

#pragma once

#include "scanner.hpp"
#include <unordered_map>
#include <string>
#include <vector>
#include <cstdio>


enum TypeData
{
	TYPE_UNKNOWN,
	TYPE_VOID,
	TYPE_CHAR,
	TYPE_SHORT,
	TYPE_INT,
	TYPE_LONG,
	TYPE___INT64
};

enum TypeObject
{
	OBJECT_UNKNOWN,
	OBJECT_VARIABLE,
	OBJECT_ARRAY,
	OBJECT_FUNCTION
};

struct Symbol
{
	std::string name;
	std::string asmName;

	TypeObject kind;
	TypeData type;
	int arraySize;

	int offset;
	int frameSize;
	bool isLocal;
};

class Tree
{
private:
	TScanner* scanner;
	std::vector<std::unordered_map<std::string, Symbol>> scopes;
	int currentScope;
	int asmIdCounter;

public:
	Tree(TScanner* scanner);
	~Tree();

	void EnterScope();
	void ExitScope();

	bool AddSymbol(const std::string& name, TypeObject kind, TypeData type, int arraySize = -1);
	Symbol* FindSymbol(const std::string& name);
	Symbol* FindSymbolCurrent(const std::string& name);
	Symbol* FindSymbolByAsmName(const std::string& asmName);

	static TypeData TokenToType(int token);
	static std::string TypeToString(TypeData type);
	static int TypeSize(TypeData type);

	const std::vector<std::unordered_map<std::string, Symbol>>& GetScopes() const { return scopes; }
};