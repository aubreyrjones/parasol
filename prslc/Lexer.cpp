//
// Created by ARJ on 9/5/15.
//

#include "Lexer.h"

namespace prsl {

SymbolToken _symbolTokens[] = {
	SymbolToken(',', COMMA),
	SymbolToken('{', L_CURLY),
	SymbolToken('}', R_CURLY),
	SymbolToken(']', R_BRACKET),
	SymbolToken('(', L_PAREN),
	SymbolToken(')', R_PAREN),
	SymbolToken(':', COLON),
	SymbolToken('+', PLUS),
	SymbolToken('-', MINUS),
	SymbolToken('*', MULT),
	SymbolToken('/', DIV),
	SymbolToken('%', CROSS),
	SymbolToken('`', DOT),
	SymbolToken('~', NOT),
	SymbolToken('\\', LAMBDA)
};
size_t _nSymbols = sizeof(_symbolTokens) / sizeof(SymbolToken);

DigraphToken _digraphTokens[] = {
	DigraphToken('=', EQUALS, '>', GOESTO)
};
size_t _nDigraphs = sizeof(_digraphTokens) / sizeof(DigraphToken);

KeywordToken _keywordTokens[] = {
	KeywordToken("def", DEF),
	KeywordToken("if", IF),
	KeywordToken("else", ELSE)
};
size_t _nKeywords = sizeof(_keywordTokens) / sizeof(KeywordToken);

size_t StringTable::pushString(std::string const &s) {
	PrevMap::iterator it = previousLocations.find(s);
	if (it != previousLocations.end()){
		return (*it).second;
	}

	size_t idx = strings.size();
	previousLocations.emplace(s, idx);

	strings.push_back(s);

	return idx;
}


std::string const &StringTable::getString(size_t index) {
	return strings[index];
}

std::string lookupToken(int token) {
	std::string retval;

	for (size_t i = 0; i < _nSymbols; i++){
		if (token == std::get<1>(_symbolTokens[i])){
			retval.push_back(std::get<0>(_symbolTokens[i]));
			return retval;
		}
	}

	for (size_t i = 0; i < _nDigraphs; i++){
		if (token == std::get<1>(_digraphTokens[i])){
			retval.push_back(std::get<0>(_digraphTokens[i]));
			return retval;
		}
		else if (token == std::get<3>(_digraphTokens[i])){
			retval.push_back(std::get<0>(_digraphTokens[i]));
			retval.push_back(std::get<1>(_digraphTokens[i]));
		}
	}

	for (size_t i = 0; i < _nKeywords; i++){
		if (token == std::get<1>(_keywordTokens[i])){
			return std::get<0>(_keywordTokens[i]);
		}
	}
}

}