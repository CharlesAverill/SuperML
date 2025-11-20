%skeleton "lalr1.cc"
%require  "3.0"
%debug 
%defines 
%define api.namespace {MC}
%define api.parser.class {MC_Parser}

%code requires{
   namespace MC {
      class MC_Driver;
      class MC_Scanner;
   }

   #include "../syntax.h"

// The following definitions is missing when %locations isn't used
# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

}

%parse-param { MC_Scanner  &scanner  }
%parse-param { MC_Driver  &driver  }

%code{
   #include <iostream>
   #include <cstdlib>
   #include <fstream>
   
   /* include for all driver functions */
   #include "driver.hpp"

#undef yylex
#define yylex scanner.yylex
}

%define api.value.type variant
%define parse.assert

%token LET
%token IN
%token FUN
%token COLON
%token ARROW
%token STAR
%token COMMA
%token LPAREN
%token RPAREN
%token TRUE
%token FALSE
%token INTLIT
%token FLOATLIT
%token STRINGLIT
%token UNIT
%token BOOL
%token INT
%token FLOAT
%token STRING
%token ID

%type <Term*> term simple_term app_term atom
%type <Type*> type arrow_type tuple_type base_type
%type <std::string> ID
%type <int> INTLIT
%type <float> FLOATLIT
%type <std::string> STRINGLIT

%locations

%%

program:
    term { driver.root_term = $1; }

term:
      LET ID COLON type '=' term IN term
        {
            $$ = new Term(makeLet($2, *$4, $6, $8));
        }
    | FUN ID COLON type ARROW term
        {
            $$ = new Term(makeFunc($2, *$4, $6));
        }
    | app_term
        { $$ = $1; }
    ;

app_term:
      app_term simple_term
        {
            Type t = makeUnknownType();
            $$ = new Term(makeApp($1, $2, t));
        }
    | simple_term
        { $$ = $1; }
    ;

simple_term:
      atom
        { $$ = $1; }
    | LPAREN term RPAREN
        { $$ = $2; }
    ;

atom:
      TRUE
        { $$ = new Term(makeBool(true)); }
    | FALSE
        { $$ = new Term(makeBool(false)); }
    | INTLIT
        { $$ = new Term(makeInt($1)); }
    | FLOATLIT
        { $$ = new Term(makeFloat($1)); }
    | STRINGLIT
        { $$ = new Term(makeString($1)); }
    | LPAREN RPAREN
        { $$ = new Term(makeUnit()); }
    | ID
        {
            Type t = makeUnknownType();
            $$ = new Term(makeVar($1, /*index*/0, t));
        }
    | LPAREN term COMMA term RPAREN
        {
            $$ = new Term(makeTuple($2,$4));
        }
    ;

type:
      arrow_type
        { $$ = $1; }
    ;

arrow_type:
      tuple_type ARROW arrow_type
        { $$ = new Type(makeArrowType($1,$3)); }
    | tuple_type
        { $$ = $1; }
    ;

tuple_type:
      tuple_type STAR base_type
        { $$ = new Type(makeTupleType($1,$3)); }
    | base_type
        { $$ = $1; }
    ;

base_type:
      LPAREN type RPAREN
        { $$ = $2; }
    | UNIT
        { $$ = new Type(makeUnitType()); }
    | BOOL
        { $$ = new Type(makeBoolType()); }
    | INT
        { $$ = new Type(makeIntType()); }
    | FLOAT
        { $$ = new Type(makeFloatType()); }
    | STRING
        { $$ = new Type(makeStringType()); }
    ;

%%

void 
MC::MC_Parser::error( const location_type &l, const std::string &err_message )
{
   std::cerr << "Error: " << err_message << " at " << l << "\n";
}
