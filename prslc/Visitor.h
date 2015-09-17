//
// Created by ARJ on 9/15/15.
//

#include "ParasolAST.h"

#ifndef PARASOL_VISITOR_H
#define PARASOL_VISITOR_H

namespace prsl {

using namespace ast;

template <class V>
void applyVisitor(V & v, ast::Node *root){
	if (!root) return;

	NodeType nType = root->type();

	if (nType == '_mod'){
		auto mod = static_cast<Module*>(root);

		if (mod->globalDecls) {
			for (Node *n : *mod->globalDecls) {
				if (n) v(n);
			}
		}
	}
	else if (nType == 'pipe'){
		auto pipe = static_cast<Pipeline*>(root);
		for (Node* n : *pipe->contents){
			v(n);
		}
	}
	else if (nType == 'bnop'){
		auto op = static_cast<BinaryOp*>(root);
		v(op->left);
		v(op->right);
	}
	else if (nType == 'unop'){
		auto op = static_cast<UnaryOp*>(root);
		v(op->argument);
	}
	else if (nType == 'fndf'){
		auto fn = static_cast<FunctionDef*>(root);
		v(fn->body);
	}
	else if (nType == 'fncl'){
		auto fn = static_cast<FunctionCall*>(root);
		for (Node *n: *fn->arguments){
			v(n);
		}
	}
	else if (nType == 'lmbd'){
		auto lambda = static_cast<Lambda*>(root);
		v(lambda->body);
	}
	else if (nType == 'csst'){
		auto cases = static_cast<CaseSet*>(root);

		for (Node *n : *(cases->cases)){
			v(n);
		}
	}
	else if (nType == 'case'){
		auto case_ = static_cast<Case*>(root);

		v(case_->condition);
		v(case_->result);
	}
	else if (nType == '_let'){
		auto let = static_cast<Let*>(root);

		for (Node *n : *let->assignments){
			v(n);
		}

		v(let->body);
	}
	else if (nType == 'strt'){
		auto struct_ = static_cast<StructDef*>(root);
		for (Node *n : *struct_->members){
			v(n);
		}
	}
}

template <class V>
void applyTypedVisitor(V & v, Node *root) {
	if (!root) return;

	NodeType nType = root->type();

	if (nType == '_mod'){
		auto mod = static_cast<Module*>(root);

		v(mod);

		if (mod->globalDecls) {
			for (Node *n : *mod->globalDecls) {
				if (n) applyTypedVisitor(v, n);
			}
		}
	}
	else if (nType == 'pipe'){
		auto pipe = static_cast<Pipeline*>(root);

		v(pipe);

		for (Node* n : *pipe->contents){
			if (n) applyTypedVisitor(v, n);
		}
	}
	else if (nType == 'bnop'){
		auto op = static_cast<BinaryOp*>(root);

		v(op);

		applyTypedVisitor(v, op->left);
		applyTypedVisitor(v, op->right);
	}
	else if (nType == 'unop'){
		auto op = static_cast<UnaryOp*>(root);

		v(op);

		applyTypedVisitor(v, op->argument);
	}
	else if (nType == 'fndf'){
		auto fn = static_cast<FunctionDef*>(root);

		v(fn);

		applyTypedVisitor(v, fn->body);
	}
	else if (nType == 'fncl'){
		auto fn = static_cast<FunctionCall*>(root);

		v(fn);

		for (Node *n: *fn->arguments){
			if (n) applyTypedVisitor(v, n);
		}
	}
	else if (nType == 'lmbd'){
		auto lambda = static_cast<Lambda*>(root);

		v(lambda);

		applyTypedVisitor(v, lambda->body);
	}
	else if (nType == 'csst'){
		auto cases = static_cast<CaseSet*>(root);

		v(cases);

		for (Node *n : *(cases->cases)){
			if (n) applyTypedVisitor(v, n);
		}
	}
	else if (nType == 'case'){
		auto case_ = static_cast<Case*>(root);

		v(case_);

		applyTypedVisitor(v, case_->condition);
		applyTypedVisitor(v, case_->result);
	}
	else if (nType == '_let'){
		auto let = static_cast<Let*>(root);

		v(let);

		for (Node *n : *let->assignments){
			if (n) applyTypedVisitor(v, n);
		}

		applyTypedVisitor(v, let->body);
	}
	else if (nType == 'strt'){
		auto struct_ = static_cast<StructDef*>(root);

		v(struct_);

		for (Node *n : *struct_->members){
			if (n) applyTypedVisitor(v, n);
		}
	}
	else if (nType == 'incl') {
		auto incl = static_cast<IncludeDecl*>(root);

		v(incl);
	}
}

}

#endif //PARASOL_VISITOR_H
