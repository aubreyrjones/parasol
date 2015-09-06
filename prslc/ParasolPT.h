//
// Created by ARJ on 9/5/15.
//

#include <string>
#include <list>

#ifndef PARASOL_PARASOLPT_H
#define PARASOL_PARASOLPT_H

namespace prsl { namespace ast {

class FunctionDef;
class PipelineDef;

class FunctionDefList : std::list<FunctionDef> {
public:
	using std::list<FunctionDef>::list;
};

class PipelineDefList : std::list<PipelineDef> {
public:
	using std::list<PipelineDef>::list;
};


class Node {
protected:
public:
	virtual ~Node() {}
};


class Module : public Node {
protected:
	std::string moduleName {""}; /// name of this module, defaults to ""
	PipelineDefList memberPipelines; /// the pipeline members of this module.
	FunctionDefList memberFunctions; /// the functions that are global members of this module
};


}} // namespace

#endif //PARASOL_PARASOLPT_H
