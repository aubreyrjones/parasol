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
	SymbolToken('{', L_CURLY)
};
size_t _nSymbols = sizeof(_symbolTokens) / sizeof(SymbolToken);

DigraphToken _digraphTokens[] = {
	DigraphToken('=', EQUALS, '>', GOESTO)
};
size_t _nDigraphs = sizeof(_digraphTokens) / sizeof(DigraphToken);

KeywordToken _keywordTokens[] = {
	KeywordToken("def", DEF)
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

}