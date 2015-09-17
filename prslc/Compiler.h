//
// Created by ARJ on 9/15/15.
//

#ifndef PARASOL_COMPILER_H
#define PARASOL_COMPILER_H

#include "ParasolAST.h"
#include "Parser.h"

namespace prsl {

using namespace ast;

typedef std::vector<ast::Module*> ModuleList;


class CallLinker;


class LinkUnit {
protected:
	ModuleList modules;

	void linkIncludes();

	void doModulePasses(ast::Module *mod);  /// compiler passes performed on each module.

public:
	LinkUnit();

	/** After adding all the modules you want to, call this function to cross-link everything. */
	void link();

	/** Find the pipeline with the given name, or nullptr if none exists. */
	Pipeline* findPipeline(std::string const& pipeline);

	/** Get all modules in this link unit.*/
	ModuleList & getModules() { return modules; }

	/**
	 * Add a module to this link unit from source code.
	 * */
	template <class ITER>
	void addModule(ITER i, ITER end);
};


template <class ITER>
void LinkUnit::addModule(ITER i, ITER end) {
	ast::Module *mod = parseModule(i, end);

	doModulePasses(mod);

	modules.push_back(mod);
}

}

#endif //PARASOL_COMPILER_H
