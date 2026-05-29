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

    void ConstantFolding();
    bool IsConstantOperand(const std::string& operand, long long& value);
    long long EvaluateOperation(const std::string& op, long long left, long long right);
};