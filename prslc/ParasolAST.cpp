//
// Created by ARJ on 9/5/15.
//

#include "ParasolAST.h"

namespace prsl {namespace ast {

std::string formatParameterList(ParameterList *params) {
	if (!params || params->empty()) return std::string("");

	std::stringstream sstream;

	auto frmt = [&sstream](VarDecl *param) -> void {
		sstream << param->toString();
	};

	for (size_t i = 0; i < params->size() - 1; i++) {
		frmt((*params)[i]);
		sstream << ", ";
	}

	frmt((*params)[params->size() - 1]); // format the last one with no comma


	return std::string(sstream.str());
}

}}