//
// Created by ARJ on 9/5/15.
//

#include <cstdint>
#include <cstddef>

#ifndef PARASOL_TOKEN_H
#define PARASOL_TOKEN_H

// not in a namespace for compatibility with Lemon.

namespace prsl {
class StringTable; // forward declaration of string table.
}

struct PRSLToken {
    int tokenType;    /// token type as defined in prsl_grammar.h
	prsl::StringTable *stringTable;
	size_t lineNumber;

    // values are discriminated based on tokenType.
    union {
        int64_t intValue; /// integer literal value (64-bit holds entire i32 and u32 ranges supported by parasol).
        float floatValue; /// floating point value
        size_t stringIndex; /// for ids, the index into the lexer's string table.
    } value;
};

#endif //PARASOL_TOKEN_H
