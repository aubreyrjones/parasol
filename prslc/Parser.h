//
// Created by ARJ on 9/5/15.
//

#include "Lexer.h"
#include "ParasolPT.h"

#ifndef PARASOL_PARSER_H
#define PARASOL_PARSER_H

namespace prsl {

class Parser {
protected:
	StringTable strings; /// all strings encountered during lexing

	void *lemonParser; /// the opaque parser object we get from lemon's output

	PRSLToken currentToken;

	ast::NodeList *globals = nullptr;

public:
	Parser();
	~Parser();

	Parser(Parser & o) = delete;
	Parser& operator=(Parser &o) = delete;
	Parser(Parser && o) = delete; // for now
	Parser& operator=(Parser &&o) = delete; // for now


	void offerToken(PRSLToken token); /// offer the next token
	StringTable *getStrings() { return &strings; }; /// get the string table (to give to the lexer)

	std::string const& getString(size_t index) { return strings.getString(index); };

	void error();
	void success();

	void pushAST(ast::NodeList *globals);

	ast::NodeList* getGlobals() { return globals; };
};



inline std::string const& getstr(prsl::Parser *p, size_t index){
	return p->getString(index);
}



template <class ITER>
ast::Module *parseModule(ITER start, ITER end) {

	Parser parser;

	prsl::Lexer<ITER> lexer(start, end, parser.getStrings());

	PRSLToken token;

	size_t tokenCount = 0;
	while (true) {
		if (lexer.atEnd()){
			break; // end of input
		}
		if (!lexer.next(token)) break;

		parser.offerToken(token);
		tokenCount++;
	}

	std::cout << "Tokens parsed: " << tokenCount << std::endl;

	return new ast::Module("unnamed", parser.getGlobals());
}

}

#endif //PARASOL_PARSER_H
