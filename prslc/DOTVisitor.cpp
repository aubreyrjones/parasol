//
// Created by ARJ on 9/6/15.
//

#include "DOTVisitor.h"
#include <iostream>
#include "Lexer.h"
#include "ParasolPT.h"

namespace prsl {

using namespace ast;

DOTVisitor::DOTVisitor(std::string const &filename) : out(filename) {
	out << "digraph \"AST\" { \n";
	out << "node [shape=record, style=filled];\n";
}

size_t DOTVisitor::dotify(Node *root) {
	if (!root) return nodeIdx;

	size_t thisIdx = nodeIdx++;

	NodeType nType = root->type();

	out << "node [label=\"";

	switch (nType){
	case '_int' :
		out << static_cast<Integer*>(root)->value;
		break;
	case '_flt':
		out << static_cast<Float*>(root)->value;
		break;
	case 'idnt':
		out << static_cast<Ident*>(root)->value;
		break;
	case '_mod':
		out << "mod:" << static_cast<Module*>(root)->name->value;
		break;
	case 'pipe':
		out << "pipe:" << static_cast<Pipeline*>(root)->name->value;
		break;
	case 'fndf':
		out << "def:" << static_cast<FunctionDef*>(root)->name->toString();
		break;
	case 'bnop':
		out << lookupToken(static_cast<BinaryOp*>(root)->operatorToken);
		break;
	case 'unop':
		out << lookupToken(static_cast<UnaryOp*>(root)->operatorToken);
		break;
	case 'vdcl':
		out << static_cast<VarDecl*>(root)->toString();
		break;
	case 'fncl':
		out << "call:" << static_cast<FunctionCall*>(root)->functionName->value;
		break;
	default:
		printNodeType(root->type());
		break;
	}
	out << "\"";
	out << "] " << thisIdx;
	out << ";\n";

	if (nType == '_mod'){
		auto mod = static_cast<Module*>(root);
		for (Node* n : *mod->globalDecls){
			dotAndLink(thisIdx, n);
		}
	}
	else if (nType == 'pipe'){
		auto pipe = static_cast<Pipeline*>(root);
		for (Node* n : *pipe->contents){
			dotAndLink(thisIdx, n);
		}
	}
	else if (nType == 'bnop'){
		auto op = static_cast<BinaryOp*>(root);
		dotAndLink(thisIdx, op->left);
		dotAndLink(thisIdx, op->right);
	}
	else if (nType == 'unop'){
		auto op = static_cast<UnaryOp*>(root);
		dotAndLink(thisIdx, op->argument);
	}
	else if (nType == 'fndf'){
		auto fn = static_cast<FunctionDef*>(root);

		if (fn->parameters && fn->parameters->size() > 0) {
			size_t pIdx = intermediate(thisIdx, "params");
			for (Node *n : *fn->parameters) {
				dotAndLink(pIdx, n);
			}
		}

		dotAndLink(thisIdx, fn->body);
	}
	else if (nType == 'fncl'){
		auto fn = static_cast<FunctionCall*>(root);
		for (Node *n: *fn->arguments){
			dotAndLink(thisIdx, n);
		}
	}

	return thisIdx;
}

void DOTVisitor::printNodeType(NodeType type) {
	char s[5];
	char *t = reinterpret_cast<char*>(&type);
	s[0] = t[3];
	s[1] = t[2];
	s[2] = t[1];
	s[3] = t[0];
	s[4] = 0;

	out << s;
}

DOTVisitor::~DOTVisitor() {
	out << "}\n";
}

size_t DOTVisitor::intermediate(size_t parent, std::string const &name) {
	size_t thisIdx = nodeIdx++;
	out << "node [label=" << name << "] " << thisIdx << ";\n";
	out << parent << " -> " << thisIdx << ";\n";

	return thisIdx;
}
}