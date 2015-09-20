%name {PRSLParse}
%token_type {PRSLToken}
%extra_argument { prsl::Parser *p }

%include {
#include <stdio.h>
#include <assert.h>
#include "Token.h"
#include "Parser.h"
#include "ParasolAST.h"

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
global_item(G) ::= struct_def(S).    {G = S;}


%type pipeline {Pipeline*}
pipeline(P) ::= id(NAME) L_CURLY pipeline_contents(CONT) R_CURLY.    {P = new Pipeline(NAME, CONT); P->line = NAME->line;}


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
function_def(F) ::= DEF(D) var_decl(VD) param_list(PL) GOESTO expr(E).    {F = new FunctionDef(VD, PL, E); F->line = D.line;}
function_def(F) ::= DEF(D) scoped_var_decl(VD) param_list(PL) GOESTO expr(E).    {F = new FunctionDef(VD, PL, E); F->line = D.line;}

%type lambda_def {Lambda*}
lambda_def(L) ::= LAMBDA(LTOK) param_list(PL) GOESTO expr(E).    {L = new Lambda(PL, E); L->line = LTOK.line;}

%type param_list {ParameterList*}
%destructor param_list {for (auto p: *$$) delete p; delete $$;}
param_list(PL) ::= .    {PL = new ParameterList;}
param_list(PL) ::= var_decl(V).    {PL = new ParameterList; PL->push_back(V);}
param_list(P) ::= param_list(PL) COMMA var_decl(V).    {PL->push_back(V); P = PL;}


%type struct_def {StructDef*}
struct_def(S) ::= STRUCT(STOK) id(N) L_CURLY struct_contents(M) R_CURLY.    {S = new StructDef(N, M); S->line = STOK.line;}


%type struct_contents {NodeList*}
%destructor struct_contents {for (auto p : *$$) delete p; delete $$;}
struct_contents(SC) ::= .    {SC = new NodeList;}
struct_contents(S) ::= struct_contents(SC) var_decl(V).    {SC->push_back(V); S = SC;}


%type include_decl {IncludeDecl*}
include_decl(I) ::= INCLUDE(ITOK) id(P).    {I = new IncludeDecl(P, nullptr); I->line = ITOK.line;}
include_decl(I) ::= INCLUDE(ITOK) id(P) AS id(A).    {I = new IncludeDecl(P, A); I->line = ITOK.line;}


// expressions... which is most of the language
%type expr {Expression*}
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


%type scoped_var_decl {VarDecl*}
scoped_var_decl(V) ::= scope(S) var_decl(DECL) R_BRACKET.    {V = new VarDecl(DECL, S); V->line = DECL->line; delete DECL;}


%type scope {Ident*}
scope(I) ::= SCOPEREF(S).    {I = new Ident(getstr(p, S.value.stringIndex)); I->line = S.line;}


%type var_decl {VarDecl*}
var_decl(V) ::= id(NAME).    {V = new VarDecl(NAME, nullptr, nullptr); V->line = NAME->line;}
var_decl(V) ::= id(NAME) COLON type_id(TYPE) .    {V = new VarDecl(NAME, TYPE, nullptr); V->line = NAME->line;}
var_decl(V) ::= id(NAME) COLON integer(IDX).    {V = new VarDecl(NAME, nullptr, IDX); V->line = NAME->line;}
var_decl(V) ::= id(NAME) COLON type_id(TYPE) integer(IDX).    {V = new VarDecl(NAME, TYPE, IDX); V->line = NAME->line;}



// arithmetic expressions
expr(E) ::= id(I).    {E = I;}
expr(E) ::= float_(F).    {E = F;}
expr(E) ::= integer(I).    {E = I;}
expr(E) ::= function_call(F).    {E = F;}

expr(E) ::= assignment_expr(I).    {E = I;}

expr(E) ::= expr(L) PLUS expr(R).    {E = new BinaryOp(PLUS, L, R); E->line = L->line;}
expr(E) ::= expr(L) MINUS expr(R).    {E = new BinaryOp(MINUS, L, R); E->line = L->line;}
expr(E) ::= expr(L) MULT expr(R).    {E = new BinaryOp(MULT, L, R); E->line = L->line;}
expr(E) ::= expr(L) DIV expr(R).    {E = new BinaryOp(DIV, L, R); E->line = L->line;}
expr(E) ::= expr(L) DOT expr(R).    {E = new BinaryOp(DOT, L, R); E->line = L->line;}
expr(E) ::= expr(L) SEQUENCE expr(R). {E = new BinaryOp(SEQUENCE, L, R); E->line = L->line;}
expr(E) ::= NOT expr(I).    {E = new UnaryOp(NOT, I); E->line = I->line;}
expr(E) ::= MINUS expr(I). [NOT]    {E = new UnaryOp(MINUS, I); E->line = I->line;}
expr(E) ::= L_PAREN expr(I) R_PAREN.    {E = I;}
expr(E) ::= let_expr(I).    {E = I;}
expr(E) ::= case_set(CS).    {E = CS;}
expr(E) ::= lambda_def(L).    {E = L;}
expr(E) ::= expr(L) SWIZZLE expr(R).    {E = new BinaryOp(SWIZZLE, L, R); E->line = L->line;}

expr(E) ::= expr(L) L_AND expr(R).    {E = new BinaryOp(L_AND, L, R); E->line = L->line;}
expr(E) ::= expr(L) L_OR expr(R).    {E = new BinaryOp(L_OR, L, R); E->line = L->line;}

expr(E) ::= expr(L) B_AND expr(R).    {E = new BinaryOp(B_AND, L, R); E->line = L->line;}
expr(E) ::= expr(L) B_OR expr(R).    {E = new BinaryOp(B_OR, L, R); E->line = L->line;}

expr(E) ::= expr(L) LESS expr(R).    {E = new BinaryOp(LESS, L, R); E->line = L->line;}
expr(E) ::= expr(L) LESS_EQ expr(R).    {E = new BinaryOp(LESS_EQ, L, R); E->line = L->line;}

expr(E) ::= expr(L) GREATER expr(R).    {E = new BinaryOp(GREATER, L, R); E->line = L->line;}
expr(E) ::= expr(L) GREATER_EQ expr(R).    {E = new BinaryOp(GREATER_EQ, L, R); E->line = L->line;}

expr(E) ::= expr(L) EQ expr(R).    {E = new BinaryOp(EQ, L, R); E->line = L->line;}
expr(E) ::= expr(L) NOT_EQ expr(R).    {E = new BinaryOp(NOT_EQ, L, R); E->line = L->line;}

%type let_expr {Let*}
let_expr(L) ::= LET(LTOK) unscoped_assignment_list(AL) IN expr(E).    {L = new Let(AL, E); L->line = LTOK.line;}


%type unscoped_assignment_list {NodeList*}
%destructor unscoped_assignment_list {for (auto p: *$$) delete p; delete $$;}
unscoped_assignment_list(AL) ::= .    {AL = new NodeList;}
unscoped_assignment_list(A) ::= unscoped_assignment_list(AL) unscoped_assignment_expr(E).    {AL->push_back(E); A = AL;}


%type case_set {PsiExpr*}
case_set(CS) ::= L_CURLY(LCTOK) case_list(CL) R_CURLY.    {CS = new PsiExpr(CL); CS->line = LCTOK.line;}


%type case_list {CaseList*}
case_list(CL) ::= .    {CL = new CaseList;}
case_list(C) ::= case_list(CL) case(CS).    {CL->push_back(CS); C = CL;}


%type case {Case*}
case(C) ::= expr(COND) GOESTO expr(R).    {C = new Case(COND, R); C->line = COND->line;}


%type assignment_expr {Expression*}
assignment_expr(E) ::= unscoped_assignment_expr(I).    {E = I;}
assignment_expr(E) ::= scoped_var_decl(L) EQUALS expr(R).    {E = new BinaryOp(EQUALS, L, R); E->line = L->line;}


%type unscoped_assignment_expr {Expression*}
unscoped_assignment_expr(E) ::= var_decl(L) EQUALS expr(R).    {E = new BinaryOp(EQUALS, L, R); E->line = L->line;}


%type function_call {FunctionCall*}
function_call(F) ::= fncall(NAME) arg_list(ARGS) R_PAREN.    {F = new FunctionCall(NAME, ARGS); F->line = NAME->line;}


%type fncall {Ident*}
fncall(I) ::= FNCALL(F).    {I = new Ident(getstr(p, F.value.stringIndex)); I->line = F.line;}


%type arg_list {ArgumentList*}
%destructor arg_list {for (auto p: *$$) delete p; delete $$;}
arg_list(PL) ::= .    {PL = new ArgumentList; }
arg_list(PL) ::= expr(E).    {PL = new ArgumentList; PL->push_back(E);}
arg_list(A) ::= arg_list(PL) COMMA expr(E).    {PL->push_back(E); A = PL;}


%type id {Ident*}
id(I) ::= ID(IDL).    {I = new Ident(getstr(p, IDL.value.stringIndex)); I->line = IDL.line;}


%type type_id {TypeIdent*}
type_id(I) ::= ID(IDL).    {I = new TypeIdent(getstr(p, IDL.value.stringIndex)); I->line = IDL.line;}
type_id(I) ::= ID(IDL) ARRAY integer(A).    {I = new TypeIdent(getstr(p, IDL.value.stringIndex), A->value); delete A; I->line = IDL.line;}


%type integer {Integer*}
integer(I) ::= INT_LIT(IL).    {I = new Integer(IL.value.intValue); I->line = IL.line;}


%type float_ {Float*}
float_(F) ::= FLOAT_LIT(FL).    {F = new Float(FL.value.floatValue); F->line = FL.line;}










