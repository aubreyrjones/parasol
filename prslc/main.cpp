#include "Lexer.h"
#include "Parser.h"
#include "DOTVisitor.h"
#include "Compiler.h"
#include <fstream>
#include <iterator>

typedef std::istreambuf_iterator<char> FileCharIterator;

int main(int argc, char **argv){

	if (argc < 2){
		std::cout << "Need source input filename." << std::endl;
		return 1;
	}

	prsl::LinkUnit linkUnit;

	try {
		std::ifstream sourceFile(argv[1]);

		linkUnit.addModule(FileCharIterator(sourceFile), FileCharIterator(), argv[1]);

		linkUnit.link();

		prsl::DOTVisitor dotter("ast.dot");
		dotter.dotify(linkUnit.getModules().front());
	}
	catch (prsl::ParseError &pe){
		std::cout << pe.what() << std::endl;
		return 2;
	}
}