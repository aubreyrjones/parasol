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

class StringTable {
protected:
	std::vector<std::string> strings;

	typedef std::unordered_map<std::string, size_t> PrevMap;

	PrevMap previousLocations;

public:
	size_t pushString(std::string const& s);  // add a string to the table

	std::string const& getString(size_t index); // get a string
};

typedef std::tuple<char, int> SymbolToken; /// symbol -> token value mapping

extern SymbolToken _symbol_tokens[]; /// lookup table for symbol -> token mappings
extern size_t _nSymbols; /// number of symbols in _symbol_tokens.


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

	size_t currentLine = 0;

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
		for (size_t i = 0; i < _nSymbols; i++){
			if (*cur == std::get<0>(_symbol_tokens[i])){
				retval.tokenType = std::get<1>(_symbol_tokens[i]);
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

		char c = *cur;

		if (!(std::isalpha(c) || c == '_')){
			return false; // must start with letter or underscore
		}

		curString.push_back(c);
		++cur;

		while (cur != end){
			c = *cur;
			if (std::isalnum(c) || c == '_' || c == '?'){
				curString.push_back(c);
				++cur;
			}
			else {
				break;
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
