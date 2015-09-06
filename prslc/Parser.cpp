//
// Created by ARJ on 9/5/15.
//

#include "Parser.h"
#include "Token.h"

// these are all defined in prsl_grammar.c, as part of the lemon output.

void* PRSLParseAlloc(void *(*mallocProc)(size_t));
void PRSLParseFree(void *p, void (*freeProc)(void*));
void PRSLParse(void *yyp, int yymajor, PRSLToken yyminor, prsl::Parser *);

namespace prsl {

Parser::Parser() {
	lemonParser = PRSLParseAlloc(malloc);
}

Parser::~Parser() {
	PRSLParseFree(lemonParser, free);
}

void Parser::offerToken(PRSLToken token) {
	currentToken = token;
	PRSLParse(lemonParser, token.tokenType, token, this);
}

void Parser::finish() {
	PRSLToken fakeTok;
	PRSLParse(lemonParser, 0, fakeTok, this);
}

void Parser::error() {
	std::cout << "Syntax error on line " << currentToken.lineNumber << ". Token type: " << currentToken.tokenType;

	switch (currentToken.tokenType) {
	case INT_LIT:
		std::cout << " Value: " << currentToken.value.intValue;
		break;
	//case FLOAT_LIT:
//		std::cout << " Value: " << currentToken.value.floatValue;
//		break;
	case ID:
		std::cout << " Value: " << getString(currentToken.value.stringIndex);
		break;
	default:
		break;
	}

	std::cout << std::endl;
}

void Parser::success() {
	std::cout << "Parse successful." << std::endl;
}
}
