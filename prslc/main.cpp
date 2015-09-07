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
		prsl::ast::Module *module = prsl::parseModule(FileCharIterator(sourceFile), FileCharIterator());
	}
	catch (prsl::ParseError &pe){
		std::cout << pe.what() << std::endl;
		return 2;
	}
}