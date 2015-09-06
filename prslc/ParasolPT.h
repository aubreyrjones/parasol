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
class Expression;
class VarDecl;

typedef std::list<FunctionDef*> FunctionDefList;
typedef std::list<PipelineDef*> PipelineDefList;
typedef std::list<VarDecl*> ParameterList;
typedef std::list<Expression*> ArgumentList;


struct Node {
	virtual ~Node() {}
};

struct Expression : public Node {
	virtual ~Expression() {}
};

struct Integer : public Expression {
	int64_t value;

	Integer(int64_t value) : value(value) {}
};

struct Float : public Expression {
	float value;

	Float(float value) : value(value) {}
};

struct Ident : public Expression {
	std::string value;

	Ident(std::string const& value) : value(value) {}
};

struct VarDecl : public Expression {
	Ident *varName = nullptr;
	Ident *varType = nullptr;
	Integer *varIndex = nullptr;
	Ident *scope = nullptr;

	VarDecl(Ident *varName, Ident *varType, Integer *varIndex) :
			varName(varName),
			varType(varType),
			varIndex(varIndex)
	{}

	VarDecl(Ident *varName, Ident *varType, Integer *varIndex, Ident *scope) :
			varName(varName),
			varType(varType),
			varIndex(varIndex),
			scope(scope)
	{}


	VarDecl(VarDecl *o, Ident *scope) : // steal from o.
			varName(o->varName),
			varType(o->varType),
			varIndex(o->varIndex),
			scope(scope)
	{
		o->varName = nullptr;
		o->varType = nullptr;
		o->varIndex = nullptr;
		o->scope = nullptr;
	}


	virtual ~VarDecl() {
		if (varName) delete varName;
		if (varType) delete varType;
		if (varIndex) delete varIndex;
		if (scope) delete scope;
	}
};

struct FunctionCall : public Expression {
	Ident *functionName = nullptr;
	ArgumentList *arguments = nullptr;

	FunctionCall(Ident *name, ArgumentList *args) :
			functionName(name),
			arguments(args)
	{}

	virtual ~FunctionCall() {
		if (functionName) {
			delete functionName;
		}

		if (arguments) {
			for (auto p : *arguments){
				delete p;
			}
			delete arguments;
		}
	}
};

struct FunctionDef : public Node {
	VarDecl *name = nullptr;
	ParameterList *parameters = nullptr;
	Expression *body = nullptr;

	FunctionDef(VarDecl *name, ParameterList *params, Expression *body) :
			name(name),
			parameters(params),
			body(body)
	{}

	virtual ~FunctionDef() {
		if (name) delete name;
		if (parameters) delete parameters;
		if (body) delete body;
	}
};

//class Module : public Node {
//protected:
//	std::string moduleName {""}; /// name of this module, defaults to ""
//	PipelineDefList memberPipelines; /// the pipeline members of this module.
//	FunctionDefList memberFunctions; /// the functions that are global members of this module
//};


}} // namespace

#endif //PARASOL_PARASOLPT_H
