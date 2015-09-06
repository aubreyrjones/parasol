#include "Lexer.h"

#include <iostream>

int main(int argc, char **argv){

	std::string sourceCode("test {} ");

	prsl::StringTable *strings = new prsl::StringTable;

	prsl::Lexer<std::string::iterator> lexer(sourceCode.begin(), sourceCode.end(), strings);

	PRSLToken token;

	while (lexer.next(token)){
		//parse!
		std::cout << "token" << std::endl;
	}
}