#include "optimizer.hpp"
#include <cstdlib>
#include <cctype>
#include <unordered_map>
#include <iostream>


Optimizer::Optimizer(TriadGenerator* triadGen) : triads(triadGen) {}


void Optimizer::Optimize()
{
    const int MAX_ITER = 100;
    for (int iter = 0; iter < MAX_ITER; ++iter)
    {
        bool anyChange = false;
        if (ConstantFolding()) anyChange = true;
        if (ConstantPropagation()) anyChange = true;
        if (OptimizeIfStatements()) anyChange = true;
        if (DeadCodeElimination()) anyChange = true;
        if (!anyChange) break;
    }
    RemoveUnreachableCode();
}


bool Optimizer::IsConstantOperand(const std::string& operand, long long& value, int depth)
{
    if (depth > 50) return false;
    if (operand.empty()) return false;
    char* end;
    value = std::strtoll(operand.c_str(), &end, 0);
    if (*end == '\0') return true;
    if (operand.size() >= 3 && operand.front() == '(' && operand.back() == ')') {
        int idx = std::stoi(operand.substr(1, operand.size() - 2));
        if (triads->IsConstTriad(idx)) {
            std::string valStr = triads->GetConstValue(idx);
            return IsConstantOperand(valStr, value, depth + 1);
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
    if (op == "-") return -left;
    return 0;
}


bool Optimizer::ConstantFolding()
{
    bool changed = false;
    for (int i = 0; i < triads->GetTriadCount(); ++i)
    {
        std::string op = triads->GetOp(i);
        // Skip these operators
        if (op == "deleted" || op == "const" || op == "=" || op == "call" ||
            op == "if" || op == "goto" || op == "nop" || op == "index" ||
            op == "prolog" || op == "epilog" || op == "ret" || op == "endp" || op == "proc") 
        {
            continue;
        }

        std::string arg1 = triads->GetArg1(i);
        std::string arg2 = triads->GetArg2(i);
        long long val1, val2;
        bool isConst1 = IsConstantOperand(arg1, val1, 0);
        bool isConst2 = arg2.empty() ? true : IsConstantOperand(arg2, val2, 0);

        // Binary
        if (!arg2.empty()) 
        {
            // Constant folding: (a + 5) + 7 -> a + 12
            if (isConst1 && isConst2) {
                long long result = EvaluateOperation(op, val1, val2);
                triads->ReplaceAllReferences(i, std::to_string(result));
                triads->SetTriad(i, "nop");
                changed = true;
                continue;
            }
            // Algebraic eleminations
            // 1. x + 0 -> x
            else if (op == "+" && isConst1 && val1 == 0) 
            {
                triads->ReplaceAllReferences(i, arg2);
                triads->SetTriad(i, "nop");
                changed = true;
            }
            // 2. 0 + x -> x
            else if (op == "+" && isConst2 && val2 == 0) 
            {
                triads->ReplaceAllReferences(i, arg1);
                triads->SetTriad(i, "nop");
                changed = true;
            }
            // 3. x * 0 -> 0
            else if (op == "*" && ((isConst1 && val1 == 0) || (isConst2 && val2 == 0))) 
            {
                triads->ReplaceAllReferences(i, "0");
                triads->SetTriad(i, "nop");
                changed = true;
            }
            // 4. x * 1 -> x
            else if (op == "*" && isConst1 && val1 == 1) 
            {
                triads->ReplaceAllReferences(i, arg2);
                triads->SetTriad(i, "nop");

                changed = true;
            }
            else if (op == "*" && isConst2 && val2 == 1) 
            {
                triads->ReplaceAllReferences(i, arg1);
                triads->SetTriad(i, "nop");
                changed = true;
            }
            // 5. x / 1 -> x
            else if (op == "/" && isConst2 && val2 == 1) 
            {
                triads->ReplaceAllReferences(i, arg1);
                triads->SetTriad(i, "nop");
                changed = true;
            }
            // 6. x - x -> 0
            else if (op == "-" && arg1 == arg2) 
            {
                triads->ReplaceAllReferences(i, "0");
                triads->SetTriad(i, "nop");
                changed = true;
            }
        }
        // Unary operation with constant
        else if (arg2.empty() && isConst1 && (op == "-")) 
        {
            long long result = EvaluateOperation(op, val1, 0);
            triads->ReplaceAllReferences(i, std::to_string(result));
            triads->SetTriad(i, "nop");
            changed = true;
        }
        else if (op.find("->") != std::string::npos && isConst1) 
        {
            triads->ReplaceAllReferences(i, std::to_string(val1));
            triads->SetTriad(i, "nop");
            changed = true;
        }
    }
    return changed;
}


bool Optimizer::ConstantPropagation()
{
    bool changed = false;
    std::unordered_map<std::string, std::string> knownVars;

    for (int i = 0; i < triads->GetTriadCount(); ++i)
    {
        std::string op = triads->GetOp(i);
        if (op == "deleted") continue;

        std::string arg1 = triads->GetArg1(i);
        std::string arg2 = triads->GetArg2(i);

        if (op == "=")
        {
            std::string lhs = arg1;
            std::string rhs = arg2;

            long long constVal;
            if (IsConstantOperand(rhs, constVal, 0))
            {
                knownVars[lhs] = std::to_string(constVal);
                changed = true;
                continue;
            }
            auto it = knownVars.find(rhs);
            if (it != knownVars.end()) {
                knownVars[lhs] = it->second;
                changed = true;
                continue;
            }
            knownVars.erase(lhs);
        }
        else if (op != "proc" && op != "prolog" && op != "epilog" && op != "ret" && op != "endp" &&
            op != "nop" && op != "call" && op != "if" && op != "goto")
        {
            if (!arg1.empty() && knownVars.find(arg1) != knownVars.end())
            {
                std::string newVal = knownVars[arg1];
                if (newVal != arg1)
                {
                    triads->SetArg1(i, newVal);
                    changed = true;
                    arg1 = newVal;
                }
            }
            if (!arg2.empty() && knownVars.find(arg2) != knownVars.end())
            {
                std::string newVal = knownVars[arg2];
                if (newVal != arg2)
                {
                    triads->SetArg2(i, newVal);
                    changed = true;
                    arg2 = newVal;
                }
            }
        }
        if (op == "call" || op == "goto")
        {
            knownVars.clear();
        }
    }
    return changed;
}


bool Optimizer::OptimizeIfStatements() {
    bool changed = false;
    for (int i = 0; i < triads->GetTriadCount(); ++i) {
        std::string op = triads->GetOp(i);
        if (op != "if") continue;

        std::string cond = triads->GetArg1(i);
        std::string targetStr = triads->GetArg2(i);
        if (targetStr.empty()) continue;

        long long condVal;
        if (!IsConstantOperand(cond, condVal, 0)) continue;

        int targetIdx = -1;
        if (targetStr.front() == '(') {
            targetIdx = std::stoi(targetStr.substr(1, targetStr.size() - 2));
        }

        if (condVal != 0) {
            triads->SetTriad(i, "goto", targetStr);
            changed = true;
        }
        else {
            if (targetIdx != -1) {
                for (int j = i + 1; j < targetIdx; ++j) {
                    triads->SetTriad(j, "nop");
                }
            }
            triads->SetTriad(i, "nop");
            changed = true;
        }
    }
    return changed;
}

bool Optimizer::DeadCodeElimination() {
    bool changed = false;
    bool localChanged;
    do {
        localChanged = false;
        std::vector<int> refCount(triads->GetTriadCount(), 0);
        for (int i = 0; i < triads->GetTriadCount(); ++i) {
            std::string arg1 = triads->GetArg1(i);
            std::string arg2 = triads->GetArg2(i);
            auto countRef = [&](const std::string& arg) {
                if (arg.size() >= 3 && arg.front() == '(' && arg.back() == ')') {
                    int idx = std::stoi(arg.substr(1, arg.size() - 2));
                    if (idx >= 0 && idx < (int)refCount.size()) refCount[idx]++;
                }
                };
            countRef(arg1);
            countRef(arg2);
        }
        for (int i = 0; i < triads->GetTriadCount(); ++i) {
            std::string op = triads->GetOp(i);
            if (op == "deleted" || op == "nop") continue;
            if (op == "=" || op == "call" || op == "if" || op == "goto" ||
                op == "proc" || op == "prolog" || op == "epilog" || op == "ret" || op == "endp") {
                continue;
            }
            if (refCount[i] == 0) {
                triads->SetTriad(i, "nop");
                localChanged = true;
            }
        }
        if (localChanged) changed = true;
    } while (localChanged);
    return changed;
}

bool Optimizer::RemoveUnreachableCode()
{
    bool changed = false;
    for (int funcStart = 0; funcStart < triads->GetTriadCount(); ++funcStart) {
        if (triads->GetOp(funcStart) != "proc") continue;

        std::vector<bool> reachable(triads->GetTriadCount(), false);
        std::vector<int> stack;
        stack.push_back(funcStart);
        reachable[funcStart] = true;

        while (!stack.empty()) {
            int idx = stack.back();
            stack.pop_back();
            std::string op = triads->GetOp(idx);
            if (op == "goto") {
                std::string target = triads->GetArg1(idx);
                if (!target.empty() && target.front() == '(') {
                    int next = std::stoi(target.substr(1, target.size() - 2));
                    if (next >= 0 && next < (int)reachable.size() && !reachable[next]) {
                        reachable[next] = true;
                        stack.push_back(next);
                    }
                }
            }
            else if (op == "if") {
                std::string targetTrue = triads->GetArg2(idx);
                if (!targetTrue.empty() && targetTrue.front() == '(') {
                    int nextTrue = std::stoi(targetTrue.substr(1, targetTrue.size() - 2));
                    if (nextTrue >= 0 && nextTrue < (int)reachable.size() && !reachable[nextTrue]) {
                        reachable[nextTrue] = true;
                        stack.push_back(nextTrue);
                    }
                }
                int nextFalse = idx + 1;
                if (nextFalse < (int)reachable.size() && !reachable[nextFalse]) {
                    reachable[nextFalse] = true;
                    stack.push_back(nextFalse);
                }
            }
            else {
                int next = idx + 1;
                if (next < (int)reachable.size() && !reachable[next]) {
                    reachable[next] = true;
                    stack.push_back(next);
                }
            }
        }

        int endFunc = funcStart + 1;
        while (endFunc < triads->GetTriadCount() && triads->GetOp(endFunc) != "proc" && triads->GetOp(endFunc) != "endp") {
            if (!reachable[endFunc]) {
                triads->SetTriad(endFunc, "nop");
                changed = true;
            }
            endFunc++;
        }
        funcStart = endFunc;
    }
    return changed;
}