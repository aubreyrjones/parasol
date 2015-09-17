//
// Created by ARJ on 9/15/15.
//

#include "IDVisitor.h"
#include "Visitor.h"

namespace prsl {

using namespace ast;

void IDVisitor::operator()(ast::Node *root) {
	if (!root) return;

	root->nodeID = id++;

	applyVisitor(*this, root);
}

}