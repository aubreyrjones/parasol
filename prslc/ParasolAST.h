//
// Created by ARJ on 9/5/15.
//

#include <string>
#include <vector>
#include <sstream>
#include <stdint.h>
#include <unordered_map>
#include "Types.h"

#ifndef PARASOL_PARASOLPT_H
#define PARASOL_PARASOLPT_H

namespace prsl { namespace ast {

class FunctionDef;
class Expression;
class VarDecl;
class Node;
class Case;
class Pipeline;
class IncludeDecl;

typedef std::vector<VarDecl*> ParameterList;
typedef std::vector<Expression*> ArgumentList;
typedef std::vector<Node*> NodeList;
typedef std::vector<Case*> CaseList;

typedef std::unordered_map<std::string, Node*> SymbolTable;

typedef uint32_t NodeType;

struct Node {
	size_t line = 0;
	size_t nodeID = 0;

	virtual ~Node() {}

	virtual NodeType type() = 0;

	virtual bool isExpr() { return false; }

	virtual std::string toString() { return "node"; };
};

struct Expression : public Node {
	virtual ~Expression() {}

	bool isExpr() override { return true; }

	//Type prslType() {}
};

struct BinaryOp : public Expression {
	int operatorToken; // as taken from prsl_grammar.h
	Expression *left;
	Expression *right;

	BinaryOp(int op, Expression *left, Expression *right) :
			operatorToken(op),
			left(left),
			right(right)
	{}

	virtual ~BinaryOp() {
		if (left) delete left;
		if (right) delete right;
	}

	virtual NodeType type() { return 'bnop'; }
};

struct UnaryOp : public Expression {
	int operatorToken;
	Expression *argument;

	UnaryOp(int op, Expression *argument) :
			operatorToken(op),
			argument(argument)
	{}

	virtual ~UnaryOp() {
		if (argument) delete argument;
	}

	virtual NodeType type() { return 'unop'; }
};

struct Integer : public Expression {
	int64_t value;

	Integer(int64_t value) : value(value) {}

	virtual NodeType type() { return '_int'; }
};

struct Float : public Expression {
	float value;

	Float(float value) : value(value) {}

	virtual NodeType type() { return '_flt'; }
};

struct Ident : public Expression {
	std::string value;

	Ident(std::string const& value) : value(value) {}

	virtual NodeType type() { return 'idnt'; }

	virtual std::string toString() { return value; };
};

struct TypeIdent : public Ident {
	int64_t dimension = 1;

	TypeIdent(std::string const& value, int64_t dimension = 1) :
			Ident(value),
			dimension(dimension)
	{}

	virtual NodeType type() { return 'tpid'; }

	virtual std::string toString() {
		std::stringstream out;

		out << Ident::toString();
		if (dimension > 1) {
			out << "@" << dimension;
		}

		return out.str();
	};
};

struct IfExpr : public Expression {
	Expression *condition = nullptr;
	Expression *thenExpr = nullptr;
	Expression *elseExpr = nullptr;

	IfExpr(Expression *cond, Expression *then, Expression *elseExpr) :
			condition(cond),
			thenExpr(then),
			elseExpr(elseExpr)
	{}

	virtual ~IfExpr() {
		if (condition) delete condition;
		if (thenExpr) delete thenExpr;
		if (elseExpr) delete elseExpr;
	}

	virtual NodeType type() { return '_if_'; }
};

struct VarDecl : public Expression {
	Ident *varName = nullptr;
	TypeIdent *varType = nullptr;
	Integer *varIndex = nullptr;
	Ident *scope = nullptr;

	VarDecl(Ident *varName, TypeIdent *varType, Integer *varIndex) :
			varName(varName),
			varType(varType),
			varIndex(varIndex)
	{}

	VarDecl(Ident *varName, TypeIdent *varType, Integer *varIndex, Ident *scope) :
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

	virtual NodeType type() { return 'vdcl'; }

	std::string toString() {
		std::stringstream out;

		if (scope) {
			out << scope->value << "[";
		}

		if (varName) {
			out << varName->value;
		}

		if (varType || varIndex) {
			out << ":";
		}

		if (varType) {
			out << " " << varType->toString();
		}

		if (varIndex) {
			out << " " << varIndex->value;
		}

		if (scope) {
			out << "]";
		}

		return out.str();
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

	virtual NodeType type() { return 'fncl'; }
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

	virtual NodeType type() { return 'fndf'; }
};

struct Lambda : public Expression {
	ParameterList *parameters = nullptr;
	Expression *body = nullptr;

	Lambda(ParameterList *params, Expression *body) :
			parameters(params),
			body(body)
	{}

	virtual ~Lambda() {
		if (parameters) delete parameters;
		if (body) delete body;
	}

	virtual NodeType type() { return 'lmbd'; }
};

struct Case : public Node {
	Expression *condition = nullptr;
	Expression *result = nullptr;

	Case(Expression *cond, Expression *res) :
			condition(cond),
			result(res)
	{}

	virtual ~Case() {
		if (condition) delete condition;
		if (result) delete result;
	}

	virtual NodeType type() { return 'case'; }
};

struct CaseSet : public Expression {
	CaseList *cases = nullptr;

	CaseSet(CaseList *cases) : cases(cases) {}

	virtual ~CaseSet(){
		if (cases){
			for (Case *c : *cases){
				delete c;
			}
			delete cases;
		}
	}

	virtual NodeType type() { return 'csst'; }
};

struct Let : public Expression {
	NodeList *assignments = nullptr;
	Expression *body = nullptr;

	Let(NodeList *assignments, Expression *body) :
			assignments(assignments),
			body(body)
	{}

	virtual ~Let() {
		if (assignments) delete assignments;
		if (body) delete body;
	}

	virtual NodeType type() { return '_let'; }
};

struct IncludeDecl : public Node {
	Ident *includedPipeline = nullptr;
	Ident *asName = nullptr;
	Pipeline *targetRef = nullptr;

	IncludeDecl(Ident *toInclude, Ident *as) :
			includedPipeline(toInclude),
			asName(as)
	{}

	virtual ~IncludeDecl() {
		if (includedPipeline) delete includedPipeline;
		if (asName) delete asName;
	}

	virtual NodeType type() { return 'incl'; }

	std::string toString() {
		std::stringstream out;
		out << "include " << includedPipeline->value;
		if (asName) {
			out << " as " << asName->value;
		}

		return out.str();
	}
};

struct Pipeline : public Node {
	Ident *name = nullptr;
	NodeList *contents = nullptr;
	SymbolTable functions {};
	SymbolTable variables {};
	std::vector<IncludeDecl*> includes {};

	Pipeline(Ident *name, NodeList *contents) :
			name(name),
			contents(contents)
	{}

	virtual ~Pipeline() {
		if (name) delete name;
		if (contents) {
			for (auto p : *contents){
				delete p;
			}
			delete contents;
		}
	}

	virtual NodeType type() { return 'pipe'; }

	FunctionDef* resolveFunction(std::string const& function);
	VarDecl* resolveVariable(std::string const& var);

	FunctionDef* getFunction(std::string const& function);

	VarDecl* getVariable(std::string const& var);
};

struct StructDef : public Node {
	Ident *name = nullptr;
	NodeList *members = nullptr;

	StructDef(Ident *name, NodeList *members) :
			name(name),
			members(members)
	{}

	virtual ~StructDef() {
		if (name) delete name;
		if (members) delete members;
	}

	virtual NodeType type() { return 'strt'; }
};

struct Module : public Node {
	Ident *name = nullptr;
	NodeList *globalDecls = nullptr;
	SymbolTable functions;
	SymbolTable pipelines;

	Module(std::string const& name, NodeList *globalDecls) :
			name(new Ident(name)),
			globalDecls(globalDecls),
			functions(),
			pipelines()
	{}

	virtual NodeType type() { return '_mod'; }

	/** Get a pipeline, or nullptr if no such function exists in that pipeline. */
	Pipeline* getPipeline(std::string const& pipeline);

	/** Get a function defined at global scope, or nullptr if no such function exists at global scope. */
	FunctionDef* getGlobalFunction(std::string const& name);

	/** Get a function defined in a particular pipeline, or nullptr if no such function exists in that
	 * pipeline. */
	FunctionDef* getPipelineFunction(std::string const& pipeline, std::string const& function);

	/** Get a variable defined in a particular pipeline, or nullptr if no such variable exists in that
	 * pipeline. */
	VarDecl* getVariable(std::string const& pipeline, std::string const& var);
};


struct IntrinsicFunction : public FunctionDef {

	IntrinsicFunction(VarDecl *name, ParameterList *params) :
			FunctionDef(name, params, nullptr)
	{ }
};

std::string formatParameterList(ParameterList *params);

}} // namespace

#endif //PARASOL_PARASOLPT_H
