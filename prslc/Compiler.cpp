//
// Created by ARJ on 9/15/15.
//

#include "Compiler.h"
#include "Visitor.h"

namespace prsl {

using namespace ast;

prsl::LinkUnit::LinkUnit() {

}

void globalDeclPass(Module *mod) {
	for (Node *n : *mod->globalDecls) {
		NodeType ntype = n->type();
		if (ntype == 'pipe') {
			auto pipe = static_cast<Pipeline *>(n);
			mod->pipelines.emplace(pipe->name->value, pipe);
		}
		else if (ntype == 'fndf') {
			auto fndf = static_cast<FunctionDef *>(n);
			mod->pipelines.emplace(fndf->name->varName->value, fndf);
		}
	}
}

void LinkUnit::doModulePasses(Module *mod) {
	globalDeclPass(mod);
}

void LinkUnit::link() {
	linkIncludes();
}

struct IncludeVisitor {
	LinkUnit *l = nullptr;
	Pipeline *curPipe = nullptr;

	IncludeVisitor(LinkUnit *l) : l(l) {}

	void operator()(IncludeDecl *id) {
		std::cout << "yay" << std::endl;
		id->targetRef = l->findPipeline(id->includedPipeline->value);
		curPipe->includes.push_back(id);
	}

	void operator()(Pipeline *p) {
		curPipe = p;
	}

	void operator()(Node *n) {}
};

void LinkUnit::linkIncludes() {
	for (Module *m : modules) {
		IncludeVisitor iv(this);
		applyTypedVisitor(iv, m);
	}
}

Pipeline* LinkUnit::findPipeline(std::string const &pipeline) {
	for (Module *m : modules) {
		auto it = m->pipelines.find(pipeline);
		if (it != m->pipelines.end()){
			return static_cast<Pipeline*>(it->second);
		}
	}

	return nullptr;
}
}