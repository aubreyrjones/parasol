//
// Created by ARJ on 9/5/15.
//

#include "Lexer.h"

#ifndef PARASOL_PARSER_H
#define PARASOL_PARSER_H

namespace prsl {

class Parser {
protected:
	StringTable strings; /// all strings encountered during lexing

	void *lemonParser; /// the opaque parser object we get from lemon's output

	PRSLToken currentToken;

public:
	Parser();
	~Parser();

	Parser(Parser & o) = delete;
	Parser& operator=(Parser &o) = delete;
	Parser(Parser && o) = delete; // for now
	Parser& operator=(Parser &&o) = delete; // for now


	void offerToken(PRSLToken token); /// offer the next token
	StringTable *getStrings() { return &strings; }; /// get the string table (to give to the lexer)

	void error();
};

}

#endif //PARASOL_PARSER_H
