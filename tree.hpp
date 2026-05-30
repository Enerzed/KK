/* tree.hpp */

#pragma once

#include "scanner.hpp"
#include <unordered_map>
#include <string>
#include <vector>


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
	TypeObject kind;
	TypeData type;
	int arraySize;
};

class Tree
{
private:
	TScanner* scanner;
	std::vector<std::unordered_map<std::string, Symbol>> scopes;
	int currentScope;

public:
	Tree(TScanner* scanner);
	~Tree();

	void EnterScope();
	void ExitScope();

	bool AddSymbol(const std::string& name, TypeObject kind, TypeData type, int arraySize = -1);
	Symbol* FindSymbol(const std::string& name);
	Symbol* FindSymbolCurrent(const std::string& name);

	static TypeData TokenToType(int token);
	static std::string TypeToString(TypeData type);
};