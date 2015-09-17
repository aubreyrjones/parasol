//
// Created by ARJ on 9/5/15.
//

#include "ParasolAST.h"
#include <tinyformat.h>

namespace prsl {namespace ast {

std::string formatParameterList(ParameterList *params) {
	if (!params || params->empty()) return std::string("");

	std::stringstream sstream;

	auto frmt = [&sstream](VarDecl *param) -> void {
		sstream << param->toString();
	};

	for (size_t i = 0; i < params->size() - 1; i++) {
		frmt((*params)[i]);
		sstream << ", ";
	}

	frmt((*params)[params->size() - 1]); // format the last one with no comma


	return std::string(sstream.str());
}

FunctionDef *Module::getGlobalFunction(std::string const &name) {

	SymbolTable::iterator it = functions.find(name);
	if (it == functions.end()){
		return nullptr;
	}

	return static_cast<FunctionDef*>(it->second);
}

FunctionDef *Module::getPipelineFunction(std::string const &pipeline, std::string const &function) {

	Pipeline *pipe = getPipeline(pipeline);
	if (!pipe){
		return nullptr;
	}

	return pipe->getFunction(function);
}

Pipeline *Module::getPipeline(std::string const &pipeline) {
	SymbolTable::iterator it = pipelines.find(pipeline);

	if (it == pipelines.end()){
		return nullptr;
	}

	return static_cast<Pipeline*>(it->second);
}

FunctionDef *Pipeline::getFunction(std::string const &function) {
	SymbolTable::iterator it = functions.find(function);

	if (it == functions.end()){
		return nullptr;
	}

	return static_cast<FunctionDef*>(it->second);
}

VarDecl *Pipeline::getVariable(std::string const &var) {
	SymbolTable::iterator it = variables.find(var);
	if (it == variables.end()){
		return nullptr;
	}

	return static_cast<VarDecl*>(it->second);
}

VarDecl *Module::getVariable(std::string const &pipeline, std::string const &var) {
	Pipeline* pipe = getPipeline(pipeline);
	if (!pipe){
		return nullptr;
	}

	return pipe->getVariable(var);
}

FunctionDef *Pipeline::resolveFunction(std::string const &function) {
	FunctionDef* retval = getFunction(function);

	if (retval) return retval;

	for (auto incl : includes) {
		retval = incl->targetRef->resolveFunction(function);
		if (retval) return retval;
	}

	return nullptr;
}

VarDecl *Pipeline::resolveVariable(std::string const &var) {
	VarDecl* retval = getVariable(var);

	if (retval) return retval;

	for (auto incl : includes) {
		retval = incl->targetRef->resolveVariable(var);
		if (retval) return retval;
	}

	return nullptr;
}
}}