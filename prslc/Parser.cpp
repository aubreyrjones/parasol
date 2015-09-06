//
// Created by ARJ on 9/5/15.
//

#include "Parser.h"

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

void Parser::error() {
	std::cout << "Syntax error on line " << currentToken.lineNumber << "." << std::endl;
}

}
