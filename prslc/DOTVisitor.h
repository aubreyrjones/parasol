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
	std::ofstream out;
	void printNodeType(ast::NodeType type);

	template <class NTYPE>
	void dotAndLink(size_t thisIdx, NTYPE *n, char const* edgeLabel = nullptr) {
		if (!n) return;
		size_t i = dotify(n);

		out << thisIdx << " -> " << i;

		if (edgeLabel) {
			out << " " << edgeLabel;
		}
		out << ";\n";
	}

public:

	DOTVisitor(std::string const &filename);
	~DOTVisitor();

	size_t dotify(prsl::ast::Node *root);
};
}

#endif //PARASOL_DOTVISITOR_H
