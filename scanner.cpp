/* scanner.cpp */

#include "scanner.hpp"

static std::unordered_map<std::string, int> keywordMap =
{
	{"int", TInt}, {"short", TShort}, {"long", TLong}, {"__int64", T__Int64},
	{"char", TChar}, {"if", TIf}, {"else", TElse}, {"void", TVoid}
};

TScanner::TScanner(const std::string& fileName)
{
	GetData(fileName);
	pointer = 0;
}

TScanner::~TScanner()
{
	breakLinePositions.clear();
}

void TScanner::SetPointer(int newPointer)
{
	pointer = newPointer;
}

int TScanner::GetPointer() const
{
	return pointer;
}

void TScanner::PrintError(const std::string& error, const std::string& text)
{
	if (breakLinePositions.empty())
		positionInLine = static_cast<int>(pointer);
	else
		positionInLine = static_cast<int>(pointer - breakLinePositions.back());
	std::cout << "Line " << lineCounter << " position " << positionInLine << ": " << error;
	if (!text.empty()) std::cout << " " << text;
	std::cout << std::endl;
	exit(1);
}

void TScanner::PrintWarning(const std::string& error, const std::string& text)
{
	if (breakLinePositions.empty())
		positionInLine = static_cast<int>(pointer);
	else
		positionInLine = static_cast<int>(pointer - breakLinePositions.back());
	std::cout << "Line " << lineCounter << " position " << positionInLine << ": " << error;
	if (!text.empty()) std::cout << " " << text;
	std::cout << std::endl;
}

int TScanner::Scanner(std::string& lex)
{
	lex.clear();
start:
	while (pointer < text.size() && (text[pointer] == ' ' || text[pointer] == '\t' || text[pointer] == '\n'))
	{
		if (text[pointer] == '\n')
		{
			if (std::find(breakLinePositions.begin(), breakLinePositions.end(), pointer) == breakLinePositions.end())
			{
				lineCounter++;
				breakLinePositions.push_back(pointer);
			}
		}
		pointer++;
	}

	if (pointer >= text.size())
	{
		lex = "#";
		return TEnd;
	}

	// Comments
	if (text[pointer] == '/')
	{
		if (pointer + 1 < text.size() && text[pointer + 1] == '/')
		{
			pointer += 2;
			while (pointer < text.size() && text[pointer] != '\n') pointer++;
			goto start;
		}
		else if (pointer + 1 < text.size() && text[pointer + 1] == '*')
		{
			pointer += 2;
			while (pointer + 1 < text.size() && !(text[pointer] == '*' && text[pointer + 1] == '/'))
			{
				if (text[pointer] == '\n')
				{
					lineCounter++;
					breakLinePositions.push_back(pointer);
				}
				pointer++;
			}
			if (pointer + 1 >= text.size())
			{
				PrintError("Unterminated multi-line comment");
				return TErr;
			}
			pointer += 2;
			goto start;
		}
		else
		{
			lex = "/";
			pointer++;
			return TDiv;
		}
	}

	if (text[pointer] == '0' && pointer + 1 < text.size() && (text[pointer + 1] == 'x' || text[pointer + 1] == 'X'))
	{
		lex += text[pointer++]; // 0
		lex += text[pointer++]; // x
		if (pointer >= text.size() || !isxdigit(text[pointer]))
		{
			PrintError("Invalid hexadecimal constant", lex);
			return TErr;
		}
		while (pointer < text.size() && isxdigit(text[pointer]))
		{
			lex += text[pointer++];
		}
		return TConst16;
	}

	if (isdigit(text[pointer]))
	{
		while (pointer < text.size() && isdigit(text[pointer]))
		{
			lex += text[pointer++];
		}
		return TConst10;
	}

	if (isalpha(text[pointer]) || text[pointer] == '_')
	{
		while (pointer < text.size() && (isalnum(text[pointer]) || text[pointer] == '_'))
		{
			lex += text[pointer++];
		}
		auto it = keywordMap.find(lex);
		if (it != keywordMap.end())
		{
			return it->second;
		}
		return TIdent;
	}

	char c = text[pointer++];
	lex = c;
	switch (c)
	{
	case ',': return TComma;
	case ';': return TSemicolon;
	case '(': return TLeftBracket;
	case ')': return TRightBracket;
	case '{': return TLeftBrace;
	case '}': return TRightBrace;
	case '[': return TLeftSquareBracket;
	case ']': return TRightSquareBracket;
	case '+': return TAdd;
	case '-': return TSub;
	case '*': return TMul;
	case '%': return TMod;
	case '=':
		if (pointer < text.size() && text[pointer] == '=')
		{
			pointer++;
			lex += '=';
			return TEq;
		}
		return TEval;
	case '!':
		if (pointer < text.size() && text[pointer] == '=')
		{
			pointer++;
			lex += '=';
			return TNe;
		}
		PrintError("Unexpected token", lex);
		return TErr;
	case '>':
		if (pointer < text.size() && text[pointer] == '=')
		{
			pointer++;
			lex += '=';
			return TGe;
		}
		return TGt;
	case '<':
		if (pointer < text.size() && text[pointer] == '=')
		{
			pointer++;
			lex += '=';
			return TLe;
		}
		return TLt;
	default:
		PrintError("Lexical error at", lex);
		return TErr;
	}
}

void TScanner::GetData(const std::string& fileName)
{
	std::ifstream file(fileName);
	if (!file.is_open())
	{
		PrintError("Error opening file", fileName);
		return;
	}
	std::stringstream buffer;
	buffer << file.rdbuf();
	text = buffer.str();
	std::cout << text << std::endl;
	std::cout << "_____________________________________________________________________________________________________" << std::endl;
}