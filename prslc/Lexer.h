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
	char c;
	ITER end;

	size_t currentLine = 1;

	std::string curString;
	StringTable *stringTable;


	void advance() {
		++cur;
		c = *cur;
	}

	void skipWhitespace() {
		while (cur != end) {
			switch (c) {
			case '\n':
				++currentLine;
			case ' ':
			case '\r':
			case '\t':
				advance();
				continue;
			default:
				return;
			}
		}
	}

	bool nextSymbol(PRSLToken & retval) {
		// check digraphs
		for (size_t i = 0; i < _nDigraphs; i++){
			if (c == std::get<0>(_digraphTokens[i])){
				advance();

				if (c == std::get<2>(_digraphTokens[i])){
					advance();
					retval.tokenType = std::get<3>(_digraphTokens[i]);
				}
				else {
					retval.tokenType = std::get<1>(_digraphTokens[i]);
					// already consumed the token above
				}

				return true;
			}
		}

		// check monographs (is that it? or unigraph? who cares?)
		for (size_t i = 0; i < _nSymbols; i++){
			if (c == std::get<0>(_symbolTokens[i])){
				retval.tokenType = std::get<1>(_symbolTokens[i]);
				advance();
				return true;
			}
		}

		return false;
	}

	bool nextNumber(PRSLToken & retval) {
		curString.clear();

		bool isNegative = false;
		bool isFloat = false;

		if (c == '-'){
			isNegative = true;
			curString.push_back(c);
			advance();
		}

		while (cur != end) {
			if (c == '.' && !isFloat) { // only one . allowed.
				isFloat = true;
				curString.push_back(c);
			}
			else if (std::isdigit(c)) {
				curString.push_back(c);
			}
			else {
				break;
			}

			advance();
		}

		std::istringstream sstream(curString);

		if (curString.size() > 0){
			if (isFloat){
				if (!(sstream >> retval.value.floatValue)){
					throw ParseError("Failed to lex float literal.");
				}
				retval.tokenType = FLOAT_LIT;
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

		if (!(std::isalpha(c) || c == '_')){
			return false; // must start with letter or underscore
		}

		curString.push_back(c);
		advance();

		while (cur != end){
			c = c;
			if (std::isalnum(c) || c == '_' || c == '?'){
				curString.push_back(c);
				advance();
			}
			else {
				break;
			}
		}

		if (isSpace(c)){
			skipWhitespace();
		}

		if (c == '['){ // scope decl
			retval.tokenType = SCOPEREF;
			retval.value.stringIndex = stringTable->pushString(curString);
			advance();
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
	{
		c = *cur;
	}


	bool next(PRSLToken & retval) {
		skipWhitespace();

		bool gotToken = false;
		if (cur == end){
			retval.tokenType = 0;
			retval.lineNumber = currentLine;
			gotToken = true;
		}
		else {
			gotToken = nextSymbol(retval) || nextNumber(retval) || nextID(retval);
		}

		if (gotToken){
			retval.lineNumber = currentLine;
		}

		return gotToken;
	}

	bool atEnd() { return cur == end; }
};

}

#endif //PARASOL_LEXER_H
