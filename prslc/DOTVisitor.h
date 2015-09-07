//
// Created by ARJ on 9/6/15.
//

#include <fstream>
#include "ParasolPT.h"

#ifndef PARASOL_DOTVISITOR_H
#define PARASOL_DOTVISITOR_H

namespace prsl {

class DOTVisitor {
protected:
	std::ofstream out;

public:

	DOTVisitor(std::string const &filename);

	void dotify(prsl::ast::Node *root);
};
}

#endif //PARASOL_DOTVISITOR_H
