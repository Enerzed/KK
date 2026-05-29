#include "translate.hpp"
#include <iostream>
#include <cstdlib>
#include <cstring>


Translate::Translate(TScanner* scanner)
    : scanner(scanner), inDeclaration(false), currentArraySize(0)
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

void Translate::ExecuteAction(int actionCode, TypeLex lex, int token)
{
    switch (actionCode)
    {
    case T_ACTION_START_DATA:      StartDeclareData(); break;
    case T_ACTION_END_DATA:        EndDeclareData(); break;
    case T_ACTION_SET_ID:          SetId(lex); break;
    case T_ACTION_SET_TYPE:        SetType(token); break;
    case T_ACTION_START_FUNC:      StartFunction(lex); break;
    case T_ACTION_END_FUNC:        EndFunction(); break;
    case T_ACTION_NEW_LEVEL:       NewLevel(); break;
    case T_ACTION_RETURN_LEVEL:    ReturnLevel(); break;
    case T_ACTION_FIND_ID:         FindId(lex); break;
    case T_ACTION_INIT_VALUE:      InitValue(); break;
    case T_ACTION_GEN_IF:          GenIf(); break;
    case T_ACTION_GEN_ELSE:        GenElse(); break;
    case T_ACTION_GEN_ENDIF:       GenEndIf(); break;
    case T_ACTION_GEN_ASSIGN:      GenAssign(); break;
    case T_ACTION_GEN_ARRAY_ASSIGN:GenArrayAssign(); break;
    case T_ACTION_GEN_ADD:         GenAdd(); break;
    case T_ACTION_GEN_SUB:         GenSub(); break;
    case T_ACTION_GEN_MUL:         GenMul(); break;
    case T_ACTION_GEN_DIV:         GenDiv(); break;
    case T_ACTION_GEN_MOD:         GenMod(); break;
    case T_ACTION_GEN_LT:          GenLt(); break;
    case T_ACTION_GEN_LE:          GenLe(); break;
    case T_ACTION_GEN_GT:          GenGt(); break;
    case T_ACTION_GEN_GE:          GenGe(); break;
    case T_ACTION_GEN_EQ:          GenEq(); break;
    case T_ACTION_GEN_NE:          GenNe(); break;
    case T_ACTION_GEN_NEG:         GenNeg(); break;
    case T_ACTION_PUSH_OPERAND:    PushOperand(lex); break;
    case T_ACTION_CALL_FUNC:       CallFunc(lex); break;
    case T_ACTION_ARRAY_ELEM:      ArrayElem(); break;
    case T_ACTION_DELETE_LEVEL:    DeleteLevel(); break;
    case T_ACTION_START_ARRAY:     StartArray(); break;
    case T_ACTION_END_ARRAY:       EndArray(); break;
	case T_ACTION_SET_ARRAY_SIZE:  SetArraySize(lex); break;
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
}

void Translate::EndDeclareData()
{
    inDeclaration = false;
    currentArrayName.clear();
}

void Translate::SetId(TypeLex lex)
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
    }
}

void Translate::SetType(int token)
{
    currentDataType = tree->TokenToType(token);
}

void Translate::StartFunction(TypeLex lex)
{
    bool added = tree->AddSymbol(lex, OBJECT_FUNCTION, TYPE_VOID);
    if (!added)
    {
        scanner->PrintError("Duplicate function name", lex);
    }
    triadGen->GenProc(lex);
    triadGen->GenProlog();
    tree->EnterScope();
}

void Translate::EndFunction()
{
    triadGen->GenEpilog();
    triadGen->GenRet();
    triadGen->GenEndp();
    tree->ExitScope();
}

void Translate::NewLevel()
{
    tree->EnterScope();
}

void Translate::ReturnLevel()
{
    tree->ExitScope();
}

void Translate::FindId(TypeLex lex)
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
                currentLHS = sym;
                triadGen->PushOperand(val);
                GenAssign();
            }
        }
    }
}

void Translate::GenAssign()
{
    if (!currentLHS) return;
    std::string rhs = triadGen->PopOperand();
    TypeData lhsType = currentLHS->type;

    bool isDirect = (rhs.front() != '(');
    std::string inner = rhs;
    if (!isDirect && rhs.front() == '(' && rhs.back() == ')')
        inner = rhs.substr(1, rhs.size() - 2);
    else
        inner = rhs;

    char* end;
    strtol(inner.c_str(), &end, 0);
    bool isConstNumber = (*end == '\0' && end != inner.c_str());
    bool isConst = isDirect && isConstNumber;

    if (lhsType != TYPE___INT64 && lhsType != TYPE_UNKNOWN && isConst)
    {
        std::string castOp = "__int64->" + tree->TypeToString(lhsType);
        int idx = triadGen->GetNextIndex();
        triadGen->AddTriad(castOp, inner);
        rhs = "(" + std::to_string(idx) + ")";
    }

    triadGen->AddTriad("=", currentLHS->name, rhs);
    currentLHS = nullptr;
}
void Translate::PushOperand(TypeLex lex)
{
    char* end;
    long val = strtol(lex, &end, 10);

    if (*end == '\0')
    {
        triadGen->PushOperand(lex);
        return;
    }

    val = strtol(lex, &end, 16);
    if (*end == '\0')
    {
        triadGen->PushOperand(lex);
        return;
    }

    Symbol* sym = tree->FindSymbol(lex);
    if (!sym) scanner->PrintError("Undeclared identifier", lex);
    triadGen->PushOperand(lex);
}

void Translate::CallFunc(TypeLex lex)
{
    Symbol* sym = tree->FindSymbol(lex);
    if (!sym) scanner->PrintError("Undeclared identifier", lex);

    bool pushResult = (sym->type != TYPE_VOID);
    triadGen->GenCall(lex, pushResult);
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

void Translate::DeleteLevel()
{
    tree->ExitScope();
}

void Translate::StartArray()
{
    currentArraySize = 0;
}

void Translate::EndArray()
{
    if (!initList.empty() && !currentArrayName.empty())
    {
        Symbol* sym = tree->FindSymbol(currentArrayName);
        if (sym)
        {
            for (size_t i = 0; i < initList.size(); ++i)
            {
                triadGen->PushOperand(currentArrayName);
                triadGen->PushOperand(std::to_string(i));
                triadGen->GenArrayElement();

                std::string val = initList[i];
                std::string inner = val;
                if (!inner.empty() && inner.front() == '(' && inner.back() == ')')
                    inner = inner.substr(1, inner.size() - 2);

                char* end;
                strtol(inner.c_str(), &end, 0);
                bool isConst = (*end == '\0' && end != inner.c_str());

                if (sym->type != TYPE___INT64 && isConst)
                {
                    std::string castOp = "__int64->" + tree->TypeToString(sym->type);
                    int idx = triadGen->GetNextIndex();
                    triadGen->AddTriad(castOp, inner);
                    val = "(" + std::to_string(idx) + ")";
                }
                triadGen->PushOperand(val);
                triadGen->GenArrayAssign();
            }
        }
        initList.clear();
    }
    currentArrayName.clear();
    currentArraySize = 0;
}

void Translate::SetArraySize(TypeLex lex)
{
    char* end;
    currentArraySize = (int)strtol(lex, &end, 0);
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
    Optimizer optimizer(triadGen);
    optimizer.Optimize();
}