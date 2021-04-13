//
// Created by ARJ on 9/5/15.
//

#include "ParasolAST.h"
#include <tinyformat.h>

namespace prsl {namespace ast {

std::string formatParameterList(shared_ptr<ParameterList> const& params) {
	if (!params || params->empty()) return std::string("");

	std::stringstream sstream;

	auto frmt = [&sstream](shared_ptr<VarDecl> const& param) -> void {
		sstream << param->toString();
	};

	for (size_t i = 0; i < params->size() - 1; i++) {
		frmt((*params)[i]);
		sstream << ", ";
	}

	frmt((*params)[params->size() - 1]); // format the last one with no comma


	return std::string(sstream.str());
}

shared_ptr<FunctionDef> Module::getGlobalFunction(std::string const &name) {

	SymbolTable::iterator it = functions.find(name);
	if (it == functions.end()){
		return nullptr;
	}

	return std::dynamic_pointer_cast<FunctionDef>(it->second);
}

shared_ptr<FunctionDef> Module::getPipelineFunction(std::string const &pipeline, std::string const &function) {

	auto pipe = getPipeline(pipeline);
	if (!pipe){
		return nullptr;
	}

	return pipe->getFunction(function);
}

shared_ptr<Pipeline> Module::getPipeline(std::string const &pipeline) {
	SymbolTable::iterator it = pipelines.find(pipeline);

	if (it == pipelines.end()){
		return nullptr;
	}

	return std::dynamic_pointer_cast<Pipeline>(it->second);
}

shared_ptr<FunctionDef> Pipeline::getFunction(std::string const &function) {
	SymbolTable::iterator it = functions.find(function);

	if (it == functions.end()){
		return nullptr;
	}

	return std::dynamic_pointer_cast<FunctionDef>(it->second);
}

shared_ptr<VarDecl> Pipeline::getVariable(std::string const &var) {
	SymbolTable::iterator it = variables.find(var);
	if (it == variables.end()){
		return nullptr;
	}

	return std::dynamic_pointer_cast<VarDecl>(it->second);
}

shared_ptr<VarDecl> Module::getVariable(std::string const &pipeline, std::string const &var) {
	auto pipe = getPipeline(pipeline);
	if (!pipe){
		return nullptr;
	}

	return pipe->getVariable(var);
}

shared_ptr<FunctionDef> Pipeline::resolveFunction(std::string const &function) {
	auto retval = getFunction(function);

	if (retval) return retval;

	for (auto incl : includes) {
		retval = incl->targetRef->resolveFunction(function);
		if (retval) return retval;
	}

	return nullptr;
}

shared_ptr<VarDecl> Pipeline::resolveVariable(std::string const &var) {
	auto retval = getVariable(var);

	if (retval) return retval;

	for (auto incl : includes) {
		retval = incl->targetRef->resolveVariable(var);
		if (retval) return retval;
	}

	return nullptr;
}
}}