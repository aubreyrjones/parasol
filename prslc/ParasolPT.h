//
// Created by ARJ on 9/5/15.
//

#include <string>
#include <vector>
#include <sstream>
#include <stdint.h>

#ifndef PARASOL_PARASOLPT_H
#define PARASOL_PARASOLPT_H

namespace prsl { namespace ast {

class FunctionDef;
class PipelineDef;
class Expression;
class VarDecl;
class Node;

typedef std::vector<VarDecl*> ParameterList;
typedef std::vector<Expression*> ArgumentList;
typedef std::vector<Node*> NodeList;

typedef uint32_t NodeType;

struct Node {
	virtual ~Node() {}

	virtual NodeType type() = 0;

	virtual bool isExpr() { return false; }
};

struct Expression : public Node {
	virtual ~Expression() {}

	bool isExpr() override { return true; }
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
			out << " " << varType->value;
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

struct Pipeline : public Node {
	Ident *name = nullptr;
	NodeList *contents = nullptr;

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
};

struct Module : public Node {
	Ident *name = nullptr;
	NodeList *globalDecls = nullptr;

	Module(std::string const& name, NodeList *globalDecls) :
			name(new Ident(name)),
			globalDecls(globalDecls)
	{}

	virtual NodeType type() { return '_mod'; }
};


std::string formatParameterList(ParameterList *params);

}} // namespace

#endif //PARASOL_PARASOLPT_H
