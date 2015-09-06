//
// Created by ARJ on 9/5/15.
//

#include "Lexer.h"

namespace prsl {

SymbolToken _symbol_tokens[] = {
	SymbolToken('{', L_CURLY),
	SymbolToken('}', R_CURLY),
	SymbolToken('[', L_BRACKET),
	SymbolToken(']', R_BRACKET),
	SymbolToken('{', L_CURLY)
};
size_t _nSymbols = sizeof(_symbol_tokens) / sizeof(SymbolToken);

size_t StringTable::pushString(std::string const &s) {
	strings.push_back(s);
	return strings.size() - 1;
}


std::string const &StringTable::getString(size_t index) {
	return strings[index];
}

}