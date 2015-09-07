//
// Created by ARJ on 9/6/15.
//

#include "DOTVisitor.h"
namespace prsl {

DOTVisitor::DOTVisitor(std::string const &filename) : out(filename) {

}

void DOTVisitor::dotify(prsl::ast::Node *root) {
	switch (root->type()){
	case '_int':
		out << "blah";
		break;
	default:
		return;
	}
}

}