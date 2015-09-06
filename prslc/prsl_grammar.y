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

module ::= pipeline_list.

pipeline_list ::= .
pipeline_list ::= pipeline_list pipeline.

%type pipeline {Pipeline*}
pipeline ::= id L_CURLY pipeline_contents R_CURLY.

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
expr(E) ::= float_(F).    {E = F;}
expr(E)  ::= integer(I).    {E = I;}
expr(E)  ::= id(I).    {E = I;}
expr(E)  ::= function_call(F).    {E = F;}
expr  ::= scoped_var_decl EQUALS expr.
expr  ::= id EQUALS expr.
expr  ::= scoped_var_decl.


%type function_call {FunctionCall*}
function_call(F) ::= id(NAME) L_PAREN arg_list(ARGS) R_PAREN.    {F = new FunctionCall(NAME, ARGS);}


%type arg_list {ArgumentList*}
%destructor arg_list {for (auto p: *$$) delete p; delete $$;}
arg_list(PL) ::= .    {PL = new ArgumentList; }
arg_list(PL) ::= expr(E).    {PL = new ArgumentList; PL->push_back(E);}
arg_list ::= arg_list(PL) COMMA expr(E).    {PL->push_back(E);}


%type scoped_var_decl {VarDecl*}
scoped_var_decl(V) ::= scope(S) var_decl(DECL) R_BRACKET.    {V = new VarDecl(DECL, S); delete DECL;}


%type var_decl {VarDecl*}
var_decl(V) ::= id(NAME).    {V = new VarDecl(NAME, nullptr, nullptr);}
var_decl(V) ::= id(NAME) COLON id(TYPE).    {V = new VarDecl(NAME, TYPE, nullptr); }
var_decl(V) ::= id(NAME) COLON id(TYPE) integer(IDX).    {V = new VarDecl(NAME, TYPE, IDX);}


%type id {Ident*}
id(I) ::= ID(IDL).    {I = new Ident(getstr(p, IDL.value.stringIndex));}

%type scope {Ident*}
scope(I) ::= SCOPEREF(S).    {I = new Ident(getstr(p, S.value.stringIndex));}


%type integer {Integer*}
integer(I) ::= INT_LIT(IL).    {I = new Integer(IL.value.intValue);}


%type float_ {Float*}
float_(F) ::= FLOAT_LIT(FL).    {F = new Float(FL.value.floatValue);}










