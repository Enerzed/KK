/* translate.cpp */

#include "translate.hpp"
#include <iostream>
#include <cstdlib>
#include <cstring>


Translate::Translate(TScanner* scanner) : scanner(scanner), inDeclaration(false), currentArraySize(0), currentStackOffset(0)
{
	tree = new Tree(scanner);
	triadGen = new TriadGenerator();
	currentDataType = TYPE_UNKNOWN;
	currentLHS = nullptr;
}

Translate::~Translate()
{
	OptimizeTriads();
	triadGen->Print();
	delete triadGen;
	delete tree;
}

// Debug
static const char* ActionName(int code)
{
	switch (code)
	{
	case T_ACTION_START_DATA:			return "START_DATA";
	case T_ACTION_END_DATA:				return "END_DATA";
	case T_ACTION_SET_ID:				return "SET_ID";
	case T_ACTION_SET_TYPE:				return "SET_TYPE";
	case T_ACTION_START_FUNC:			return "START_FUNC";
	case T_ACTION_END_FUNC:				return "END_FUNC";
	case T_ACTION_NEW_LEVEL:			return "NEW_LEVEL";
	case T_ACTION_RETURN_LEVEL:			return "RETURN_LEVEL";
	case T_ACTION_FIND_ID:				return "FIND_ID";
	case T_ACTION_INIT_VALUE:			return "INIT_VALUE";
	case T_ACTION_GEN_IF:				return "GEN_IF";
	case T_ACTION_GEN_ELSE:				return "GEN_ELSE";
	case T_ACTION_GEN_ENDIF:			return "GEN_ENDIF";
	case T_ACTION_GEN_ASSIGN:			return "GEN_ASSIGN";
	case T_ACTION_GEN_ARRAY_ASSIGN:		return "GEN_ARRAY_ASSIGN";
	case T_ACTION_GEN_ADD:				return "GEN_ADD";
	case T_ACTION_GEN_SUB:				return "GEN_SUB";
	case T_ACTION_GEN_MUL:				return "GEN_MUL";
	case T_ACTION_GEN_DIV:				return "GEN_DIV";
	case T_ACTION_GEN_MOD:				return "GEN_MOD";
	case T_ACTION_GEN_LT:				return "GEN_LT";
	case T_ACTION_GEN_LE:				return "GEN_LE";
	case T_ACTION_GEN_GT:				return "GEN_GT";
	case T_ACTION_GEN_GE:				return "GEN_GE";
	case T_ACTION_GEN_EQ:				return "GEN_EQ";
	case T_ACTION_GEN_NE:				return "GEN_NE";
	case T_ACTION_GEN_NEG:				return "GEN_NEG";
	case T_ACTION_PUSH_OPERAND:			return "PUSH_OPERAND";
	case T_ACTION_CALL_FUNC:			return "CALL_FUNC";
	case T_ACTION_ARRAY_ELEM:			return "ARRAY_ELEM";
	case T_ACTION_START_ARRAY:			return "START_ARRAY";
	case T_ACTION_END_ARRAY:			return "END_ARRAY";
	case T_ACTION_SET_ARRAY_SIZE:		return "SET_ARRAY_SIZE";
	case T_ACTION_FIND_ID_FOR_CALL:		return "FIND_ID_FOR_CALL";
	default:							return "UNKNOWN";
	}
}

void Translate::ExecuteAction(int actionCode, const std::string& lex, int token)
{
	/*
	std::cout << "[ACTION] " << ActionName(actionCode);
	if (!lex.empty())
		std::cout << " lex = " << lex << " ";
	if (token != 0)
		std::cout << " token = " << token;
	std::cout << std::endl;
	*/

	switch (actionCode)
	{
	case T_ACTION_START_DATA:		StartDeclareData(); break;
	case T_ACTION_END_DATA:			EndDeclareData(); break;
	case T_ACTION_SET_ID:			SetId(lex); break;
	case T_ACTION_SET_TYPE:			SetType(token); break;
	case T_ACTION_START_FUNC:		StartFunction(lex); break;
	case T_ACTION_END_FUNC:			EndFunction(); break;
	case T_ACTION_NEW_LEVEL:		NewLevel(); break;
	case T_ACTION_RETURN_LEVEL:		ReturnLevel(); break;
	case T_ACTION_FIND_ID:			FindId(lex); break;
	case T_ACTION_INIT_VALUE:		InitValue(); break;
	case T_ACTION_GEN_IF:			GenIf(); break;
	case T_ACTION_GEN_ELSE:			GenElse(); break;
	case T_ACTION_GEN_ENDIF:		GenEndIf(); break;
	case T_ACTION_GEN_ASSIGN:		GenAssign(); break;
	case T_ACTION_GEN_ARRAY_ASSIGN:	GenArrayAssign(); break;
	case T_ACTION_GEN_ADD:			GenAdd(); break;
	case T_ACTION_GEN_SUB:			GenSub(); break;
	case T_ACTION_GEN_MUL:			GenMul(); break;
	case T_ACTION_GEN_DIV:			GenDiv(); break;
	case T_ACTION_GEN_MOD:			GenMod(); break;
	case T_ACTION_GEN_LT:			GenLt(); break;
	case T_ACTION_GEN_LE:			GenLe(); break;
	case T_ACTION_GEN_GT:			GenGt(); break;
	case T_ACTION_GEN_GE:			GenGe(); break;
	case T_ACTION_GEN_EQ:			GenEq(); break;
	case T_ACTION_GEN_NE:			GenNe(); break;
	case T_ACTION_GEN_NEG:			GenNeg(); break;
	case T_ACTION_PUSH_OPERAND:		PushOperand(lex); break;
	case T_ACTION_CALL_FUNC:		CallFunc(lex); break;
	case T_ACTION_ARRAY_ELEM:		ArrayElem(); break;
	case T_ACTION_START_ARRAY:		StartArray(); break;
	case T_ACTION_END_ARRAY:		EndArray(); break;
	case T_ACTION_SET_ARRAY_SIZE:	SetArraySize(lex); break;
	case T_ACTION_FIND_ID_FOR_CALL:
	{
		Symbol* sym = tree->FindSymbol(lex);
		if (!sym) scanner->PrintError("Undeclared identifier", lex);
	}
	break;
	default: scanner->PrintError("Unknown semantic action", "");
	}
}

void Translate::StartDeclareData()
{
	inDeclaration = true;
	currentDataType = TYPE_UNKNOWN;
	currentArraySize = 0;
	initList.clear();
	lastDeclared.clear();
}

void Translate::EndDeclareData()
{
	if (inFunction)
	{
		for (Symbol* sym : lastDeclared)
		{
			int elemSize = Tree::TypeSize(sym->type);
			int totalSize = elemSize * (sym->arraySize > 0 ? sym->arraySize : 1);
			currentStackOffset -= totalSize;
			sym->offset = currentStackOffset;
			sym->isLocal = true;
		}
	}
	lastDeclared.clear();
	inDeclaration = false;
	currentArrayName.clear();
}

void Translate::SetId(const std::string& lex)
{
	if (inDeclaration)
	{
		TypeObject kind = (currentArraySize > 0) ? OBJECT_ARRAY : OBJECT_VARIABLE;
		bool added = tree->AddSymbol(lex, kind, currentDataType, currentArraySize);
		if (!added)
		{
			scanner->PrintError("Duplicate identifier", lex);
		}
		if (kind == OBJECT_ARRAY)
		{
			currentArrayName = lex;
			currentArrayType = currentDataType;
		}
		currentDeclId = lex;
		if (inFunction)
		{
			localVarNames.insert(lex);
		}
		Symbol* sym = tree->FindSymbolCurrent(lex);
		if (sym) lastDeclared.push_back(sym);
	}
}

void Translate::SetType(int token)
{
	currentDataType = tree->TokenToType(token);
}

void Translate::StartFunction(const std::string& lex)
{
	bool added = tree->AddSymbol(lex, OBJECT_FUNCTION, TYPE_VOID);
	if (!added)
	{
		scanner->PrintError("Duplicate function name", lex);
	}
	triadGen->GenProc(lex);
	triadGen->GenProlog();
	triadGen->ClearKnownVars();
	inFunction = true;
	currentFunctionName = lex;
	currentStackOffset = 0;
	savedStackOffsets.clear();
	tree->EnterScope();
}

void Translate::EndFunction()
{
	triadGen->GenEpilog();
	triadGen->GenRet();
	triadGen->GenEndp();
	Symbol* funcSym = tree->FindSymbol(currentFunctionName);
	if (funcSym)
	{
		funcSym->frameSize = -currentStackOffset;
	}
	inFunction = false;
	currentFunctionName.clear();
	tree->ExitScope();
}

void Translate::NewLevel()
{
	tree->EnterScope();
	triadGen->ClearKnownVars();
}

void Translate::ReturnLevel()
{
	tree->ExitScope();
	triadGen->ClearKnownVars();
}

void Translate::FindId(const std::string& lex)
{
	Symbol* sym = tree->FindSymbol(lex);
	if (!sym) scanner->PrintError("Undeclared identifier", lex);
	currentLHS = sym;
}

void Translate::InitValue()
{
	if (inDeclaration && !triadGen->IsStackEmpty())
	{
		std::string val = triadGen->PopOperand();
		if (!currentArrayName.empty())
		{
			initList.push_back(val);
		}
		else if (!currentDeclId.empty())
		{
			Symbol* sym = tree->FindSymbol(currentDeclId);
			if (sym)
			{
				if (!inFunction)
				{
					sym->initValue = EvaluateConstant(val);
				}
				else
				{
					currentLHS = sym;
					triadGen->PushOperand(val);
					GenAssign();
				}
			}
		}
	}
}

void Translate::GenAssign()
{
	if (!currentLHS) return;
	std::string rhs = triadGen->PopOperand();

	Symbol* rhsSym = tree->FindSymbolByAsmName(rhs);
	if (!rhsSym) rhsSym = tree->FindSymbol(rhs);
	if (rhsSym && rhsSym->kind == OBJECT_ARRAY)
		scanner->PrintError("Cannot assign array to scalar variable", currentLHS->name);

	triadGen->AddTriad("=", currentLHS->asmName, rhs);
	currentLHS = nullptr;
}

void Translate::PushOperand(const std::string& lex)
{
	Symbol* sym = tree->FindSymbol(lex);
	if (sym)
	{
		triadGen->PushOperand(sym->asmName);
		return;
	}

	char* end;
	long val = strtol(lex.c_str(), &end, 10);
	if (*end == '\0')
	{
		triadGen->PushOperand(lex);
		return;
	}
	val = strtol(lex.c_str(), &end, 16);
	if (*end == '\0')
	{
		triadGen->PushOperand(lex);
		return;
	}

	scanner->PrintError("Invalid operand", lex);
}

void Translate::CallFunc(const std::string& lex)
{
	Symbol* sym = tree->FindSymbol(lex);
	if (!sym) scanner->PrintError("Undeclared identifier", lex);

	bool pushResult = (sym->type != TYPE_VOID);
	triadGen->GenCall(sym->asmName, pushResult);
	currentLHS = nullptr;
}

void Translate::ArrayElem()
{
	triadGen->GenArrayElement();
}

void Translate::GenArrayAssign()
{
	triadGen->GenArrayAssign();
}

void Translate::StartArray()
{
	currentArraySize = 0;
}

void Translate::EndArray()
{
	if (!currentArrayName.empty())
	{
		Symbol* sym = tree->FindSymbol(currentArrayName);
		if (sym)
		{
			if (!inFunction)
			{
				std::string initValues;
				for (size_t i = 0; i < initList.size(); ++i)
				{
					if (!initValues.empty()) initValues += ", ";
					std::string val = EvaluateConstant(initList[i]);
					initValues += val;
				}
				sym->initValue = initValues;
			}
			else
			{
				for (size_t i = 0; i < initList.size(); ++i)
				{
					triadGen->PushOperand(sym->asmName);
					triadGen->PushOperand(std::to_string(i));
					triadGen->GenArrayElement();

					std::string rawVal = initList[i];
					std::string inner = rawVal;
					if (!inner.empty() && inner.front() == '(' && inner.back() == ')')
						inner = inner.substr(1, inner.size() - 2);

					char* end;
					strtol(inner.c_str(), &end, 0);
					bool isConst = (*end == '\0' && end != inner.c_str());

					if (sym->type != TYPE___INT64 && isConst)
					{
						triadGen->PushOperand(inner);
						triadGen->GenTypeCast(tree->TypeToString(sym->type));
					}
					else
					{
						triadGen->PushOperand(rawVal);
					}

					triadGen->GenArrayAssign();
				}
			}
		}
		initList.clear();
	}
	currentArrayName.clear();
	currentArraySize = 0;
}

void Translate::SetArraySize(const std::string& lex)
{
	char* end;
	currentArraySize = (int)strtol(lex.c_str(), &end, 0);
	if (!currentDeclId.empty())
	{
		Symbol* sym = tree->FindSymbol(currentDeclId);
		if (sym)
		{
			sym->kind = OBJECT_ARRAY;
			sym->arraySize = currentArraySize;
			currentArrayName = currentDeclId;
			currentArrayType = sym->type;
		}
	}
}

void Translate::GenAdd()
{
	triadGen->GenBinary("+");
}

void Translate::GenSub()
{
	triadGen->GenBinary("-");
}

void Translate::GenMul()
{
	triadGen->GenBinary("*");
}

void Translate::GenDiv()
{
	triadGen->GenBinary("/");
}

void Translate::GenMod()
{
	triadGen->GenBinary("%");
}

void Translate::GenLt()
{
	triadGen->GenBinary("<");
}

void Translate::GenLe()
{
	triadGen->GenBinary("<=");
}

void Translate::GenGt()
{
	triadGen->GenBinary(">");
}

void Translate::GenGe()
{
	triadGen->GenBinary(">=");
}

void Translate::GenEq()
{
	triadGen->GenBinary("==");
}

void Translate::GenNe()
{
	triadGen->GenBinary("!=");
}

void Translate::GenNeg()
{
	triadGen->GenUnary("-");
}

void Translate::GenIf()
{
	triadGen->GenIf();
}

void Translate::GenElse()
{
	triadGen->GenElse();
}

void Translate::GenEndIf()
{
	triadGen->GenEndIf();
}

void Translate::OptimizeTriads()
{
	triadGen->SetLocalVars(localVarNames);
	triadGen->Optimize();
	localVarNames.clear();
}

std::string Translate::EvaluateConstant(const std::string& val)
{
	if (val.empty()) return "0";
	char* end;
	long num = strtol(val.c_str(), &end, 10);
	if (*end == '\0') return val;
	if (val.front() == '(' && val.back() == ')')
	{
		int idx = std::stoi(val.substr(1, val.size() - 2));
		const auto& triads = triadGen->GetTriads();
		if (idx >= 0 && idx < (int)triads.size())
		{
			const auto& t = triads[idx];
			if (t.op == "-" && t.arg2.empty())
			{
				std::string inner = EvaluateConstant(t.arg1);
				long v = strtol(inner.c_str(), &end, 10);
				if (*end == '\0') return std::to_string(-v);
			}
			else if (t.op == "cast")
			{
				return EvaluateConstant(t.arg2);
			}
		}
	}
	return "0";
}