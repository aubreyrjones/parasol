//
// Created by ARJ on 9/5/15.
//

#include <string>
#include <vector>
#include <sstream>
#include <stdint.h>
#include <memory>
#include <unordered_map>
#include "Types.h"

#ifndef PARASOL_PARASOLPT_H
#define PARASOL_PARASOLPT_H

namespace prsl { namespace ast {
using std::shared_ptr;

template <class T, class...U>
shared_ptr<T> make_node(U...args) {
	auto retval = std::make_shared<T>(std::forward(args...));
	retval->initChildren();
	return retval;
}

class FunctionDef;
class Expression;
class VarDecl;
class Node;
class Case;
class Pipeline;
class IncludeDecl;

typedef std::vector<shared_ptr<VarDecl>> ParameterList;
typedef std::vector<shared_ptr<Expression>> ArgumentList;
typedef std::vector<shared_ptr<Node>> NodeList;
typedef std::vector<shared_ptr<Case>> CaseList;

typedef std::unordered_map<std::string, shared_ptr<Node>> SymbolTable;

typedef uint64_t NodeType;

struct Node : public std::enable_shared_from_this<Node> {
	size_t line = 0;
	size_t nodeID = 0;
	std::weak_ptr<Node> parent;

	virtual ~Node() {}

	virtual NodeType type() = 0;

	virtual bool isExpr() { return false; }

	virtual std::string toString() { return "node"; }

	virtual void initChildren() {}
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
	shared_ptr<Expression> left;
	shared_ptr<Expression> right;

	BinaryOp(int op, shared_ptr<Expression> const& left, shared_ptr<Expression> const& right) :
			operatorToken(op),
			left(left),
			right(right)
	{
	}

	void initChildren() {
		left->parent = shared_from_this();
		right->parent = shared_from_this();
	}

	virtual NodeType type() { return 'bnop'; }
};

struct UnaryOp : public Expression {
	int operatorToken;
	shared_ptr<Expression> argument;

	UnaryOp(int op, Expression *argument) :
			operatorToken(op),
			argument(argument)
	{
	}

	void initChildren() {
		argument->parent = shared_from_this();
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
	shared_ptr<Ident> varName = nullptr;
	shared_ptr<TypeIdent> varType = nullptr;
	shared_ptr<Integer> varIndex = nullptr;
	shared_ptr<Ident> scope = nullptr;

	VarDecl(shared_ptr<Ident> varName, shared_ptr<TypeIdent> varType, shared_ptr<Integer> varIndex) :
			varName(varName),
			varType(varType),
			varIndex(varIndex)
	{
	}

	void initChildren() {
		if (varName) varName->parent = shared_from_this();
		if (varType) varType->parent = shared_from_this();;
		if (varIndex) varIndex->parent = shared_from_this();
		if (scope) scope->parent = shared_from_this();
	}

	VarDecl(Ident *varName, TypeIdent *varType, Integer *varIndex, Ident *scope) :
			varName(varName),
			varType(varType),
			varIndex(varIndex),
			scope(scope)
	{
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
	shared_ptr<Ident> functionName = nullptr;
	shared_ptr<ArgumentList> arguments = nullptr;

	FunctionCall(shared_ptr<Ident> const& name, shared_ptr<ArgumentList> const& args) :
			functionName(name),
			arguments(args)
	{

	}

	void initChildren() {
		functionName->parent = shared_from_this();

		if (arguments) {
			for (auto & n : *arguments){
				n->parent = shared_from_this();
			}
		}
	}

	virtual NodeType type() { return 'fncl'; }
};

struct FunctionDef : public Node {
	shared_ptr<VarDecl> name = nullptr;
	shared_ptr<ParameterList> parameters = nullptr;
	shared_ptr<Expression> body = nullptr;

	FunctionDef(shared_ptr<VarDecl> const& name, shared_ptr<ParameterList> const& params, shared_ptr<Expression> const& body) :
			name(name),
			parameters(params),
			body(body)
	{
	}

	void initChildren() {
		name->parent = shared_from_this();
		body->parent = shared_from_this();

		if (parameters) {
			for (auto & n : *parameters){
				n->parent = shared_from_this();
			}
		}
	}

	virtual NodeType type() { return 'fndf'; }
};

struct Lambda : public Expression {
	shared_ptr<ParameterList> parameters = nullptr;
	shared_ptr<Expression> body = nullptr;

	Lambda(shared_ptr<ParameterList> const& params, shared_ptr<Expression> const& body) :
			parameters(params),
			body(body)
	{

	}

	void initChildren() {
		if (parameters)
			for (auto & p : *parameters)
				p->parent = shared_from_this();
	}

	virtual NodeType type() { return 'lmbd'; }
};

struct Case : public Node {
	shared_ptr<Expression> condition = nullptr;
	shared_ptr<Expression> result = nullptr;

	Case(shared_ptr<Expression> const& cond, shared_ptr<Expression> const& res) :
			condition(cond),
			result(res)
	{

	}

	void initChildren() {
		condition->parent = shared_from_this();
		result->parent = shared_from_this();
	}

	virtual NodeType type() { return 'case'; }
};

struct PsiExpr : public Expression {
	shared_ptr<CaseList> cases = nullptr;

	PsiExpr(shared_ptr<CaseList> const& cases) : cases(cases) {
		
	}

	void initChildren() override {
		if (cases) for (auto p : *cases) p->parent = shared_from_this();
	}

	virtual NodeType type() { return 'csst'; }
};

struct Let : public Expression {
	shared_ptr<NodeList> assignments = nullptr;
	shared_ptr<Expression> body = nullptr;

	Let(shared_ptr<NodeList> const& assignments, shared_ptr<Expression> const& body) :
			assignments(assignments),
			body(body)
	{
		
	}

	void initChildren() {
		for (auto & p : *assignments) p->parent = shared_from_this();
		body->parent = shared_from_this();
	}

	virtual NodeType type() { return '_let'; }
};

struct IncludeDecl : public Node {
	shared_ptr<Ident> includedPipeline = nullptr;
	shared_ptr<Ident> asName = nullptr;
	shared_ptr<Pipeline> targetRef = nullptr;

	IncludeDecl(shared_ptr<Ident> const& toInclude, shared_ptr<Ident> const& as) :
			includedPipeline(toInclude),
			asName(as)
	{
	}

	void initChildren() {
		includedPipeline->parent = shared_from_this();
		if (asName) asName->parent = shared_from_this();
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
	shared_ptr<Ident> name = nullptr;
	shared_ptr<NodeList> contents = nullptr;
	SymbolTable functions {};
	SymbolTable variables {};
	std::vector<shared_ptr<IncludeDecl>> includes {};

	Pipeline(Ident *name, NodeList *contents) :
			name(name),
			contents(contents)
	{

	}

	void initChildren() {
		name->parent = shared_from_this();
		if (contents) for (auto & p : *contents) p->parent = shared_from_this();
	}

	virtual NodeType type() { return 'pipe'; }

	shared_ptr<FunctionDef> resolveFunction(std::string const& function);
	shared_ptr<VarDecl> resolveVariable(std::string const& var);

	shared_ptr<FunctionDef> getFunction(std::string const& function);

	shared_ptr<VarDecl> getVariable(std::string const& var);
};

struct StructDef : public Node {
	shared_ptr<Ident> name = nullptr;
	shared_ptr<NodeList> members = nullptr;

	StructDef(shared_ptr<Ident> const& name, shared_ptr<NodeList> const& members) :
			name(name),
			members(members)
	{
		
	}

	void initChildren() override {
		name->parent = shared_from_this();
		if (members) for (auto & p : *members) p->parent = shared_from_this();
	}

	virtual NodeType type() { return 'strt'; }
};

struct Module : public Node {
	shared_ptr<Ident> name = nullptr;
	shared_ptr<NodeList> globalDecls = nullptr;
	SymbolTable functions;
	SymbolTable pipelines;

	Module(std::string const& name, shared_ptr<NodeList> globalDecls) :
			name(new Ident(name)),
			globalDecls(globalDecls),
			functions(),
			pipelines()
	{

	}

	void initChildren() {
		this->name->parent = shared_from_this();
		if (globalDecls) for (auto & p : *globalDecls) p->parent = shared_from_this();
	}

	virtual NodeType type() { return '_mod'; }

	/** Get a pipeline, or nullptr if no such function exists in that pipeline. */
	shared_ptr<Pipeline> getPipeline(std::string const& pipeline);

	/** Get a function defined at global scope, or nullptr if no such function exists at global scope. */
	shared_ptr<FunctionDef> getGlobalFunction(std::string const& name);

	/** Get a function defined in a particular pipeline, or nullptr if no such function exists in that
	 * pipeline. */
	shared_ptr<FunctionDef> getPipelineFunction(std::string const& pipeline, std::string const& function);

	/** Get a variable defined in a particular pipeline, or nullptr if no such variable exists in that
	 * pipeline. */
	shared_ptr<VarDecl> getVariable(std::string const& pipeline, std::string const& var);
};


struct IntrinsicFunction : public FunctionDef {

	IntrinsicFunction(shared_ptr<VarDecl> name, shared_ptr<ParameterList> params) : FunctionDef(name, params, nullptr)
	{ }
};

std::string formatParameterList(shared_ptr<ParameterList> const& params);

}} // namespace

#endif //PARASOL_PARASOLPT_H
