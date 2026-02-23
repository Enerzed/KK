#pragma once


#include <cstdlib>
#include <string>


#define MAX_TEXT 16384
#define MAX_LEX 64
#define MAX_DEC_LEX 20
#define MAX_HEX_LEX 19

#define MAX_KEYWORD 16

typedef char TypeLex[MAX_LEX];
typedef char TypeMod[MAX_TEXT];


/** Tokens **/
#define TIdent 1
#define TConst10 11
#define TConst16 12
#define TInt 21
#define TShort 22
#define TLong 23
#define T__Int64 24
#define TChar 25
#define TIf 31
#define TElse 32
#define TMain 33
#define TVoid 34
#define TComma 41
#define TSemicolon 42
#define TLeftBracket 43
#define TRightBracket 44
#define TLeftBrace 45
#define TRightBrace 46
#define TLeftSquareBracket 47
#define TRightSquareBracket 48
#define TAdd 51
#define TSub 52
#define TMul 53
#define TDiv 54
#define TMod 55
#define TEval 56
#define TEq 57
#define TNe 58
#define TGt 59
#define TLt 60
#define TGe 61
#define TLe 62
#define TEnd 71
#define TErr 72


/** Nonterminals **/
#define TPROGRAM 100
#define TDESCRIPTORS 101
#define TDESCRIPTOR 102
#define TDATA 103
#define TTYPE 104
#define TLIST 105
#define TLIST_END 106
#define TELEMENT 107
#define TELEMENT_END 108
#define TARRAY_END 109
#define TSIZE 110
#define TARRAY_EXPRESSION 111
#define TARRAY_LIST 112
#define TARRAY_LIST_END 113
#define TFUNCTION 114
#define TCOMPOSITE_OPERATOR 115
#define TOPERATORS_AND_DESCRIPTORS 116
#define TOPERATORS_AND_DESCRIPTORS_END 117
#define TBLOCK_ELEMENT 118
#define TOPERATOR 119
#define TEMPTY_OPERATOR 120
#define TCONDITION 121
#define TCONDITION_END 122
#define TOPERATOR_ID 123
#define TOPERATOR_ID_END 124
#define TEXPRESSION 125
#define TEXPRESSION_END 126
#define TCOMPARISON 127
#define TCOMPARISON_END 128
#define TADDENDUM 129
#define TADDENDUM_END 130
#define TMULTIPLICAND 131
#define TMULTIPLICAND_END 132
#define TUNARY 133
#define TELEMENTARY 134