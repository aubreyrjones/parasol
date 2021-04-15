//
// Created by ARJ on 9/16/15.
//

#include <string>
#include <unordered_map>
#include <vector>

#ifndef PARASOL_TYPES_H
#define PARASOL_TYPES_H

namespace prsl {

struct Type;

typedef std::unordered_map<std::string, Type*> TypeSpace;

struct Type {
	std::string name;

	Type(std::string const& n) : name(n) {}
};





void addBuiltinTypes(TypeSpace& table);

}

#endif //PARASOL_TYPES_H
