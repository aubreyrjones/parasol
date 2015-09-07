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


%type pipeline {Pipeline*}
pipeline(P) ::= id(NAME) L_CURLY pipeline_contents(CONT) R_CURLY.    {P = new Pipeline(NAME, CONT);}


%type pipeline_contents {NodeList*}
%destructor pipeline_contents {for (auto p: *$$) delete p; delete $$;}
pipeline_contents(NL) ::= .    {NL = new NodeList;}
//pipeline_contents(NL) ::= pipeline_item(I).    {NL = new NodeList; NL->push_back(I);}
pipeline_contents(N) ::= pipeline_contents(NL) pipeline_item(I).    {NL->push_back(I); N = NL;}


%type pipeline_item {Node*}
pipeline_item(PI) ::= function_def(F).    {PI = F;}
pipeline_item(PI) ::= expr(E).    {PI = E;}


%type function_def {FunctionDef*}
function_def(F) ::= DEF var_decl(VD) param_list(PL) GOESTO expr(E).    {F = new FunctionDef(VD, PL, E);}
function_def(F) ::= DEF scoped_var_decl(VD) param_list(PL) GOESTO expr(E).    {F = new FunctionDef(VD, PL, E);}


%type param_list {ParameterList*}
%destructor param_list {for (auto p: *$$) delete p; delete $$;}
param_list(PL) ::= .    {PL = new ParameterList;}
param_list(PL) ::= var_decl(V).    {PL = new ParameterList; PL->push_back(V);}
param_list(P) ::= param_list(PL) COMMA var_decl(V).    {PL->push_back(V); P = PL;}



// expressions... which is most of the language
%type expr {Expression*}

// declarative expressions
expr(E)  ::= scoped_var_decl(V).    {E = V;}
expr  ::= scoped_var_decl EQUALS expr.


%type scoped_var_decl {VarDecl*}
scoped_var_decl ::= scope var_decl R_BRACKET.//    {V = new VarDecl(DECL, S); delete DECL;}


%type scope {Ident*}
scope(I) ::= SCOPEREF(S).    {I = new Ident(getstr(p, S.value.stringIndex));}


%type var_decl {VarDecl*}
var_decl(V) ::= id(NAME).    {V = new VarDecl(NAME, nullptr, nullptr);}
var_decl(V) ::= id(NAME) COLON id(TYPE).    {V = new VarDecl(NAME, TYPE, nullptr); }
var_decl(V) ::= id(NAME) COLON integer(IDX).    {V = new VarDecl(NAME, nullptr, IDX);}
var_decl(V) ::= id(NAME) COLON id(TYPE) integer(IDX).    {V = new VarDecl(NAME, TYPE, IDX);}


// arithmetic expressions
expr  ::= id EQUALS expr.
expr(E)  ::= id(I).    {E = I;}
expr(E) ::= float_(F).    {E = F;}
expr(E)  ::= integer(I).    {E = I;}
expr(E)  ::= function_call(F).    {E = F;}


%type function_call {FunctionCall*}
function_call(F) ::= id(NAME) L_PAREN arg_list(ARGS) R_PAREN.    {F = new FunctionCall(NAME, ARGS);}


%type arg_list {ArgumentList*}
%destructor arg_list {for (auto p: *$$) delete p; delete $$;}
arg_list(PL) ::= .    {PL = new ArgumentList; }
arg_list(PL) ::= expr(E).    {PL = new ArgumentList; PL->push_back(E);}
arg_list ::= arg_list(PL) COMMA expr(E).    {PL->push_back(E);}


%type id {Ident*}
id(I) ::= ID(IDL).    {I = new Ident(getstr(p, IDL.value.stringIndex));}


%type integer {Integer*}
integer(I) ::= INT_LIT(IL).    {I = new Integer(IL.value.intValue);}


%type float_ {Float*}
float_(F) ::= FLOAT_LIT(FL).    {F = new Float(FL.value.floatValue);}










