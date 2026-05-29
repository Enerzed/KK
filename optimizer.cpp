#include "optimizer.hpp"
#include <cstdlib>
#include <cctype>

Optimizer::Optimizer(TriadGenerator* triadGen) : triads(triadGen) {}

void Optimizer::Optimize()
{
    ConstantFolding();
}

bool Optimizer::IsConstantOperand(const std::string& operand, long long& value)
{
    if (!operand.empty()) {
        char* end;
        value = std::strtoll(operand.c_str(), &end, 0);
        if (*end == '\0') return true;
    }
    if (operand.size() >= 3 && operand.front() == '(' && operand.back() == ')')
    {
        int idx = std::stoi(operand.substr(1, operand.size() - 2));
        if (triads->IsConstTriad(idx)) {
            std::string valStr = triads->GetConstValue(idx);
            return IsConstantOperand(valStr, value);
        }
    }
    return false;
}

long long Optimizer::EvaluateOperation(const std::string& op, long long left, long long right)
{
    if (op == "+") return left + right;
    if (op == "-") return left - right;
    if (op == "*") return left * right;
    if (op == "/") return left / right;
    if (op == "%") return left % right;
    if (op == "<") return left < right;
    if (op == "<=") return left <= right;
    if (op == ">") return left > right;
    if (op == ">=") return left >= right;
    if (op == "==") return left == right;
    if (op == "!=") return left != right;
    return 0;
}

void Optimizer::ConstantFolding()
{
    for (int i = 0; i < triads->GetTriadCount(); ++i)
    {
        std::string op = triads->GetOp(i);
        if (op == "deleted" || op == "const" || op == "=" || op == "call" ||
            op == "if" || op == "goto" || op == "nop" || op == "index" || op == "prolog" || op == "epilog" || op == "ret" || op == "endp" || op == "proc")
            continue;

        std::string arg1 = triads->GetArg1(i);
        std::string arg2 = triads->GetArg2(i);

        long long val1, val2;
        bool isConst1 = IsConstantOperand(arg1, val1);
        bool isConst2 = arg2.empty() ? true : IsConstantOperand(arg2, val2);

        if (!arg2.empty() && isConst1 && isConst2)
        {
            long long result = EvaluateOperation(op, val1, val2);
            triads->SetTriad(i, "const", std::to_string(result));
            triads->ReplaceAllReferences(i, std::to_string(result));
        }
        else if (arg2.empty() && isConst1)
        {
            long long result = EvaluateOperation(op, val1, 0);
            triads->SetTriad(i, "const", std::to_string(result));
            triads->ReplaceAllReferences(i, std::to_string(result));
        }
    }
}