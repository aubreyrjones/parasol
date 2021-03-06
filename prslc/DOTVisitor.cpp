//
// Created by ARJ on 9/6/15.
//

#include "DOTVisitor.h"
#include <iostream>
#include "Lexer.h"
#include "ParasolAST.h"

namespace prsl {

using namespace ast;

DOTVisitor::DOTVisitor(std::string const &filename) : out(filename) {
	out << "digraph \"AST\" { \n";
	out << "node [shape=record, style=filled];\n";
}


void dotSanitize(std::string &str) {
	auto sani = [&str](char c) -> void {
		size_t index = 0;
		while (true) {
			index = str.find(c, index);
			if (index == std::string::npos) break;
			str.replace(str.begin() + index, str.begin() + index + 1, "\\");
			index++;
			str.insert(str.begin() + index++, c);
		}
	};

	sani('<');
	sani('>');
	sani('|');
}

size_t DOTVisitor::dotify(Node *root) {
	if (!root) return 0;

	size_t thisIdx = root->nodeID;

	NodeType nType = root->type();

	out << "node [shape=record, label=\"<f0> ";

	if (nType == '_int' ) {
		out << static_cast<Integer *>(root)->value;
	}
	else if (nType == '_flt') {
		out << static_cast<Float *>(root)->value;
	}
	else if (nType == 'idnt') {
		out << static_cast<Ident *>(root)->value;
	}
	else if (nType == '_mod') {
		out << "mod:" << static_cast<Module *>(root)->name->value;
	}
	else if (nType == 'pipe') {
		out << "=" << static_cast<Pipeline *>(root)->name->value << "=" ;
	}
	else if (nType == 'fndf') {
		std::string formattedParams = formatParameterList(static_cast<FunctionDef *>(root)->parameters);
		out << "&Delta;" << static_cast<FunctionDef *>(root)->name->toString() << " " << formattedParams;
	}
	else if (nType == 'bnop') {
		std::string tokenString = lookupToken(static_cast<BinaryOp *>(root)->operatorToken);
		dotSanitize(tokenString);
		out << " | <f1> " << tokenString << " | <f2> ";
	}
	else if (nType == 'unop') {
		out << lookupToken(static_cast<UnaryOp *>(root)->operatorToken);
	}
	else if (nType == 'vdcl') {
		out << static_cast<VarDecl *>(root)->toString();
	}
	else if (nType == 'fncl') {
		auto fncl = static_cast<FunctionCall *>(root);
		out << fncl->functionName->value;
		for (int i = 0; i < fncl->arguments->size(); i++){
			int idx = i + 1;
			out << "| <f" << idx << "> " << i;
		}
	}
	else if (nType == 'lmbd') {
		std::string formattedParams = formatParameterList(static_cast<Lambda *>(root)->parameters);
		out << "&lambda;" << formattedParams;
	}
	else if (nType == 'csst') {
		out << "&Psi;";
	}
	else if (nType == 'case') {
		out << "cond | <f1> result";
	}
	else if (nType == 'strt') {
		out << "\\{" << static_cast<StructDef *>(root)->name->value << "\\}";
	}
	else if (nType == 'incl') {
		auto id = static_cast<IncludeDecl*>(root);
		out << id->toString();
	}
	else {
		printNodeType(root->type());
	}
	//out << "|<f1>: " << root->line;
	out << "\"";
	out << "] " << thisIdx;
	out << ";\n";

	if (nType == '_mod'){
		auto mod = static_cast<Module*>(root);

		if (mod->globalDecls) {
			for (Node *n : *mod->globalDecls) {
				dotAndLink(thisIdx, n);
			}
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

		dotify(op->left);
		dotify(op->right);

		out << '"' << thisIdx << "\":<f0> -> " << op->left->nodeID << ";\n";
		out << '"' << thisIdx << "\":<f2> -> " << op->right->nodeID << ";\n";
	}
	else if (nType == 'unop'){
		auto op = static_cast<UnaryOp*>(root);
		dotAndLink(thisIdx, op->argument);
	}
	else if (nType == 'fndf'){
		auto fn = static_cast<FunctionDef*>(root);
		dotAndLink(thisIdx, fn->body);
	}
	else if (nType == 'fncl'){
		auto fn = static_cast<FunctionCall*>(root);
		int i = 1;
		for (Node *n: *fn->arguments){
			dotify(n);

			out << '"' << thisIdx << "\":<f" << i << "> -> " << n->nodeID << ";\n";

			++i;
		}
	}
	else if (nType == 'lmbd'){
		auto lambda = static_cast<Lambda*>(root);
		dotAndLink(thisIdx, lambda->body);
	}
	else if (nType == 'csst'){
		auto cases = static_cast<PsiExpr *>(root);

		for (Node *n : *(cases->cases)){
			dotAndLink(thisIdx, n);
		}
	}
	else if (nType == 'case'){
		auto case_ = static_cast<Case*>(root);

		dotify(case_->condition);
		dotify(case_->result);

		out << '"' << thisIdx << "\":<f0> -> " << case_->condition->nodeID << ";\n";
		out << '"' << thisIdx << "\":<f1> -> " << case_->result->nodeID << ";\n";


		//dotAndLink(thisIdx, case_->condition);
		//dotAndLink(thisIdx, case_->result);
	}
	else if (nType == '_let'){
		auto let = static_cast<Let*>(root);

		for (Node *n : *let->assignments){
			dotAndLink(thisIdx, n);
		}

		dotAndLink(thisIdx, let->body, "[label=\"in\"]");
	}
	else if (nType == 'strt'){
		auto struct_ = static_cast<StructDef*>(root);
		for (Node *n : *struct_->members){
			dotAndLink(thisIdx, n);
		}
	}
	else if (nType == 'incl') {
		auto id = static_cast<IncludeDecl*>(root);
		if (id->targetRef) {
			out << thisIdx << " -> " << id->targetRef->nodeID << "[style=dotted];\n";
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

}