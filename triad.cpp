/* triad.cpp */

#include "triad.hpp"
#include <iostream>
#include <cstdlib>
#include <set>


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
	FuncInfo fi;
	fi.start = triads.size();
	fi.end = -1;
	functions.push_back(fi);
	AddTriad("proc", funcName);
}

void TriadGenerator::GenProlog() { AddTriad("prolog"); }
void TriadGenerator::GenEpilog() { AddTriad("epilog"); }
void TriadGenerator::GenRet() { AddTriad("ret"); }

void TriadGenerator::GenEndp()
{
	if (!functions.empty())
	{
		functions.back().end = triads.size();
	}
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

void TriadGenerator::GenNop() { AddTriad("nop"); }

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

void TriadGenerator::GenTypeCast(const std::string& targetType)
{
	std::string operand = PopOperand();
	int idx = triads.size();
	AddTriad("cast", targetType, operand);
	PushOperand("(" + std::to_string(idx) + ")");
}

// Triad access
int TriadGenerator::ParseIndexArg(const std::string& arg) const
{
	if (arg.empty()) return -1;
	if (arg.size() >= 3 && arg.front() == '(' && arg.back() == ')')
	{
		try { return std::stoi(arg.substr(1, arg.size() - 2)); }
		catch (...) { return -1; }
	}
	char* end;
	long idx = std::strtol(arg.c_str(), &end, 10);
	if (*end == '\0' && idx >= 0) return (int)idx;
	return -1;
}

void TriadGenerator::SetTriad(int index, const std::string& op, const std::string& arg1, const std::string& arg2)
{
	if (index >= 0 && index < (int)triads.size())
	{
		triads[index].op = op;
		triads[index].arg1 = arg1;
		triads[index].arg2 = arg2;
	}
}

bool TriadGenerator::IsConstTriad(int index) const
{
	return (index >= 0 && index < (int)triads.size() && triads[index].op == "const");
}

std::string TriadGenerator::GetConstValue(int index) const
{
	if (IsConstTriad(index))
		return triads[index].arg1;
	return "";
}

void TriadGenerator::ReplaceAllReferences(int oldIdx, const std::string& newValue)
{
	std::string pattern = "(" + std::to_string(oldIdx) + ")";
	for (auto& t : triads)
	{
		if (t.arg1 == pattern) t.arg1 = newValue;
		if (t.arg2 == pattern) t.arg2 = newValue;
	}
}

void TriadGenerator::DeleteTriad(int index)
{
	if (index >= 0 && index < (int)triads.size())
		triads[index].op = "deleted";
}

// Optimize
void TriadGenerator::Optimize()
{
	const int MAX_ITER = 100;
	for (int iter = 0; iter < MAX_ITER; ++iter)
	{
		bool anyChange = false;
		if (ConstantFolding()) anyChange = true;
		if (ConstantPropagation()) anyChange = true;
		if (OptimizeIfStatements()) anyChange = true;
		if (DeadCodeElimination()) anyChange = true;
		if (RemoveUnreachableCode()) anyChange = true;
		CleanupNopsAndJumps();
		if (!anyChange) break;
	}
}

bool TriadGenerator::IsConstantOperand(const std::string& operand, long long& value, int depth)
{
	if (depth > 50) return false;
	if (operand.empty()) return false;
	char* end;
	value = std::strtoll(operand.c_str(), &end, 0);
	if (*end == '\0') return true;
	if (operand.size() >= 3 && operand.front() == '(' && operand.back() == ')')
	{
		int idx = std::stoi(operand.substr(1, operand.size() - 2));
		if (IsConstTriad(idx))
		{
			std::string valStr = GetConstValue(idx);
			return IsConstantOperand(valStr, value, depth + 1);
		}
	}
	return false;
}

long long TriadGenerator::EvaluateOperation(const std::string& op, long long left, long long right, bool unary)
{
	if (unary)
	{
		if (op == "-") return -left;
		return left;
	}
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

bool TriadGenerator::ConstantFolding()
{
	bool changed = false;
	for (int i = 0; i < (int)triads.size(); ++i)
	{
		std::string op = triads[i].op;
		if (op == "deleted" || op == "const" || op == "=" || op == "call" ||
			op == "if" || op == "goto" || op == "nop" || op == "index" ||
			op == "prolog" || op == "epilog" || op == "ret" || op == "endp" || op == "proc")
			continue;

		std::string arg1 = triads[i].arg1;
		std::string arg2 = triads[i].arg2;
		long long val1, val2;
		bool isConst1 = IsConstantOperand(arg1, val1, 0);
		bool isConst2 = arg2.empty() ? true : IsConstantOperand(arg2, val2, 0);

		// Обработка cast — отдельно и до всех остальных проверок
		if (op == "cast")
		{
			if (isConst2)
			{
				ReplaceAllReferences(i, std::to_string(val2));
				SetTriad(i, "nop");
				changed = true;
			}
			continue;
		}

		// Бинарные операции
		if (!arg2.empty())
		{
			if (isConst1 && isConst2)
			{
				long long result = EvaluateOperation(op, val1, val2, false);
				ReplaceAllReferences(i, std::to_string(result));
				SetTriad(i, "nop");
				changed = true;
				continue;
			}
			if (op == "+" && isConst1 && val1 == 0)
			{
				ReplaceAllReferences(i, arg2);
				SetTriad(i, "nop");
				changed = true;
			}
			else if (op == "+" && isConst2 && val2 == 0)
			{
				ReplaceAllReferences(i, arg1);
				SetTriad(i, "nop");
				changed = true;
			}
			else if (op == "*" && ((isConst1 && val1 == 0) || (isConst2 && val2 == 0)))
			{
				ReplaceAllReferences(i, "0");
				SetTriad(i, "nop");
				changed = true;
			}
			else if (op == "*" && isConst1 && val1 == 1)
			{
				ReplaceAllReferences(i, arg2);
				SetTriad(i, "nop");
				changed = true;
			}
			else if (op == "*" && isConst2 && val2 == 1)
			{
				ReplaceAllReferences(i, arg1);
				SetTriad(i, "nop");
				changed = true;
			}
			else if (op == "/" && isConst2 && val2 == 1)
			{
				ReplaceAllReferences(i, arg1);
				SetTriad(i, "nop");
				changed = true;
			}
			else if (op == "-" && arg1 == arg2)
			{
				ReplaceAllReferences(i, "0");
				SetTriad(i, "nop");
				changed = true;
			}
		}
		else if (arg2.empty() && isConst1 && op == "-")
		{
			long long result = EvaluateOperation(op, val1, 0, false);
			ReplaceAllReferences(i, std::to_string(result));
			SetTriad(i, "nop");
			changed = true;
		}
	}
	return changed;
}

bool TriadGenerator::ConstantPropagation()
{
	bool changed = false;

	for (int i = 0; i < (int)triads.size(); ++i)
	{
		if (triads[i].op == "deleted") continue;
		std::string op = triads[i].op;
		std::string arg1 = triads[i].arg1;
		std::string arg2 = triads[i].arg2;

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
			if (it != knownVars.end())
			{
				knownVars[lhs] = it->second;
				triads[i].arg2 = it->second;
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
					triads[i].arg1 = newVal;
					changed = true;
					arg1 = newVal;
				}
			}
			if (!arg2.empty() && knownVars.find(arg2) != knownVars.end())
			{
				std::string newVal = knownVars[arg2];
				if (newVal != arg2)
				{
					triads[i].arg2 = newVal;
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

bool TriadGenerator::OptimizeIfStatements()
{
	bool changed = false;
	for (int i = 0; i < (int)triads.size(); ++i)
	{
		if (triads[i].op != "if") continue;
		std::string cond = triads[i].arg1;
		std::string targetStr = triads[i].arg2;
		if (targetStr.empty() || targetStr == "?") continue;

		long long condVal;
		if (!IsConstantOperand(cond, condVal, 0)) continue;

		int targetIdx = ParseIndexArg(targetStr);

		if (condVal != 0)
		{
			SetTriad(i, "nop");
			changed = true;
		}
		else
		{
			SetTriad(i, "goto", targetStr);
			if (targetIdx >= 0)
			{
				for (int j = i + 1; j < targetIdx; ++j)
					SetTriad(j, "nop");
			}
			changed = true;
		}
	}
	return changed;
}

bool TriadGenerator::DeadCodeElimination()
{
	bool changed = false;
	bool localChanged;
	do
	{
		localChanged = false;
		std::vector<int> refCount(triads.size(), 0);
		std::set<std::string> usedVars;

		for (int i = 0; i < (int)triads.size(); ++i)
		{
			if (triads[i].op == "deleted" || triads[i].op == "nop") continue;

			auto processArg = [&](const std::string& arg, bool isLHS)
				{
					if (arg.empty()) return;
					if (arg.size() >= 3 && arg.front() == '(' && arg.back() == ')')
					{
						int idx = ParseIndexArg(arg);
						if (idx >= 0 && idx < (int)refCount.size()) refCount[idx]++;
					}
					if (!isLHS && !arg.empty() && arg.front() != '(')
					{
						char* end;
						long val = std::strtol(arg.c_str(), &end, 10);
						if (*end != '\0')
						{
							usedVars.insert(arg);
						}
					}
				};

			bool isAssignment = (triads[i].op == "=");
			processArg(triads[i].arg1, isAssignment);
			processArg(triads[i].arg2, false);
		}

		for (int i = 0; i < (int)triads.size(); ++i)
		{
			std::string op = triads[i].op;
			if (op == "deleted" || op == "nop") continue;

			if (op != "=" && op != "call" && op != "if" && op != "goto" &&
				op != "proc" && op != "prolog" && op != "epilog" && op != "ret" && op != "endp")
			{
				if (refCount[i] == 0)
				{
					SetTriad(i, "nop");
					localChanged = true;
				}
			}
			else if (op == "=")
			{
				std::string lhs = triads[i].arg1;
				if (!lhs.empty() && lhs.front() != '(' && localVars.count(lhs))
				{
					if (usedVars.find(lhs) == usedVars.end())
					{
						SetTriad(i, "nop");
						localChanged = true;
					}
				}
			}
		}

		if (localChanged) changed = true;
	} while (localChanged);
	return changed;
}

bool TriadGenerator::RemoveUnreachableCode()
{
	bool changed = false;
	for (int funcStart = 0; funcStart < (int)triads.size(); ++funcStart)
	{
		if (triads[funcStart].op != "proc") continue;

		std::vector<bool> reachable(triads.size(), false);
		std::vector<int> stack;
		stack.push_back(funcStart);
		reachable[funcStart] = true;

		while (!stack.empty())
		{
			int idx = stack.back();
			stack.pop_back();
			const std::string& op = triads[idx].op;
			if (op == "goto")
			{
				int target = ParseIndexArg(triads[idx].arg1);
				if (target >= 0 && target < (int)reachable.size() && !reachable[target])
				{
					reachable[target] = true;
					stack.push_back(target);
				}
			}
			else if (op == "if")
			{
				int targetTrue = ParseIndexArg(triads[idx].arg2);
				if (targetTrue >= 0 && targetTrue < (int)reachable.size() && !reachable[targetTrue])
				{
					reachable[targetTrue] = true;
					stack.push_back(targetTrue);
				}
				int nextFalse = idx + 1;
				if (nextFalse < (int)reachable.size() && !reachable[nextFalse])
				{
					reachable[nextFalse] = true;
					stack.push_back(nextFalse);
				}
			}
			else
			{
				int next = idx + 1;
				if (next < (int)reachable.size() && !reachable[next])
				{
					reachable[next] = true;
					stack.push_back(next);
				}
			}
		}

		int endFunc = funcStart + 1;
		while (endFunc < (int)triads.size() && triads[endFunc].op != "proc" && triads[endFunc].op != "endp")
		{
			if (!reachable[endFunc])
			{
				SetTriad(endFunc, "nop");
				changed = true;
			}
			endFunc++;
		}
		funcStart = endFunc;
	}
	return changed;
}

void TriadGenerator::CleanupNopsAndJumps()
{
	for (int i = 0; i < (int)triads.size(); ++i)
	{
		if (triads[i].op == "if")
		{
			int targetIdx = ParseIndexArg(triads[i].arg2);
			if (targetIdx == i + 1)
			{
				SetTriad(i, "nop", "", "");
			}
		}
	}

	for (int i = 0; i < (int)triads.size(); ++i)
	{
		if (triads[i].op == "goto")
		{
			int targetIdx = ParseIndexArg(triads[i].arg1);
			if (targetIdx == i + 1)
			{
				SetTriad(i, "nop", "", "");
			}
		}
	}

	for (int i = 0; i < (int)triads.size(); ++i)
	{
		if (triads[i].op == "goto")
		{
			int targetIdx = ParseIndexArg(triads[i].arg1);
			if (targetIdx >= 0 && targetIdx < (int)triads.size() &&
				triads[targetIdx].op == "nop")
			{
				bool otherRefs = false;
				for (const auto& t : triads)
				{
					if (&t == &triads[i]) continue;
					auto check = [&](const std::string& arg)
						{
							int idx = ParseIndexArg(arg);
							if (idx == targetIdx) otherRefs = true;
						};
					check(t.arg1);
					check(t.arg2);
					if (otherRefs) break;
				}
				if (!otherRefs)
				{
					SetTriad(i, "nop", "", "");
				}
			}
		}
	}

	bool changed = true;
	while (changed)
	{
		changed = false;
		for (int i = 0; i < (int)triads.size(); ++i)
		{
			if (triads[i].op == "if")
			{
				int targetIdx = ParseIndexArg(triads[i].arg2);
				if (targetIdx == i)
				{
					SetTriad(i, "nop", "", "");
					changed = true;
				}
			}
			else if (triads[i].op == "goto")
			{
				int targetIdx = ParseIndexArg(triads[i].arg1);
				if (targetIdx == i + 1)
				{
					SetTriad(i, "nop", "", "");
					changed = true;
				}
			}
		}
		if (changed)
		{
			RemoveNopsWithoutReferences();
		}
	}

	// 4. Финальное удаление всех nop/deleted без ссылок
	RemoveNopsWithoutReferences();
}

void TriadGenerator::RemoveNopsWithoutReferences()
{
	std::set<int> referenced;

	for (const auto& t : triads)
	{
		auto addBracketedRef = [&](const std::string& arg)
			{
				if (arg.size() >= 3 && arg.front() == '(' && arg.back() == ')')
				{
					int idx = ParseIndexArg(arg);
					if (idx >= 0 && idx < (int)triads.size())
						referenced.insert(idx);
				}
			};
		addBracketedRef(t.arg1);
		addBracketedRef(t.arg2);

		if (t.op == "goto")
		{
			int idx = ParseIndexArg(t.arg1);
			if (idx >= 0 && idx < (int)triads.size())
				referenced.insert(idx);
		}
		else if (t.op == "if")
		{
			int idx = ParseIndexArg(t.arg2);
			if (idx >= 0 && idx < (int)triads.size())
				referenced.insert(idx);
		}
	}

	std::vector<int> newIndex(triads.size(), -1);
	std::vector<Triad> newTriads;
	for (size_t i = 0; i < triads.size(); ++i)
	{
		bool remove = (triads[i].op == "nop" || triads[i].op == "deleted")
			&& referenced.find(i) == referenced.end();
		if (!remove)
		{
			newIndex[i] = newTriads.size();
			newTriads.push_back(triads[i]);
		}
	}

	for (auto& t : newTriads)
	{
		auto updateRefOrLabel = [&](std::string& arg, bool bareNumber)
			{
				if (arg.empty()) return;
				bool isRef = (arg.size() >= 3 && arg.front() == '(' && arg.back() == ')');
				int oldIdx = ParseIndexArg(arg);
				if (oldIdx >= 0 && oldIdx < (int)newIndex.size() && newIndex[oldIdx] != -1)
				{
					int newIdx = newIndex[oldIdx];
					if (bareNumber)
						arg = std::to_string(newIdx);
					else if (isRef)
						arg = "(" + std::to_string(newIdx) + ")";
				}
			};


		if (t.op == "goto") updateRefOrLabel(t.arg1, true);
		else if (t.op == "if") updateRefOrLabel(t.arg2, true);

		auto updateExprArg = [&](std::string& arg)
			{
				if (arg.size() >= 3 && arg.front() == '(' && arg.back() == ')')
				{
					int oldIdx = ParseIndexArg(arg);
					if (oldIdx >= 0 && oldIdx < (int)newIndex.size() && newIndex[oldIdx] != -1)
						arg = "(" + std::to_string(newIndex[oldIdx]) + ")";
				}
			};

		updateExprArg(t.arg1);
		updateExprArg(t.arg2);
	}

	triads = std::move(newTriads);
	ifStack.clear();
	elseStack.clear();
}

// Debug
void TriadGenerator::Print()
{
	for (size_t i = 0; i < triads.size(); ++i)
	{
		const Triad& t = triads[i];
		if (t.op == "deleted") continue;
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