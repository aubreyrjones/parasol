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
	void finish(); /// finish parsing
	StringTable *getStrings() { return &strings; }; /// get the string table (to give to the lexer)

	std::string const& getString(size_t index) { return strings.getString(index); };

	void error();
	void success();
};

inline std::string const& getstr(prsl::Parser *p, size_t index){
	return p->getString(index);
}

}

#endif //PARASOL_PARSER_H
