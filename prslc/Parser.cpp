//
// Created by ARJ on 9/5/15.
//

#include "Parser.h"

// these are all defined in prsl_grammar.c, as part of the lemon output.

void* ParseAlloc(void *(*mallocProc)(size_t));
void ParseFree(void *p, void (*freeProc)(void*));
void Parse(void *yyp, int yymajor, PRSLToken yyminor, prsl::Parser *);

namespace prsl {

Parser::Parser() {
	lemonParser = ParseAlloc(malloc);
}

Parser::~Parser() {
	ParseFree(lemonParser, free);
}

void Parser::offerToken(PRSLToken token) {
	currentToken = token;
	Parse(lemonParser, token.tokenType, token, this);
}

void Parser::error() {
	std::cout << "Syntax error on line " << currentToken.lineNumber << "." << std::endl;
}

}
