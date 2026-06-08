/* tree.cpp */

#include "tree.hpp"


Tree::Tree(TScanner* scanner) : scanner(scanner), currentScope(-1), asmIdCounter(0)
{
	EnterScope();
}

Tree::~Tree()
{
	while (currentScope >= 0)
		ExitScope();
}

void Tree::EnterScope()
{
	scopes.push_back({});
	currentScope++;
}

void Tree::ExitScope()
{
	if (currentScope >= 0)
	{
		currentScope--;
	}
}

bool Tree::AddSymbol(const std::string& name, TypeObject kind, TypeData type, int arraySize)
{
	if (FindSymbolCurrent(name) != nullptr)
		return false;
	Symbol sym;
	sym.name = name;
	sym.kind = kind;
	sym.type = type;
	sym.arraySize = arraySize;
	sym.isLocal = false;
	sym.offset = 0;
	sym.frameSize = 0;

	asmIdCounter++;
	char buf[32];
	sprintf_s(buf, "%s@%02d", name.c_str(), asmIdCounter);
	sym.asmName = buf;

	scopes[currentScope][name] = sym;
	return true;
}

Symbol* Tree::FindSymbol(const std::string& name)
{
	for (int i = currentScope; i >= 0; --i)
	{
		auto it = scopes[i].find(name);
		if (it != scopes[i].end())
			return &(it->second);
	}
	return nullptr;
}

Symbol* Tree::FindSymbolCurrent(const std::string& name)
{
	if (currentScope < 0) return nullptr;
	auto it = scopes[currentScope].find(name);
	return (it != scopes[currentScope].end()) ? &(it->second) : nullptr;
}

Symbol* Tree::FindSymbolByAsmName(const std::string& asmName)
{
	for (auto& scope : scopes)
	{
		for (auto& kv : scope)
		{
			if (kv.second.asmName == asmName)
				return &kv.second;
		}
	}
	return nullptr;
}

TypeData Tree::TokenToType(int token)
{
	switch (token)
	{
	case TVoid:			return TYPE_VOID;
	case TInt:			return TYPE_INT;
	case TShort:		return TYPE_SHORT;
	case TLong:			return TYPE_LONG;
	case T__Int64:		return TYPE___INT64;
	case TChar:			return TYPE_CHAR;
	default:			return TYPE_UNKNOWN;
	}
}

std::string Tree::TypeToString(TypeData type)
{
	switch (type)
	{
	case TYPE_VOID:		return "void";
	case TYPE_CHAR:		return "char";
	case TYPE_SHORT:	return "short";
	case TYPE_INT:		return "int";
	case TYPE_LONG:		return "long";
	case TYPE___INT64:	return "__int64";
	default:			return "unknown";
	}
}

int Tree::TypeSize(TypeData type)
{
	switch (type)
	{
	case TYPE_CHAR:		return 1;
	case TYPE_SHORT:	return 2;
	case TYPE_INT:		return 4;
	case TYPE_LONG:		return 4;
	case TYPE___INT64:	return 8;
	default:			return 4;
	}
}