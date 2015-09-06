%token_type {PRSLToken}

%include {
#include <stdio.h>
#include <assert.h>
#include "Token.h"
}

%syntax_error {
  printf("Syntax error.\n");
}

module ::= pipeline_list.

pipeline_list ::= pipeline pipeline_list.
pipeline_list ::= .

pipeline ::= ID L_CURLY decl_list R_CURLY.

decl_list ::= decl decl_list.
decl_list ::= decl_set decl_list.
decl_list ::= .

decl_set ::= decl EQUALS expr.

decl ::= scoped_var_decl.
decl ::= var_decl.

scoped_var_decl ::= ID L_BRACKET var_decl R_BRACKET.

var_decl ::= ID.
var_decl ::= ID COLON ID.
var_decl ::= ID COLON ID UINT.

expr ::= FLOAT_LIT.
expr ::= INT_LIT.

