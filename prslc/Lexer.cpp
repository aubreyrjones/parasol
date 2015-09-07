//
// Created by ARJ on 9/5/15.
//

#include "Lexer.h"

namespace prsl {

using std::get;

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
	SymbolToken('\\', LAMBDA),
	SymbolToken('.', SWIZZLE)
};
size_t _nSymbols = sizeof(_symbolTokens) / sizeof(SymbolToken);

DigraphToken _digraphTokens[] = {
	DigraphToken('=', EQUALS, '>', GOESTO),
	DigraphToken('=', EQUALS, '=', EQ),
	DigraphToken('&', B_AND, '&', L_AND),
	DigraphToken('|', B_OR, '|', L_OR),
	DigraphToken('!', NOT, '=', NOT_EQ),
	DigraphToken('<', LESS, '=', LESS_EQ),
	DigraphToken('>', GREATER, '=', GREATER_EQ)
};
size_t _nDigraphs = sizeof(_digraphTokens) / sizeof(DigraphToken);

KeywordToken _keywordTokens[] = {
	KeywordToken("def", DEF),
	KeywordToken("let", LET),
	KeywordToken("in", IN),
	KeywordToken("struct", STRUCT),
	KeywordToken("include", INCLUDE),
	KeywordToken("as", AS)
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
		if (token == get<1>(_symbolTokens[i])){
			retval.push_back(get<0>(_symbolTokens[i]));
			return retval;
		}
	}

	for (size_t i = 0; i < _nDigraphs; i++){
		if (token == get<1>(_digraphTokens[i])){
			retval.push_back(get<0>(_digraphTokens[i]));
			return retval;
		}
		else if (token == get<3>(_digraphTokens[i])){
			retval.push_back(get<0>(_digraphTokens[i]));
			retval.push_back(get<2>(_digraphTokens[i]));
			return retval;
		}
	}

	for (size_t i = 0; i < _nKeywords; i++){
		if (token == get<1>(_keywordTokens[i])){
			return get<0>(_keywordTokens[i]);
		}
	}
}

}