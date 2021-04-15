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
	Node *parent = nullptr;

	virtual ~Node() {}

	virtual NodeType type() = 0;

	virtual bool isExpr() { return false; }

	virtual std::string toString() { return "node"; };
};

struct Expression : public Node {
	std::string type;

	Expression() : type() {}

	virtual ~Expression() {}

	bool isExpr() override { return true; }
	bool isTypeBound() const { return !type.empty(); }
};

struct BinaryOp : public Expression {
	int operatorToken; // as taken from prsl_grammar.h
	Expression *left;
	Expression *right;

	BinaryOp(int op, Expression *left, Expression *right) :
			operatorToken(op),
			left(left),
			right(right)
	{
		left->parent = this;
		right->parent = this;
	}

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
	{
		argument->parent = this;
	}

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

struct VarDecl : public Expression {
	Ident *varName = nullptr;
	TypeIdent *varType = nullptr;
	Integer *varIndex = nullptr;
	Ident *scope = nullptr;

	VarDecl(Ident *varName, TypeIdent *varType, Integer *varIndex) :
			varName(varName),
			varType(varType),
			varIndex(varIndex)
	{
		if (varName) varName->parent = this;
		if (varType) varType->parent = this;
		if (varIndex) varIndex->parent = this;
	}

	VarDecl(Ident *varName, TypeIdent *varType, Integer *varIndex, Ident *scope) :
			varName(varName),
			varType(varType),
			varIndex(varIndex),
			scope(scope)
	{
		if (varName) varName->parent = this;
		if (varType) varType->parent = this;
		if (varIndex) varIndex->parent = this;
		if (scope) scope->parent = this;
	}


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

		if (varName) varName->parent = this;
		if (varType) varType->parent = this;
		if (varIndex) varIndex->parent = this;
		if (scope) scope->parent = this;
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
	{
		functionName->parent = this;

		if (arguments) {
			for (Node *n : *arguments){
				n->parent = this;
			}
		}
	}

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
	{
		name->parent = this;
		body->parent = this;

		if (params) {
			for (Node *n : *params){
				n->parent = this;
			}
		}
	}

	virtual ~FunctionDef() {
		if (name) delete name;
		if (parameters) {
			for (auto p : *parameters) delete p;
			delete parameters;
		}
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
	{
		if (parameters)
			for (auto p : *parameters)
				p->parent = this;
	}

	virtual ~Lambda() {
		if (parameters) {
			for (auto p : *parameters) delete p;
			delete parameters;
		}
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
	{
		cond->parent = this;
		res->parent = this;
	}

	virtual ~Case() {
		if (condition) delete condition;
		if (result) delete result;
	}

	virtual NodeType type() { return 'case'; }
};

struct PsiExpr : public Expression {
	CaseList *cases = nullptr;

	PsiExpr(CaseList *cases) : cases(cases) {
		if (cases) for (auto p : *cases) p->parent = this;
	}

	virtual ~PsiExpr(){
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
	{
		for (auto p : *assignments) p->parent = this;
		body->parent = this;
	}

	virtual ~Let() {
		if (assignments) {
			for (auto p : *assignments) delete p;
			delete assignments;
		}
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
	{
		includedPipeline->parent = this;
		if (asName) asName->parent = this;
	}

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
	{
		name->parent = this;
		if (contents) for (auto p : *contents) p->parent = this;
	}

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
	{
		name->parent = this;
		if (members) for (auto p : *members) p->parent = this;
	}

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
	{
		this->name->parent = this;
		if (globalDecls) for (auto p : *globalDecls) p->parent = this;
	}

	virtual ~Module() {
		if (name) delete name;
		if (globalDecls) {
			for (auto p : *globalDecls) delete p;
			delete globalDecls;
		}
	}

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
