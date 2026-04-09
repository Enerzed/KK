#include "triad.hpp"
#include <iostream>


TriadGenerator::TriadGenerator() {}


void TriadGenerator::AddTriad(const std::string& op, const std::string& arg1, const std::string& arg2)
{
    triads.push_back({ op, arg1, arg2 });
}


void TriadGenerator::PushOperand(const std::string& val)
{
    valueStack.push_back(val);
}


std::string TriadGenerator::PopOperand()
{
    if (valueStack.empty()) return "";
    std::string top = valueStack.back();
    valueStack.pop_back();
    return top;
}


void TriadGenerator::GenProc(const std::string& funcName) 
{
    AddTriad("proc", funcName); 
}

void TriadGenerator::GenProlog() 
{ 
    AddTriad("prolog"); 
}

void TriadGenerator::GenEpilog() 
{
    AddTriad("epilog"); 
}

void TriadGenerator::GenRet()
{
    AddTriad("ret"); 
}

void TriadGenerator::GenEndp()
{
    AddTriad("endp"); 
}

void TriadGenerator::GenAssign(const std::string& lhs)
{
    std::string rhs = PopOperand();
    AddTriad("=", lhs, rhs);
}

void TriadGenerator::GenBinary(const std::string& op)
{
    std::string right = PopOperand();
    std::string left = PopOperand();
    int idx = triads.size();
    AddTriad(op, left, right);
    PushOperand("(" + std::to_string(idx) + ")");
}

void TriadGenerator::GenUnary(const std::string& op)
{
    std::string operand = PopOperand();
    int idx = triads.size();
    AddTriad(op, operand);
    PushOperand("(" + std::to_string(idx) + ")");
}

void TriadGenerator::GenIf()
{
    std::string cond = PopOperand();
    int idx = triads.size();
    AddTriad("if", cond, "?");
    ifStack.push_back(idx);
}

void TriadGenerator::GenElse()
{
    if (ifStack.empty()) return;
    int ifIdx = ifStack.back();
    triads[ifIdx].arg2 = std::to_string(triads.size() + 1);
    int gotoIdx = triads.size();
    AddTriad("goto", "?");
    elseStack.push_back(gotoIdx);
}

void TriadGenerator::GenEndIf()
{
    if (!elseStack.empty())
    {
        int gotoIdx = elseStack.back();
        elseStack.pop_back();
        triads[gotoIdx].arg1 = std::to_string(triads.size());
    }
    if (!ifStack.empty())
    {
        int ifIdx = ifStack.back();
        ifStack.pop_back();
        if (triads[ifIdx].arg2 == "?")
            triads[ifIdx].arg2 = std::to_string(triads.size());
    }
    AddTriad("nop");
}

void TriadGenerator::GenNop()
{
    AddTriad("nop");
}

void TriadGenerator::GenCall(const std::string& funcName, bool pushResult)
{
    int idx = triads.size();
    AddTriad("call", funcName);
    if (pushResult)
        PushOperand("(" + std::to_string(idx) + ")");
}

void TriadGenerator::GenArrayElement()
{
    std::string index = PopOperand();
    std::string base = PopOperand();
    int idx = triads.size();
    AddTriad("index", base, index);
    PushOperand("(" + std::to_string(idx) + ")");
}

void TriadGenerator::GenArrayAssign()
{
    std::string value = PopOperand();
    std::string elemRef = PopOperand();
    AddTriad("=", elemRef, value);
}

void TriadGenerator::Print()
{
    for (size_t i = 0; i < triads.size(); ++i)
    {
        const Triad& t = triads[i];
        if (t.op == "if")
        {
            std::cout << i << ") if (" << (i + 1) << ") (" << t.arg2 << ")" << std::endl;
        }
        else if (t.op == "goto")
        {
            std::cout << i << ") goto (" << t.arg1 << ")" << std::endl;
        }
        else
        {
            std::cout << i << ") " << t.op;
            if (!t.arg1.empty()) std::cout << " " << t.arg1;
            if (!t.arg2.empty()) std::cout << ", " << t.arg2;
            std::cout << std::endl;
        }
    }
}

void TriadGenerator::Clear()
{
    triads.clear();
    valueStack.clear();
    ifStack.clear();
    elseStack.clear();
}