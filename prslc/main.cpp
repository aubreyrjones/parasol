#include "Lexer.h"
#include "Parser.h"
#include <fstream>
#include <iterator>

typedef std::istreambuf_iterator<char> FileCharIterator;

int main(int argc, char **argv){

	if (argc < 2){
		std::cout << "Need source input filename." << std::endl;
		return 1;
	}

	try {
		std::ifstream sourceFile(argv[1]);
		prsl::Parser parser;

		prsl::Lexer<FileCharIterator> lexer(FileCharIterator(sourceFile), FileCharIterator(), parser.getStrings());

		PRSLToken token;

		size_t tokenCount = 0;
		while (lexer.next(token)) {
			std::cout << token.tokenType << " ";
			parser.offerToken(token);
			tokenCount++;
			if (lexer.atEnd()){
				break; // end of input
			}
		}

		std::cout << "Tokens parsed: " << tokenCount << std::endl;
	}
	catch (prsl::ParseError &pe){
		std::cout << pe.what() << std::endl;
		return 2;
	}
}