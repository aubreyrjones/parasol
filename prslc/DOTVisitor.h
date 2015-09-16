//
// Created by ARJ on 9/6/15.
//

#include <fstream>
#include "ParasolAST.h"

#ifndef PARASOL_DOTVISITOR_H
#define PARASOL_DOTVISITOR_H

namespace prsl {

class DOTVisitor {
protected:
	size_t nodeIdx = 0;
	std::ofstream out;
	void printNodeType(ast::NodeType type);

	template <class NTYPE>
	void dotAndLink(size_t thisIdx, NTYPE *n) {
		if (!n) return;
		size_t i = dotify(n);
		out << thisIdx << " -> " << i << ";\n";
	}

	size_t intermediate(size_t parent, std::string const& name);

public:

	DOTVisitor(std::string const &filename);
	~DOTVisitor();

	size_t dotify(prsl::ast::Node *root);
};
}

#endif //PARASOL_DOTVISITOR_H
