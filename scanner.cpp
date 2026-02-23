#include "scanner.hpp"


TScanner::TScanner(std::string fileName)
{
	GetData(fileName);
	SetPointer(0);
}


TypeLex keyword[MAX_KEYWORD] =
{
	"int", "short", "long", "__int64", "char", "if", "else", "main", "void"
};


int indexKeyword[MAX_KEYWORD] =
{
	TInt, TShort, TLong, T__Int64, TChar, TIf, TElse, TMain, TVoid
};


void TScanner::SetPointer(int newPointer)
{
	pointer = newPointer;
}


int TScanner::GetPointer()
{
	return pointer;
}


void TScanner::PrintError(std::string error, std::string text)
{
	if (breakLinePositions.empty())
		positionInLine = pointer;
	else
		positionInLine = pointer - breakLinePositions.back();
	if (text[0] == '\0')
		std::cout << "Line " << lineCounter << " position " << positionInLine << ": " << error << std::endl;
	else
		std::cout << "Line " << lineCounter << " position " << positionInLine << ": " << error << " " << text << std::endl;
	exit(1);
}


void TScanner::PrintWarning(std::string warning, std::string text)
{
	if (breakLinePositions.empty())
		positionInLine = pointer;
	else
		positionInLine = pointer - breakLinePositions.back();
	if (text[0] == '\0')
		std::cout << "Line " << lineCounter << " position " << positionInLine << ": " << warning << std::endl;
	else
		std::cout << "Line " << lineCounter << " position " << positionInLine << ": " << warning << " " << text << std::endl;
}


int TScanner::Scanner(TypeLex lex)
{
	int i = 0;
	lex[0] = '\0';
start:
	while (text[pointer] == ' ' || text[pointer] == '\t' || text[pointer] == '\n') {
		if (text[pointer] == '\n')
			if (std::find(breakLinePositions.begin(), breakLinePositions.end(), pointer) == breakLinePositions.end())
			{
				lineCounter++;
				breakLinePositions.push_back(pointer);
			}
		pointer++;
	}
	i = 0;

	// End of program
	if (text[pointer] == '\0')
	{
		lex[i++] = '#';
		lex[i] = '\0';
		return TEnd;
	}

	// Comments one-line and multi-line
	if (text[pointer] == '/')
	{
		pointer++;
		if (text[pointer] == '/')
		{
			//std::cout << "Encountered comment line" << std::endl;
			pointer++;
			while (text[pointer] != '\n' && text[pointer] != '\0')
				pointer++;
			goto start;
		}
		else if (text[pointer] == '*')
		{
			//std::cout << "Encountered comment block" << std::endl;
			pointer++;
			while (text[pointer] != '*' || text[pointer + 1] != '/')
			{
				if (text[pointer] == '\0')
				{
					PrintError("Unterminated multi-line comment", "");
					return TErr;
				}
				pointer++;
			}
			pointer += 2;
			goto start;
		}
		else
		{
			// Division
			lex[i++] = '/';
			lex[i++] = '\0';
			return TDiv;
		}
	}

	// Hexadecimal constant
	if (text[pointer] == '0' && text[pointer + 1] == 'x')
	{
		lex[i++] = text[pointer++];
		lex[i++] = text[pointer++];
		if (!isxdigit(text[pointer]))
		{ // Check for at least one hex digit
			lex[i] = '\0';
			{
				PrintError("Invalid hexadecimal constant", lex);
				return TErr;
			}
			while (isxdigit(text[pointer]) && i < MAX_HEX_LEX - 1)
			{
				lex[i++] = text[pointer++];
			}
			lex[i] = '\0';
			if (i == MAX_HEX_LEX - 1 && isxdigit(text[pointer]))
			{
				lex[i] = '\0';
				while (isxdigit(text[pointer]))
					pointer++;
				PrintError("Hexadecimal constant exceeds maximum lexeme length", lex);
				return TErr;
			}
			return TConst16;
		}

		// Decimal constants
		if (isdigit(text[pointer]))
		{
			while (isdigit(text[pointer]) && i < MAX_DEC_LEX - 1)
			{ // Added length check
				lex[i++] = text[pointer++];
			}
			lex[i] = '\0';
			if (i == MAX_DEC_LEX - 1 && isdigit(text[pointer]))
			{
				while (isdigit(text[pointer]))
					pointer++;
				PrintError("Decimal constant exceeds maximum lexeme length", lex);
				return TErr;
			}
			return TConst10;
		}

		// Identifiers
		if (isalpha(text[pointer]) || text[pointer] == '_')
		{
			while ((isalnum(text[pointer]) || text[pointer] == '_') && i < MAX_LEX - 1)
			{ // Added length check
				lex[i++] = text[pointer++];
			}
			lex[i] = '\0';
			if (i == MAX_LEX - 1 && (isalnum(text[pointer]) || text[pointer] == '_'))
			{
				while (isalnum(text[pointer]) || text[pointer] == '_')
					pointer++;
				PrintError("Identifier exceeds maximum lexeme length", lex);
				return TErr;
			}

			for (int j = 0; j < MAX_KEYWORD; j++)
			{
				if (strcmp(lex, keyword[j]) == 0)
				{
					return indexKeyword[j];
				}
			}
			return TIdent;
		}

		// Operators
		switch (text[pointer])
		{
		case ',': pointer++; lex[i++] = ','; lex[i] = '\0'; return TComma;
		case ';': pointer++; lex[i++] = ';'; lex[i] = '\0'; return TSemicolon;
		case '(': pointer++; lex[i++] = '('; lex[i] = '\0'; return TLeftBracket;
		case ')': pointer++; lex[i++] = ')'; lex[i] = '\0'; return TRightBracket;
		case '{': pointer++; lex[i++] = '{'; lex[i] = '\0'; return TLeftBrace;
		case '}': pointer++; lex[i++] = '}'; lex[i] = '\0'; return TRightBrace;
		case '[': pointer++; lex[i++] = '['; lex[i] = '\0'; return TLeftSquareBracket;
		case ']': pointer++; lex[i++] = ']'; lex[i] = '\0'; return TRightSquareBracket;
		case '+': pointer++; lex[i++] = '+'; lex[i] = '\0'; return TAdd;
		case '-': pointer++; lex[i++] = '-'; lex[i] = '\0'; return TSub;
		case '*': pointer++; lex[i++] = '*'; lex[i] = '\0'; return TMul;
		case '%': pointer++; lex[i++] = '%'; lex[i] = '\0'; return TMod;
		case '=':
			pointer++;
			lex[i++] = '=';
			if (text[pointer] == '=')
			{
				pointer++;
				lex[i++] = '=';
				lex[i] = '\0';
				return TEq;
			}
			else
			{
				lex[i] = '\0';
				return TEval;
			}
		case '!':
			pointer++;
			lex[i++] = '!';
			if (text[pointer] == '=')
			{
				pointer++;
				lex[i++] = '=';
				lex[i] = '\0';
				return TNe;
			}
			// Not implementing NOT operation
			else
			{
				lex[i] = '\0';
				PrintError("Unexpected token", lex);
				return TErr;
			}
		case '>':
			pointer++;
			lex[i++] = '>';
			if (text[pointer] == '=')
			{
				pointer++;
				lex[i++] = '=';
				lex[i] = '\0';
				return TGe;
			}
			else
			{
				lex[i] = '\0';
				return TGt;
			}
		case '<':
			pointer++;
			lex[i++] = '<';
			if (text[pointer] == '=')
			{
				pointer++;
				lex[i++] = '=';
				lex[i] = '\0';
				return TLe;
			}
			else
			{
				lex[i] = '\0';
				return TLt;
			}
		default:
			lex[i++] = text[pointer];
			lex[i] = '\0';
			PrintError("Lexical error at", lex);
			pointer++;
			return TErr;
		}
	}
}


void TScanner::GetData(std::string fileName)
{
	std::ifstream file(fileName);
	if (file.is_open())
	{
		std::stringstream buffer;
		buffer << file.rdbuf();

		strncpy(text, buffer.str().c_str(), MAX_TEXT - 1);
		text[MAX_TEXT - 1] = '\0';

		std::cout << text << std::endl;
		std::cout << "_____________________________________________________________________________________________________	" << std::endl;
	}
	else
	{
		PrintError("Error opening file", fileName);
		return;
	}
}