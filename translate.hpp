#pragma once

#include "tree.hpp"
#include "triad.hpp"
#include "scanner.hpp"


class Translate
{
private:
    Tree* tree;
    TScanner* scanner;
    TypeData currentDataType;
    TypeData currentArrayType;
    bool inDeclaration;
    int currentArraySize;
    std::string currentArrayName;
    std::string currentDeclId;
    Symbol* currentLHS;
    std::vector<std::string> initList;

public:
    TriadGenerator* triadGen;
    Translate(TScanner* scanner);
    ~Translate();
    void ExecuteAction(int actionCode, TypeLex lex, int token);
    // Actions
    void StartDeclareData();
    void EndDeclareData();
    void SetId(TypeLex lex);
    void SetType(int token);
    void StartFunction(TypeLex lex);
    void EndFunction();
    void NewLevel();
    void ReturnLevel();
    void FindId(TypeLex lex);
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
    void PushOperand(TypeLex lex);
    void CallFunc(TypeLex lex);
    void ArrayElem();
    void DeleteLevel();
    void StartArray();
    void EndArray();
    void SetArraySize(TypeLex lex);
    
    TriadGenerator* GetTriadGenerator() { return triadGen; }
};