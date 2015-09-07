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
#include <tinyformat.h>

#ifndef PARASOL_LEXER_H
#define PARASOL_LEXER_H

namespace prsl {

using std::get;

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

std::string lookupToken(int token);

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

	void skipToEOL(){
		while (c != '\n') advance();
	}

	void skipWhitespace() {
		while (cur != end) {
			switch (c) {
			case ';':
				skipToEOL();
				continue;
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
			if (c == get<0>(_digraphTokens[i])){
				DigraphToken &firstTok = _digraphTokens[i];

				char ch = c;
				advance();

				for (; i < _nDigraphs; i++){ // using the same i!
					if (c == get<2>(_digraphTokens[i])){
						retval.tokenType = get<3>(_digraphTokens[i]);
						advance();
						return true;
					}
				}

				retval.tokenType = get<1>(firstTok);
				return true;
			}
		}

		// check monographs (is that it? or unigraph? who cares?)
		for (size_t i = 0; i < _nSymbols; i++){
			if (c == get<0>(_symbolTokens[i])){
				retval.tokenType = get<1>(_symbolTokens[i]);
				advance();
				return true;
			}
		}

		return false;
	}

	bool isIDChar(char ch) {
		return std::isalnum(ch) || ch == '_' || ch == '?';
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

		if (curString.size() > 0){
			std::istringstream sstream(curString);

			if (isIDChar(c)){
				// basically, this guards against "22.rgb" being lexed as 22.0(floating) and then rgb.
				// but pushing a swizzle is also ambiguous. Best just to ask them to parenthesize or space.
				throw ParseError(tfm::format("You seem to have some identifier in your numeric literal on line %d. Do you need a paren or a space?", currentLine));
			}

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
			if (isIDChar(c)){
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

		auto pushAndAdvance = [this, &retval]() -> void {
			retval.value.stringIndex = stringTable->pushString(curString);
			advance();
		};

		// check keywords
		for (size_t i = 0; i < _nKeywords; i++){
			if (curString == std::string(get<0>(_keywordTokens[i]))){
				retval.tokenType = get<1>(_keywordTokens[i]);
				return true;
			}
		}

		if (c == '['){ // scope decl
			retval.tokenType = SCOPEREF;
			pushAndAdvance();
			return true;
		}
		if (c == '('){ // function call
			retval.tokenType = FNCALL;
			pushAndAdvance();
			return true;
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
