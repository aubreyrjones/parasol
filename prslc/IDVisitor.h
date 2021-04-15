//
// Created by ARJ on 9/15/15.
//

#ifndef PARASOL_IDVISITOR_H
#define PARASOL_IDVISITOR_H

#include "ParasolAST.h"

namespace prsl {

struct IDVisitor {
	size_t id = 1;
	void operator()(ast::Node* root);
};

}

#endif //PARASOL_IDVISITOR_H
