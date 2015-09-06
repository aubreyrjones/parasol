#include "Lexer.h"
#include "Parser.h"

#include <iostream>

int main(int argc, char **argv){

	std::string sourceCode("foo {  } ");

	prsl::Parser parser;

	prsl::Lexer<std::string::iterator> lexer(sourceCode.begin(), sourceCode.end(), parser.getStrings());

	PRSLToken token;

	while (lexer.next(token)){
		parser.offerToken(token);
	}
}