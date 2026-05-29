#pragma once

#include "triad.hpp"
#include <string>

class Optimizer
{
public:
    Optimizer(TriadGenerator* triadGen);
    void Optimize();

private:
    TriadGenerator* triads;
    bool ConstantFolding();
    bool ConstantPropagation();
    bool OptimizeIfStatements();
    bool DeadCodeElimination();
    bool RemoveUnreachableCode();
    bool IsConstantOperand(const std::string& operand, long long& value, int depth);
    long long EvaluateOperation(const std::string& op, long long left, long long right = 0);
};