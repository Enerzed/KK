/* ll1.cpp */

#include "ll1.hpp"


TLL1::TLL1(TScanner* scanner)
{
	this->scanner = scanner;
	translate = new Translate(scanner);
	stack.push_back(TEnd);
	stack.push_back(TPROGRAM);
	prevToken = TEnd;
	prevLex[0] = '\0';
	NextToken();
}

TLL1::~TLL1()
{
	stack.clear();
	delete translate;
}

void TLL1::NextToken()
{
	currentToken = scanner->Scanner(lex);
	//std::cout << lex << std::endl;
}

bool TLL1::Analyze()
{
	while (!stack.empty())
	{
		int top = stack.back();
		stack.pop_back();

		// Semantic
		if (top >= 200)
		{
			translate->ExecuteAction(top, prevLex, prevToken);
		}
		// Terminal
		else if (top < 100)
		{
			if (top == currentToken)
			{
				if (top == TEnd) break;
				prevToken = currentToken;
				prevLex = lex;
				NextToken();
			}
			else
			{
				scanner->PrintError("Wrong token ", lex);
			}
		}
		// Nonterminal
		else
		{
			switch (top)
			{
			case TPROGRAM:
				stack.push_back(TDESCRIPTORS);
				break;

			case TDESCRIPTORS:
				if (currentToken == TInt || currentToken == TShort ||
					currentToken == TLong || currentToken == T__Int64 ||
					currentToken == TChar || currentToken == TVoid)
				{
					stack.push_back(TDESCRIPTORS);
					stack.push_back(TDESCRIPTOR);
				}
				else if (currentToken == TEnd) {}
				else scanner->PrintError("Expected descriptor or EOF got ", lex);
				break;

			case TDESCRIPTOR:
				if (currentToken == TInt || currentToken == TShort ||
					currentToken == TLong || currentToken == T__Int64 ||
					currentToken == TChar)
					stack.push_back(TDATA);
				else if (currentToken == TVoid)
					stack.push_back(TFUNCTION);
				else scanner->PrintError("Expected type or void got ", lex);
				break;

			case TDATA:
				stack.push_back(T_ACTION_END_DATA);
				stack.push_back(TSemicolon);
				stack.push_back(TLIST);
				stack.push_back(TTYPE);
				stack.push_back(T_ACTION_START_DATA);
				break;

			case TTYPE:
				if (currentToken == TInt || currentToken == TShort ||
					currentToken == TLong || currentToken == T__Int64 ||
					currentToken == TChar)
				{
					stack.push_back(T_ACTION_SET_TYPE);
					stack.push_back(currentToken);
				}
				else scanner->PrintError("Expected type got", lex);
				break;

			case TLIST:
				stack.push_back(TLIST_END);
				stack.push_back(TELEMENT);
				break;

			case TLIST_END:
				if (currentToken == TComma)
				{
					stack.push_back(TLIST_END);
					stack.push_back(TELEMENT);
					stack.push_back(TComma);
				}
				else if (currentToken != TSemicolon)
					scanner->PrintError("Expected comma or semicolon got ", lex);
				break;

			case TELEMENT:
				if (currentToken == TIdent)
				{
					stack.push_back(TELEMENT_END);
					stack.push_back(T_ACTION_SET_ID);
					stack.push_back(TIdent);
				}
				else scanner->PrintError("Expected identifier got ", lex);
				break;

			case TELEMENT_END:
				if (currentToken == TLeftSquareBracket)
				{
					stack.push_back(TARRAY_END);
					stack.push_back(TRightSquareBracket);
					stack.push_back(TSIZE);
					stack.push_back(TLeftSquareBracket);
					stack.push_back(T_ACTION_START_ARRAY);
				}
				else if (currentToken == TEval)
				{
					int saved_pos = scanner->GetPointer();
					std::string next_lex;
					int nextToken = scanner->Scanner(next_lex);
					scanner->SetPointer(saved_pos);
					if (nextToken == TLeftBrace)
					{
						stack.push_back(T_ACTION_END_ARRAY);
						stack.push_back(TRightBrace);
						stack.push_back(TARRAY_LIST);
						stack.push_back(T_ACTION_INIT_VALUE);
						stack.push_back(TLeftBrace);
						stack.push_back(TEval);
					}
					else
					{
						stack.push_back(T_ACTION_INIT_VALUE);
						stack.push_back(TEXPRESSION);
						stack.push_back(TEval);
					}
				}
				else if (currentToken != TComma && currentToken != TSemicolon)
					scanner->PrintError("Expected =, [ or ;", lex);
				break;

			case TARRAY_END:
				if (currentToken == TEval)
				{
					stack.push_back(T_ACTION_END_ARRAY);
					stack.push_back(TRightBrace);
					stack.push_back(TARRAY_EXPRESSION);
					stack.push_back(T_ACTION_INIT_VALUE);
					stack.push_back(TLeftBrace);
					stack.push_back(TEval);
				}
				break;

			case TSIZE:
				if (currentToken == TConst10 || currentToken == TConst16)
				{
					stack.push_back(T_ACTION_SET_ARRAY_SIZE);
					stack.push_back(currentToken);
				}
				else scanner->PrintError("Expected const10 or const16 got ", lex);
				break;

			case TARRAY_EXPRESSION:
				if (currentToken == TIdent || currentToken == TConst10 ||
					currentToken == TConst16 || currentToken == TLeftBracket ||
					currentToken == TAdd || currentToken == TSub)
					stack.push_back(TARRAY_LIST);
				else if (currentToken != TRightBrace)
					scanner->PrintError("Expected expression or } got ", lex);
				break;

			case TARRAY_LIST:
				stack.push_back(TARRAY_LIST_END);
				stack.push_back(T_ACTION_INIT_VALUE);
				stack.push_back(TEXPRESSION);
				break;

			case TARRAY_LIST_END:
				if (currentToken == TComma)
				{
					stack.push_back(TARRAY_LIST_END);
					stack.push_back(T_ACTION_INIT_VALUE);
					stack.push_back(TEXPRESSION);
					stack.push_back(TComma);
				}
				else if (currentToken != TRightBrace)
					scanner->PrintError("Expected , or } got ", lex);
				break;

			case TFUNCTION:
				if (currentToken == TVoid)
				{
					stack.push_back(T_ACTION_END_FUNC);
					stack.push_back(TCOMPOSITE_OPERATOR);
					stack.push_back(TRightBracket);
					stack.push_back(TLeftBracket);
					stack.push_back(T_ACTION_START_FUNC);
					stack.push_back(TIdent);
					stack.push_back(TVoid);
				}
				else scanner->PrintError("Expected void got ", lex);
				break;

			case TCOMPOSITE_OPERATOR:
				stack.push_back(T_ACTION_RETURN_LEVEL);
				stack.push_back(TOPERATORS_AND_DESCRIPTORS);
				stack.push_back(T_ACTION_NEW_LEVEL);
				stack.push_back(TLeftBrace);
				break;

			case TOPERATORS_AND_DESCRIPTORS:
				stack.push_back(TOPERATORS_AND_DESCRIPTORS_END);
				stack.push_back(TBLOCK_ELEMENT);
				break;

			case TOPERATORS_AND_DESCRIPTORS_END:
				if (currentToken == TInt || currentToken == TShort ||
					currentToken == TLong || currentToken == T__Int64 ||
					currentToken == TChar || currentToken == TLeftBrace ||
					currentToken == TSemicolon || currentToken == TIf ||
					currentToken == TIdent)
				{
					stack.push_back(TOPERATORS_AND_DESCRIPTORS_END);
					stack.push_back(TBLOCK_ELEMENT);
				}
				else if (currentToken == TRightBrace)
				{
					NextToken();
				}
				else
				{
					scanner->PrintError("Expected operator or descriptor got ", lex);
				}
				break;

			case TBLOCK_ELEMENT:
				if (currentToken == TInt || currentToken == TShort ||
					currentToken == TLong || currentToken == T__Int64 ||
					currentToken == TChar)
					stack.push_back(TDATA);
				else if (currentToken == TLeftBrace)
					stack.push_back(TCOMPOSITE_OPERATOR);
				else if (currentToken == TSemicolon)
					stack.push_back(TEMPTY_OPERATOR);
				else if (currentToken == TIf)
					stack.push_back(TCONDITION);
				else if (currentToken == TIdent)
					stack.push_back(TOPERATOR_ID);
				else scanner->PrintError("Wrong description of data or function", lex);
				break;

			case TEMPTY_OPERATOR:
				if (currentToken == TSemicolon)
					stack.push_back(TSemicolon);
				else scanner->PrintError("Expected ; got ", lex);
				break;

			case TCONDITION:
				if (currentToken == TIf)
				{
					stack.push_back(TCONDITION_END);
					stack.push_back(TCOMPOSITE_OPERATOR);
					stack.push_back(T_ACTION_GEN_IF);
					stack.push_back(TRightBracket);
					stack.push_back(TEXPRESSION);
					stack.push_back(TLeftBracket);
					stack.push_back(TIf);
				}
				else scanner->PrintError("Expected if got ", lex);
				break;

			case TCONDITION_END:
				if (currentToken == TElse)
				{
					stack.push_back(T_ACTION_GEN_ENDIF);
					stack.push_back(TCOMPOSITE_OPERATOR);
					stack.push_back(T_ACTION_GEN_ELSE);
					stack.push_back(TElse);
				}
				else
				{
					stack.push_back(T_ACTION_GEN_ENDIF);
				}
				break;

			case TOPERATOR_ID:
				if (currentToken == TIdent)
				{
					stack.push_back(TOPERATOR_ID_END);
					stack.push_back(TIdent);
				}
				else scanner->PrintError("Expected identifier got ", lex);
				break;

			case TOPERATOR_ID_END:
				if (currentToken == TLeftBracket)
				{
					stack.push_back(TSemicolon);
					stack.push_back(TRightBracket);
					stack.push_back(TLeftBracket);
					stack.push_back(T_ACTION_CALL_FUNC);
					stack.push_back(T_ACTION_FIND_ID_FOR_CALL);
				}
				else if (currentToken == TEval)
				{
					stack.push_back(TSemicolon);
					stack.push_back(T_ACTION_GEN_ASSIGN);
					stack.push_back(TEXPRESSION);
					stack.push_back(TEval);
					stack.push_back(T_ACTION_FIND_ID);
				}
				else if (currentToken == TLeftSquareBracket)
				{
					stack.push_back(TSemicolon);
					stack.push_back(T_ACTION_GEN_ARRAY_ASSIGN);
					stack.push_back(TEXPRESSION);
					stack.push_back(TEval);
					stack.push_back(T_ACTION_ARRAY_ELEM);
					stack.push_back(TRightSquareBracket);
					stack.push_back(TEXPRESSION);
					stack.push_back(TLeftSquareBracket);
					stack.push_back(T_ACTION_PUSH_OPERAND);
					stack.push_back(T_ACTION_FIND_ID);
				}
				else scanner->PrintError("Expected (, = or [", lex);
				break;

			case TEXPRESSION:
				stack.push_back(TEXPRESSION_END);
				stack.push_back(TCOMPARISON);
				break;

			case TEXPRESSION_END:
				if (currentToken == TEq || currentToken == TNe)
				{
					int op = currentToken;
					stack.push_back(TEXPRESSION_END);
					if (op == TEq) stack.push_back(T_ACTION_GEN_EQ);
					else stack.push_back(T_ACTION_GEN_NE);
					stack.push_back(TCOMPARISON);
					stack.push_back(op);
				}
				break;

			case TCOMPARISON:
				stack.push_back(TCOMPARISON_END);
				stack.push_back(TADDENDUM);
				break;

			case TCOMPARISON_END:
				if (currentToken == TLt || currentToken == TGt || currentToken == TLe || currentToken == TGe)
				{
					int op = currentToken;
					stack.push_back(TCOMPARISON_END);
					if (op == TLt) stack.push_back(T_ACTION_GEN_LT);
					else if (op == TGt) stack.push_back(T_ACTION_GEN_GT);
					else if (op == TLe) stack.push_back(T_ACTION_GEN_LE);
					else stack.push_back(T_ACTION_GEN_GE);
					stack.push_back(TADDENDUM);
					stack.push_back(op);
				}
				break;

			case TADDENDUM:
				stack.push_back(TADDENDUM_END);
				stack.push_back(TMULTIPLICAND);
				break;

			case TADDENDUM_END:
				if (currentToken == TAdd || currentToken == TSub)
				{
					int op = currentToken;
					stack.push_back(TADDENDUM_END);
					if (op == TAdd) stack.push_back(T_ACTION_GEN_ADD);
					else stack.push_back(T_ACTION_GEN_SUB);
					stack.push_back(TMULTIPLICAND);
					stack.push_back(op);
				}
				break;

			case TMULTIPLICAND:
				stack.push_back(TMULTIPLICAND_END);
				stack.push_back(TUNARY);
				break;

			case TMULTIPLICAND_END:
				if (currentToken == TMul || currentToken == TDiv || currentToken == TMod)
				{
					int op = currentToken;
					stack.push_back(TMULTIPLICAND_END);
					if (op == TMul) stack.push_back(T_ACTION_GEN_MUL);
					else if (op == TDiv) stack.push_back(T_ACTION_GEN_DIV);
					else stack.push_back(T_ACTION_GEN_MOD);
					stack.push_back(TUNARY);
					stack.push_back(op);
				}
				break;

			case TUNARY:
				if (currentToken == TAdd)
				{
					stack.push_back(TELEMENTARY);
					stack.push_back(TAdd);
				}
				else if (currentToken == TSub)
				{
					stack.push_back(T_ACTION_GEN_NEG);
					stack.push_back(TELEMENTARY);
					stack.push_back(TSub);
				}
				else
					stack.push_back(TELEMENTARY);
				break;

			case TELEMENTARY:
				if (currentToken == TIdent)
				{
					int saved_pos = scanner->GetPointer();
					std::string next_lex;
					int nextToken = scanner->Scanner(next_lex);
					scanner->SetPointer(saved_pos);
					if (nextToken == TLeftSquareBracket)
					{
						stack.push_back(T_ACTION_ARRAY_ELEM);
						stack.push_back(TRightSquareBracket);
						stack.push_back(TEXPRESSION);
						stack.push_back(TLeftSquareBracket);
						stack.push_back(T_ACTION_PUSH_OPERAND);
						stack.push_back(TIdent);
					}
					else
					{
						stack.push_back(T_ACTION_PUSH_OPERAND);
						stack.push_back(TIdent);
					}
				}
				else if (currentToken == TConst10 || currentToken == TConst16)
				{
					stack.push_back(T_ACTION_PUSH_OPERAND);
					stack.push_back(currentToken);
				}
				else if (currentToken == TLeftBracket)
				{
					stack.push_back(TRightBracket);
					stack.push_back(TEXPRESSION);
					stack.push_back(TLeftBracket);
				}
				else
				{
					scanner->PrintError("Expected identifier, constant or ( got", lex);
				}
				break;

			default:
				scanner->PrintError("Unknown token got", lex);
			}
		}
	}

	if (currentToken != TEnd)
	{
		scanner->PrintError("Expected end got ", lex);
		return false;
	}
	return true;
}