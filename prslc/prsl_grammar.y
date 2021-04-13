%name {PRSLParse}
%token_type {PRSLToken}
%extra_argument { prsl::Parser *p }

%include {
#include <stdio.h>
#include <assert.h>
#include <memory>
#include "Token.h"
#include "Parser.h"
#include "ParasolAST.h"

using namespace prsl;
using namespace prsl::ast;
using std::shared_ptr;
using std::make_shared;

#define NOP() 
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

%default_destructor { NOP(); }


module ::= global_list(GL).    {p->pushAST(GL);}


%type global_list {shared_ptr<NodeList>}
//%destructor global_list {for (auto p : *$$) delete p; delete $$;}
global_list(G) ::= .    {G = make_shared<NodeList>();}
global_list(G) ::= global_list(GL) global_item(I).    {GL->push_back(I); G = GL;}


%type global_item {shared_ptr<Node>}
global_item(G) ::= pipeline(P).    {G = P;}
global_item(G) ::= struct_def(S).    {G = S;}


%type pipeline {shared_ptr<Pipeline>}
pipeline(P) ::= id(NAME) L_CURLY pipeline_contents(CONT) R_CURLY.    {P = make_node<Pipeline>(NAME, CONT); P->line = NAME->line;}


%type pipeline_contents {shared_ptr<NodeList>}
//%destructor pipeline_contents {for (auto p: *$$) delete p; delete $$;}
pipeline_contents(NL) ::= .    {NL = make_shared<NodeList>();}
pipeline_contents(N) ::= pipeline_contents(NL) pipeline_item(I).    {NL->push_back(I); N = NL;}


%type pipeline_item {shared_ptr<Node>}
pipeline_item(PI) ::= function_def(F).    {PI = F;}
pipeline_item(PI) ::= var_decl(E).    {PI = E;}
pipeline_item(PI) ::= scoped_var_decl(E).    {PI = E;}
pipeline_item(PI) ::= assignment_expr(E).    {PI = E;}
pipeline_item(PI) ::= include_decl(I).    {PI = I;}


%type function_def {shared_ptr<FunctionDef>}
function_def(F) ::= DEF(D) var_decl(VD) param_list(PL) GOESTO expr(E).    {F = make_node<FunctionDef>(VD, PL, E); F->line = D.line;}
function_def(F) ::= DEF(D) scoped_var_decl(VD) param_list(PL) GOESTO expr(E).    {F = make_node<FunctionDef>(VD, PL, E); F->line = D.line;}

%type lambda_def {shared_ptr<Lambda>}
lambda_def(L) ::= LAMBDA(LTOK) param_list(PL) GOESTO expr(E).    {L = make_node<Lambda>(PL, E); L->line = LTOK.line;}

%type param_list {shared_ptr<ParameterList>}
//%destructor param_list {for (auto p: *$$) delete p; delete $$;}
param_list(PL) ::= .    {PL = make_shared<ParameterList>();}
param_list(PL) ::= var_decl(V).    {PL = make_shared<ParameterList>; PL->push_back(V);}
param_list(P) ::= param_list(PL) COMMA var_decl(V).    {PL->push_back(V); P = PL;}


%type struct_def {shared_ptr<StructDef>}
struct_def(S) ::= STRUCT(STOK) id(N) L_CURLY struct_contents(M) R_CURLY.    {S = make_node<StructDef>(N, M); S->line = STOK.line;}


%type struct_contents {shared_ptr<NodeList>}
//%destructor struct_contents {for (auto p : *$$) delete p; delete $$;}
struct_contents(SC) ::= .    {SC = make_shared<NodeList>();}
struct_contents(S) ::= struct_contents(SC) var_decl(V).    {SC->push_back(V); S = SC;}


%type include_decl {shared_ptr<IncludeDecl>}
include_decl(I) ::= INCLUDE(ITOK) id(P).    {I = make_node<IncludeDecl>(P, nullptr); I->line = ITOK.line;}
include_decl(I) ::= INCLUDE(ITOK) id(P) AS id(A).    {I = make_node<IncludeDecl>(P, A); I->line = ITOK.line;}


// expressions... which is most of the language
%type expr {shared_ptr<Expression>}
%right LAMBDA LET.
%left GOESTO.
%left COMMA FNCALL.
%right EQUALS.
%left L_AND L_OR.
%left B_AND B_OR.
%left LESS LESS_EQ GREATER GREATER_EQ.
%left EQ NOT_EQ.
%left PLUS MINUS.
%left MULT DIV DOT.
%left SWIZZLE SEQUENCE.
%right NOT ELSE.

// declarative expressions
expr(E)  ::= scoped_var_decl(V).    {E = V;}


%type scoped_var_decl {shared_ptr<VarDecl>}
scoped_var_decl(V) ::= scope(S) var_decl(DECL) R_BRACKET.    {V = make_node<VarDecl>(DECL, S); V->line = DECL->line; delete DECL;}


%type scope {shared_ptr<Ident>}
scope(I) ::= SCOPEREF(S).    {I = make_node<Ident>(getstr(p, S.value.stringIndex)); I->line = S.line;}


%type var_decl {shared_ptr<VarDecl>}
var_decl(V) ::= id(NAME).    {V = make_node<VarDecl>(NAME, nullptr, nullptr); V->line = NAME->line;}
var_decl(V) ::= id(NAME) COLON type_id(TYPE) .    {V = make_node<VarDecl>(NAME, TYPE, nullptr); V->line = NAME->line;}
var_decl(V) ::= id(NAME) COLON integer(IDX).    {V = make_node<VarDecl>(NAME, nullptr, IDX); V->line = NAME->line;}
var_decl(V) ::= id(NAME) COLON type_id(TYPE) integer(IDX).    {V = make_node<VarDecl>(NAME, TYPE, IDX); V->line = NAME->line;}



// arithmetic expressions
expr(E) ::= id(I).    {E = I;}
expr(E) ::= float_(F).    {E = F;}
expr(E) ::= integer(I).    {E = I;}
expr(E) ::= function_call(F).    {E = F;}

expr(E) ::= assignment_expr(I).    {E = I;}

expr(E) ::= expr(L) PLUS expr(R).    {E = make_node<BinaryOp>(PLUS, L, R); E->line = L->line;}
expr(E) ::= expr(L) MINUS expr(R).    {E = make_node<BinaryOp>(MINUS, L, R); E->line = L->line;}
expr(E) ::= expr(L) MULT expr(R).    {E = make_node<BinaryOp>(MULT, L, R); E->line = L->line;}
expr(E) ::= expr(L) DIV expr(R).    {E = make_node<BinaryOp>(DIV, L, R); E->line = L->line;}
expr(E) ::= expr(L) DOT expr(R).    {E = make_node<BinaryOp>(DOT, L, R); E->line = L->line;}
expr(E) ::= expr(L) SEQUENCE expr(R). {E = make_node<BinaryOp>(SEQUENCE, L, R); E->line = L->line;}
expr(E) ::= NOT expr(I).    {E = make_node<UnaryOp>(NOT, I); E->line = I->line;}
expr(E) ::= MINUS expr(I). [NOT]    {E = make_node<UnaryOp>(MINUS, I); E->line = I->line;}
expr(E) ::= L_PAREN expr(I) R_PAREN.    {E = I;}
expr(E) ::= let_expr(I).    {E = I;}
expr(E) ::= case_set(CS).    {E = CS;}
expr(E) ::= lambda_def(L).    {E = L;}
expr(E) ::= expr(L) SWIZZLE expr(R).    {E = make_node<BinaryOp>(SWIZZLE, L, R); E->line = L->line;}

expr(E) ::= expr(L) L_AND expr(R).    {E = make_node<BinaryOp>(L_AND, L, R); E->line = L->line;}
expr(E) ::= expr(L) L_OR expr(R).    {E = make_node<BinaryOp>(L_OR, L, R); E->line = L->line;}

expr(E) ::= expr(L) B_AND expr(R).    {E = make_node<BinaryOp>(B_AND, L, R); E->line = L->line;}
expr(E) ::= expr(L) B_OR expr(R).    {E = make_node<BinaryOp>(B_OR, L, R); E->line = L->line;}

expr(E) ::= expr(L) LESS expr(R).    {E = make_node<BinaryOp>(LESS, L, R); E->line = L->line;}
expr(E) ::= expr(L) LESS_EQ expr(R).    {E = make_node<BinaryOp>(LESS_EQ, L, R); E->line = L->line;}

expr(E) ::= expr(L) GREATER expr(R).    {E = make_node<BinaryOp>(GREATER, L, R); E->line = L->line;}
expr(E) ::= expr(L) GREATER_EQ expr(R).    {E = make_node<BinaryOp>(GREATER_EQ, L, R); E->line = L->line;}

expr(E) ::= expr(L) EQ expr(R).    {E = make_node<BinaryOp>(EQ, L, R); E->line = L->line;}
expr(E) ::= expr(L) NOT_EQ expr(R).    {E = make_node<BinaryOp>(NOT_EQ, L, R); E->line = L->line;}

%type let_expr {shared_ptr<Let>}
let_expr(L) ::= LET(LTOK) unscoped_assignment_list(AL) IN expr(E).    {L = make_node<Let>(AL, E); L->line = LTOK.line;}


%type unscoped_assignment_list {shared_ptr<NodeList>}
//%destructor unscoped_assignment_list {for (auto p: *$$) delete p; delete $$;}
unscoped_assignment_list(AL) ::= .    {AL = make_shared<NodeList>();}
unscoped_assignment_list(A) ::= unscoped_assignment_list(AL) unscoped_assignment_expr(E).    {AL->push_back(E); A = AL;}


%type case_set {shared_ptr<PsiExpr>}
case_set(CS) ::= L_CURLY(LCTOK) case_list(CL) R_CURLY.    {CS = make_node<PsiExpr>(CL); CS->line = LCTOK.line;}


%type case_list {shared_ptr<CaseList>}
case_list(CL) ::= .    {CL = make_shared<CaseList>();}
case_list(C) ::= case_list(CL) case(CS).    {CL->push_back(CS); C = CL;}


%type case {shared_ptr<Case>}
case(C) ::= expr(COND) GOESTO expr(R).    {C = make_node<Case>(COND, R); C->line = COND->line;}


%type assignment_expr {shared_ptr<Expression>}
assignment_expr(E) ::= unscoped_assignment_expr(I).    {E = I;}
assignment_expr(E) ::= scoped_var_decl(L) EQUALS expr(R).    {E = make_node<BinaryOp>(EQUALS, L, R); E->line = L->line;}


%type unscoped_assignment_expr {shared_ptr<Expression>}
unscoped_assignment_expr(E) ::= var_decl(L) EQUALS expr(R).    {E = make_node<BinaryOp>(EQUALS, L, R); E->line = L->line;}


%type function_call {shared_ptr<FunctionCall>}
function_call(F) ::= fncall(NAME) arg_list(ARGS) R_PAREN.    {F = make_node<FunctionCall>(NAME, ARGS); F->line = NAME->line;}


%type fncall {shared_ptr<Ident>}
fncall(I) ::= FNCALL(F).    {I = make_node<Ident>(getstr(p, F.value.stringIndex)); I->line = F.line;}


%type arg_list {shared_ptr<ArgumentList>}
//%destructor arg_list {for (auto p: *$$) delete p; delete $$;}
arg_list(PL) ::= .    {PL = make_shared<ArgumentList>(); }
arg_list(PL) ::= expr(E).    {PL = make_shared<ArgumentList>(); PL->push_back(E);}
arg_list(A) ::= arg_list(PL) COMMA expr(E).    {PL->push_back(E); A = PL;}


%type id {shared_ptr<Ident>}
id(I) ::= ID(IDL).    {I = make_node<Ident>(getstr(p, IDL.value.stringIndex)); I->line = IDL.line;}


%type type_id {shared_ptr<TypeIdent>}
type_id(I) ::= ID(IDL).    {I = make_node<TypeIdent>(getstr(p, IDL.value.stringIndex)); I->line = IDL.line;}
type_id(I) ::= ID(IDL) ARRAY integer(A).    {I = make_node<TypeIdent>(getstr(p, IDL.value.stringIndex), A->value); delete A; I->line = IDL.line;}


%type integer {shared_ptr<Integer>}
integer(I) ::= INT_LIT(IL).    {I = make_node<Integer>(IL.value.intValue); I->line = IL.line;}


%type float_ {shared_ptr<Float>}
float_(F) ::= FLOAT_LIT(FL).    {F = make_node<Float>(FL.value.floatValue); F->line = FL.line;}










