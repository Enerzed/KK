#pragma once

#include <vector>
#include <string>


class TriadGenerator
{
public:
    TriadGenerator();
    // Triads storing
    void AddTriad(const std::string& op, const std::string& arg1 = "", const std::string& arg2 = "");
    void PushOperand(const std::string& val);
    std::string PopOperand();
    // Triad access
    int GetTriadCount() const;
    std::string GetOp(int index) const;
    std::string GetArg1(int index) const;
    std::string GetArg2(int index) const;
    void SetTriad(int index, const std::string& op, const std::string& arg1 = "", const std::string& arg2 = "");
    bool IsConstTriad(int index) const;
    std::string GetConstValue(int index) const;
    void ReplaceAllReferences(int oldIdx, const std::string& newValue);
    void DeleteTriad(int index);
    // Generating triads
    void GenProc(const std::string& funcName);
    void GenProlog();
    void GenEpilog();
    void GenRet();
    void GenEndp();
    void GenAssign(const std::string& lhs);
    void GenBinary(const std::string& op);
    void GenUnary(const std::string& op);
    void GenIf();
    void GenElse();
    void GenEndIf();
    void GenCall(const std::string& funcName, bool pushResult = false);
    void GenArrayElement();
    void GenArrayAssign();
    void GenTypeCast(const std::string& toType, const std::string& fromType, const std::string& value);
    void GenNop();
    // Helpers
    int nextTriadIndex() const { return triads.size(); }
    int GetNextIndex() const { return triads.size(); }
    int GetLastIndex() const { return triads.empty() ? -1 : triads.size() - 1; }
    bool IsStackEmpty() const { return valueStack.empty(); }
    // Debug
    void Print();
    void Clear();

private:
    struct Triad
    {
        std::string op;
        std::string arg1;
        std::string arg2;
    };
    std::vector<Triad> triads;
    std::vector<std::string> valueStack;

    std::vector<int> ifStack;
    std::vector<int> elseStack;
};