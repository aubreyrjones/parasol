%name {PRSLParse}
%token_type {PRSLToken}
%extra_argument { prsl::Parser *p }

%include {
#include <stdio.h>
#include <assert.h>
#include "Token.h"
#include "Parser.h"
#include "ParasolPT.h"

using namespace prsl;
using namespace prsl::ast;
}

%syntax_error {
  p->error();
}

%parse_failure {
  p->error();
}

%parse_accept {
  p->success();
}

%default_destructor {delete $$;}


module ::= global_list(GL).    {p->pushAST(GL);}


%type global_list {NodeList*}
%destructor global_list {for (auto p : *$$) delete p; delete $$;}
global_list(G) ::= .    {G = new NodeList;}
global_list(G) ::= global_list(GL) global_item(I).    {GL->push_back(I); G = GL;}


%type global_item {Node*}
global_item(G) ::= pipeline(P).    {G = P;}
global_item(G) ::= function_def(F).    {G = F;}
global_item(G) ::= struct_def(S).    {G = S;}


%type pipeline {Pipeline*}
pipeline(P) ::= id(NAME) L_CURLY pipeline_contents(CONT) R_CURLY.    {P = new Pipeline(NAME, CONT);}


%type pipeline_contents {NodeList*}
%destructor pipeline_contents {for (auto p: *$$) delete p; delete $$;}
pipeline_contents(NL) ::= .    {NL = new NodeList;}
pipeline_contents(N) ::= pipeline_contents(NL) pipeline_item(I).    {NL->push_back(I); N = NL;}


%type pipeline_item {Node*}
pipeline_item(PI) ::= function_def(F).    {PI = F;}
pipeline_item(PI) ::= var_decl(E).    {PI = E;}
pipeline_item(PI) ::= scoped_var_decl(E).    {PI = E;}
pipeline_item(PI) ::= assignment_expr(E).    {PI = E;}
pipeline_item(PI) ::= include_decl(I).    {PI = I;}


%type function_def {FunctionDef*}
function_def(F) ::= DEF var_decl(VD) param_list(PL) GOESTO expr(E).    {F = new FunctionDef(VD, PL, E);}
function_def(F) ::= DEF scoped_var_decl(VD) param_list(PL) GOESTO expr(E).    {F = new FunctionDef(VD, PL, E);}

%type lambda_def {Lambda*}
lambda_def(L) ::= LAMBDA param_list(PL) GOESTO expr(E).    {L = new Lambda(PL, E);}

%type param_list {ParameterList*}
%destructor param_list {for (auto p: *$$) delete p; delete $$;}
param_list(PL) ::= .    {PL = new ParameterList;}
param_list(PL) ::= var_decl(V).    {PL = new ParameterList; PL->push_back(V);}
param_list(P) ::= param_list(PL) COMMA var_decl(V).    {PL->push_back(V); P = PL;}


%type struct_def {StructDef*}
struct_def(S) ::= STRUCT id(N) L_CURLY struct_contents(M) R_CURLY.    {S = new StructDef(N, M);}


%type struct_contents {NodeList*}
%destructor struct_contents {for (auto p : *$$) delete p; delete $$;}
struct_contents(SC) ::= .    {SC = new NodeList;}
struct_contents(S) ::= struct_contents(SC) var_decl(V).    {SC->push_back(V); S = SC;}


%type include_decl {IncludeDecl*}
include_decl(I) ::= INCLUDE id(P).    {I = new IncludeDecl(P, nullptr);}
include_decl(I) ::= INCLUDE id(P) AS id(A).    {I = new IncludeDecl(P, A);}


// expressions... which is most of the language
%type expr {Expression*}
%right LAMBDA LET.
%left GOESTO.
%left COMMA FNCALL.
%left EQUALS.
%left L_AND L_OR.
%left B_AND B_OR.
%left LESS LESS_EQ GREATER GREATER_EQ.
%left EQ NOT_EQ.
%left PLUS MINUS.
%left MULT DIV DOT.
%left SWIZZLE.
%right NOT ELSE.

// declarative expressions
expr(E)  ::= scoped_var_decl(V).    {E = V;}


%type scoped_var_decl {VarDecl*}
scoped_var_decl(V) ::= scope(S) var_decl(DECL) R_BRACKET.    {V = new VarDecl(DECL, S); delete DECL;}


%type scope {Ident*}
scope(I) ::= SCOPEREF(S).    {I = new Ident(getstr(p, S.value.stringIndex));}


%type var_decl {VarDecl*}
var_decl(V) ::= id(NAME).    {V = new VarDecl(NAME, nullptr, nullptr);}
var_decl(V) ::= id(NAME) COLON type_id(TYPE) .    {V = new VarDecl(NAME, TYPE, nullptr); }
var_decl(V) ::= id(NAME) COLON integer(IDX).    {V = new VarDecl(NAME, nullptr, IDX);}
var_decl(V) ::= id(NAME) COLON type_id(TYPE) integer(IDX).    {V = new VarDecl(NAME, TYPE, IDX);}



// arithmetic expressions
expr(E) ::= id(I).    {E = I;}
expr(E) ::= float_(F).    {E = F;}
expr(E) ::= integer(I).    {E = I;}
expr(E) ::= function_call(F).    {E = F;}

expr(E) ::= assignment_expr(I).    {E = I;}

expr(E) ::= expr(L) PLUS expr(R).    {E = new BinaryOp(PLUS, L, R);}
expr(E) ::= expr(L) MINUS expr(R).    {E = new BinaryOp(MINUS, L, R);}
expr(E) ::= expr(L) MULT expr(R).    {E = new BinaryOp(MULT, L, R);}
expr(E) ::= expr(L) DIV expr(R).    {E = new BinaryOp(DIV, L, R);}
expr(E) ::= expr(L) DOT expr(R).    {E = new BinaryOp(DOT, L, R);}
expr(E) ::= NOT expr(I).    {E = new UnaryOp(NOT, I);}
expr(E) ::= MINUS expr(I). [NOT]    {E = new UnaryOp(MINUS, I);}
expr(E) ::= L_PAREN expr(I) R_PAREN.    {E = I;}
expr(E) ::= let_expr(I).    {E = I;}
expr(E) ::= case_set(CS).    {E = CS;}
expr(E) ::= lambda_def(L).    {E = L;}
expr(E) ::= expr(L) SWIZZLE expr(R).    {E = new BinaryOp(SWIZZLE, L, R);}

expr(E) ::= expr(L) L_AND expr(R).    {E = new BinaryOp(L_AND, L, R);}
expr(E) ::= expr(L) L_OR expr(R).    {E = new BinaryOp(L_OR, L, R);}

expr(E) ::= expr(L) B_AND expr(R).    {E = new BinaryOp(B_AND, L, R);}
expr(E) ::= expr(L) B_OR expr(R).    {E = new BinaryOp(B_OR, L, R);}

expr(E) ::= expr(L) LESS expr(R).    {E = new BinaryOp(LESS, L, R);}
expr(E) ::= expr(L) LESS_EQ expr(R).    {E = new BinaryOp(LESS_EQ, L, R);}

expr(E) ::= expr(L) GREATER expr(R).    {E = new BinaryOp(GREATER, L, R);}
expr(E) ::= expr(L) GREATER_EQ expr(R).    {E = new BinaryOp(GREATER_EQ, L, R);}

expr(E) ::= expr(L) EQ expr(R).    {E = new BinaryOp(EQ, L, R);}
expr(E) ::= expr(L) NOT_EQ expr(R).    {E = new BinaryOp(NOT_EQ, L, R);}

%type let_expr {Let*}
let_expr(L) ::= LET unscoped_assignment_list(AL) IN expr(E).    {L = new Let(AL, E);}


%type unscoped_assignment_list {NodeList*}
%destructor unscoped_assignment_list {for (auto p: *$$) delete p; delete $$;}
unscoped_assignment_list(AL) ::= .    {AL = new NodeList;}
unscoped_assignment_list(A) ::= unscoped_assignment_list(AL) unscoped_assignment_expr(E).    {AL->push_back(E); A = AL;}


%type case_set {CaseSet*}
case_set(CS) ::= L_CURLY case_list(CL) R_CURLY.    {CS = new CaseSet(CL);}


%type case_list {CaseList*}
case_list(CL) ::= .    {CL = new CaseList;}
case_list(C) ::= case_list(CL) case(CS).    {CL->push_back(CS); C = CL;}


%type case {Case*}
case(C) ::= expr(COND) GOESTO expr(R).    {C = new Case(COND, R);}


%type assignment_expr {Expression*}
assignment_expr(E) ::= unscoped_assignment_expr(I).    {E = I;}
assignment_expr(E) ::= scoped_var_decl(L) EQUALS expr(R).    {E = new BinaryOp(EQUALS, L, R);}


%type unscoped_assignment_expr {Expression*}
unscoped_assignment_expr(E) ::= var_decl(L) EQUALS expr(R).    {E = new BinaryOp(EQUALS, L, R);}


%type function_call {FunctionCall*}
function_call(F) ::= fncall(NAME) arg_list(ARGS) R_PAREN.    {F = new FunctionCall(NAME, ARGS);}


%type fncall {Ident*}
fncall(I) ::= FNCALL(F).    {I = new Ident(getstr(p, F.value.stringIndex));}


%type arg_list {ArgumentList*}
%destructor arg_list {for (auto p: *$$) delete p; delete $$;}
arg_list(PL) ::= .    {PL = new ArgumentList; }
arg_list(PL) ::= expr(E).    {PL = new ArgumentList; PL->push_back(E);}
arg_list(A) ::= arg_list(PL) COMMA expr(E).    {PL->push_back(E); A = PL;}


%type id {Ident*}
id(I) ::= ID(IDL).    {I = new Ident(getstr(p, IDL.value.stringIndex));}


%type type_id {TypeIdent*}
type_id(I) ::= ID(IDL).    {I = new TypeIdent(getstr(p, IDL.value.stringIndex));}
type_id(I) ::= ID(IDL) ARRAY integer(A).    {I = new TypeIdent(getstr(p, IDL.value.stringIndex), A->value); delete A;}


%type integer {Integer*}
integer(I) ::= INT_LIT(IL).    {I = new Integer(IL.value.intValue);}


%type float_ {Float*}
float_(F) ::= FLOAT_LIT(FL).    {F = new Float(FL.value.floatValue);}










