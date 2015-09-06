//
// Created by ARJ on 9/5/15.
//

#include "Token.h"
#include "prsl_grammar.h"
#include <string>
#include <vector>
#include <tuple>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <unordered_map>

#ifndef PARASOL_LEXER_H
#define PARASOL_LEXER_H

namespace prsl {

class ParseError : public std::runtime_error {
public:
	using std::runtime_error::runtime_error;
};

/** Used to intern strings found by the lexer. */
class StringTable {
protected:
	std::vector<std::string> strings;

	typedef std::unordered_map<std::string, size_t> PrevMap;

	PrevMap previousLocations;

public:
	size_t pushString(std::string const& s);  // add a string to the table

	std::string const& getString(size_t index); // get a string
};


typedef std::tuple<char, int> SymbolToken;
extern SymbolToken _symbolTokens[];
extern size_t _nSymbols;

typedef std::tuple<char, int, char, int> DigraphToken;
extern DigraphToken _digraphTokens[];
extern size_t _nDigraphs;

typedef std::tuple<const char*, int> KeywordToken;
extern KeywordToken _keywordTokens[];
extern size_t _nKeywords;

inline bool isSpace(char const& c) {
	return std::isblank(c) || c == '\n' || c == '\r';
}

/**
 *
 * Main lexer template.
 *
 * */
template <class ITER = std::string::iterator>
class Lexer {

protected:
	ITER cur;
	ITER end;

	size_t currentLine = 1;

	std::string curString;
	StringTable *stringTable;

	void skipWhitespace() {
		while (cur != end) {
			char c = *cur;
			switch (c) {
			case '\n':
				++currentLine;
			case ' ':
			case '\r':
			case '\t':
				++cur;
				continue;
			default:
				return;
			}
		}
	}

	bool nextSymbol(PRSLToken & retval) {
		// check digraphs
		for (size_t i = 0; i < _nDigraphs; i++){
			if (*cur == std::get<0>(_digraphTokens[i])){
				++cur;

				if (*cur == std::get<2>(_digraphTokens[i])){
					++cur;
					retval.tokenType = std::get<3>(_digraphTokens[i]);
					return true;
				}
				else {
					retval.tokenType = std::get<1>(_digraphTokens[i]);
				}
			}
		}

		// check monographs (is that it? or unigraph? who cares?)
		for (size_t i = 0; i < _nSymbols; i++){
			if (*cur == std::get<0>(_symbolTokens[i])){
				retval.tokenType = std::get<1>(_symbolTokens[i]);
				++cur;
				return true;
			}
		}

		return false;
	}

	bool nextNumber(PRSLToken & retval) {
		curString.clear();

		bool isNegative = false;
		bool isFloat = false;

		if (*cur == '-'){
			isNegative = true;
			curString.push_back(*cur);
			++cur;
		}

		while (cur != end) {
			char c = *cur;
			if (c == '.' && !isFloat) { // only one . allowed.
				isFloat = true;
				curString.push_back(c);
			}
			else if (c > '0' && c < '9') {
				curString.push_back(c);
			}
			else {
				break;
			}

			++cur;
		}

		std::istringstream sstream(curString);

		if (curString.size() > 0){
			if (isFloat){
				if (!(sstream >> retval.value.floatValue)){
					throw ParseError("Failed to lex float literal.");
				}
				//retval.tokenType = FLOAT_LIT;
			}
			else {
				if (!(sstream >> retval.value.intValue)){
					throw ParseError("Failed to lex integer literal.");
				}
				retval.tokenType = INT_LIT;
			}
			return true;
		}

		return false;
	}

	bool nextID(PRSLToken & retval) {
		curString.clear();

		char c = *cur;

		if (!(std::isalpha(c) || c == '_')){
			return false; // must start with letter or underscore
		}

		std::cout << "*" << std::endl;
		curString.push_back(c);
		std::cout << c;
		++cur;

		while (cur != end){
			c = *cur;
			if (std::isalnum(c) || c == '_' || c == '?'){
				curString.push_back(c);
				std::cout << c;
				++cur;
			}
			else {
				break;
			}
		}
		std::cout << std::endl;
		std::cout << curString << std::endl << "****" << std::endl;

		if (isSpace(*cur)){
			skipWhitespace();
		}

		if (*cur == '['){ // scope decl
			retval.tokenType = SCOPEREF;
			retval.value.stringIndex = stringTable->pushString(curString);
			return true;
		}

		// check keywords
		for (size_t i = 0; i < _nKeywords; i++){
			if (curString == std::string(std::get<0>(_keywordTokens[i]))){
				retval.tokenType = std::get<1>(_keywordTokens[i]);
				return true;
			}
		}

		retval.tokenType = ID;
		retval.value.stringIndex = stringTable->pushString(curString);
		return true;
	}

public:
	Lexer(ITER const& start, ITER const& end, StringTable *stringTable) :
			cur(start),
			end(end),
			stringTable(stringTable)
	{}


	bool next(PRSLToken & retval) {
		skipWhitespace();

		if (cur == end){
			return false;
		}

		bool gotToken =
				nextSymbol(retval) || nextNumber(retval) || nextID(retval);

		if (gotToken){
			retval.lineNumber = currentLine;
		}

		return gotToken;
	}
};

}

#endif //PARASOL_LEXER_H
