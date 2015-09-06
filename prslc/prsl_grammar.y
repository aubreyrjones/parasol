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

module ::= pipeline_list.

pipeline_list ::= .
pipeline_list ::= pipeline_list pipeline.

pipeline ::= id L_CURLY pipeline_contents R_CURLY.

pipeline_contents ::= .
pipeline_contents ::= pipeline_contents expr.
pipeline_contents ::= pipeline_contents function_def.

%type function_def {FunctionDef*}
%destructor function_def {delete $$;}
function_def(F) ::= DEF var_decl(VD) param_list(PL) GOESTO expr(E).    {F = new FunctionDef(VD, PL, E);}
function_def(F) ::= DEF scoped_var_decl(VD) param_list(PL) GOESTO expr(E).    {F = new FunctionDef(VD, PL, E);}

%type param_list {ParameterList *}
%destructor param_list {for (auto p: *$$) delete p; delete $$;}
param_list(PL) ::= .    {PL = new ParameterList;}
param_list(PL) ::= var_decl(V).    {PL = new ParameterList; PL->push_back(V);}
param_list ::= param_list(PL) COMMA var_decl(V).    {PL->push_back(V);}


// expressions... which is most of the language

%nonassoc EQUALS.

%type expr {Expression*}
expr ::= scoped_var_decl EQUALS expr.
expr ::= id EQUALS expr.
expr ::= float_.
expr ::= integer.
expr ::= id.
expr ::= scoped_var_decl.
expr ::= function_call.

%type function_call {FunctionCall*}
%destructor function_call {delete $$;}
function_call(F) ::= id(NAME) L_PAREN arg_list(ARGS) R_PAREN.    {F = new FunctionCall(NAME, ARGS);}

%type arg_list {ArgumentList*}
%destructor arg_list {for (auto p: *$$) delete p; delete $$;}
arg_list(PL) ::= .    {PL = new ArgumentList; }
arg_list(PL) ::= expr(E).    {PL = new ArgumentList; PL->push_back(E);}
arg_list ::= arg_list(PL) COMMA expr(E).    {PL->push_back(E);}

%type scoped_var_decl {VarDecl*}
%destructor scoped_var_decl {delete $$;}
scoped_var_decl(V) ::= id(SCOPE) L_BRACKET var_decl(DECL) R_BRACKET.    {V = new VarDecl(DECL, SCOPE); delete DECL;}

%type var_decl {VarDecl*}
%destructor var_decl {delete $$;}
var_decl(V) ::= id(NAME).    {V = new VarDecl(NAME, nullptr, nullptr);}
var_decl(V) ::= id(NAME) COLON id(TYPE).    {V = new VarDecl(NAME, TYPE, nullptr); }
var_decl(V) ::= id(NAME) COLON id(TYPE) integer(IDX).    {V = new VarDecl(NAME, TYPE, IDX);}

%type id {Ident*}
%destructor id {delete $$;}
id(I) ::= ID(IDL).    {I = new Ident(p->getString(IDL.value.stringIndex));}

%type integer {Integer*}
%destructor integer {delete $$;}
integer(I) ::= INT_LIT(IL).    {I = new Integer(IL.value.intValue);}

%type float_ {Float*}
%destructor float_ {delete $$;}
float_(F) ::= FLOAT_LIT(FL).    {F = new Float(FL.value.floatValue);}