/* triad.hpp */

#pragma once

#include <vector>
#include <string>
#include <set>
#include <unordered_map>


class TriadGenerator
{
private:
	struct Triad
	{
		std::string op;
		std::string arg1;
		std::string arg2;
	};

	struct FuncInfo
	{
		int start;
		int end;
	};

	std::vector<Triad> triads;
	std::vector<FuncInfo> functions;
	std::vector<std::string> valueStack;
	std::vector<int> ifStack;
	std::vector<int> elseStack;
	std::unordered_map<std::string, std::string> knownVars;
	std::set<std::string> localVars;

	// Helpers for optimization
	bool IsConstantOperand(const std::string& operand, long long& value, int depth);
	long long EvaluateOperation(const std::string& op, long long left, long long right, bool unary);
	void ReplaceAllReferences(int oldIdx, const std::string& newValue);
	void DeleteTriad(int index);
	void SetTriad(int index, const std::string& op, const std::string& arg1 = "", const std::string& arg2 = "");
	bool IsConstTriad(int index) const;
	std::string GetConstValue(int index) const;
	int ParseIndexArg(const std::string& arg) const;

	// Optimization methods
	bool ConstantFolding();
	bool ConstantPropagation();
	bool OptimizeIfStatements();
	bool DeadCodeElimination();
	bool RemoveUnreachableCode();
	void CleanupNopsAndJumps();
	void RemoveNopsWithoutReferences();

public:
	TriadGenerator();

	// Storing triads
	void AddTriad(const std::string& op, const std::string& arg1 = "", const std::string& arg2 = "");
	void PushOperand(const std::string& val);
	std::string PopOperand();

	// Triad generators
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
	void GenTypeCast(const std::string& targetType);
	void GenNop();

	// Helpers
	int NextTriadIndex() const { return triads.size(); }
	int GetNextIndex() const { return triads.size(); }
	int GetLastIndex() const { return triads.empty() ? -1 : triads.size() - 1; }
	bool IsStackEmpty() const { return valueStack.empty(); }
	void ClearKnownVars() { knownVars.clear(); }
	void SetLocalVars(const std::set<std::string>& vars) { localVars = vars; }

	// Optimize
	void Optimize();

	// Debug
	void Print();
	void Clear();

	const std::vector<Triad>& GetTriads() const { return triads; }
	const std::vector<FuncInfo>& GetFunctions() const { return functions; }


};