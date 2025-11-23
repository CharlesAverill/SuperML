%skeleton "lalr1.cc"
%require "3.0"
// %debug
%defines
%define api.namespace {MC}
%define api.parser.class {MC_Parser}

%code requires {
    namespace MC {
        class MC_Driver;
        class MC_Scanner;
    }

    #include "../syntax.h"
}

%parse-param { MC_Scanner &scanner }
%parse-param { MC_Driver &driver }

%code {
    #include <iostream>
    #include <cstdlib>
    #include <fstream>

    #include "driver.hpp"

    #undef yylex
    #define yylex scanner.yylex
}

%define api.value.type variant
%define parse.assert

/* ---------- Token Definitions ---------- */
%token LET IN FUN COLON SEMICOLON ARROW STAR COMMA
%token LPAREN RPAREN
%token TRUE FALSE
%token INTLIT FLOATLIT STRINGLIT
%token UNIT BOOL INT FLOAT STRING
%token ID

/* ---------- Modern C++ Types ---------- */
%type <Term> term simple_term app_term atom
%type <Type> type arrow_type tuple_type base_type

%type <std::string> ID STRINGLIT
%type <int> INTLIT
%type <float> FLOATLIT

%locations

%%

/* ===========================================================
   PROGRAM
   =========================================================== */

program:
    term  { driver.root_term = $1; }
    ;

/* ===========================================================
   TERMS
   =========================================================== */

term:
      LET ID COLON type '=' term IN term
        {
            $$ = TermNode::LetTerm($2, $4, $6, $8);
        }

    | FUN ID COLON type ARROW term
        {
            $$ = TermNode::AbsTerm($2, $4, $6);
        }

    | term SEMICOLON term
        {
            // desugar: t1 ; t2  ==>  let _ : unit = t1 in t2
            $$ = TermNode::LetTerm("_", TypeNode::Unit(), $1, $3);
        }

    | app_term   { $$ = $1; }
    ;

/* ===========================================================
   APPLICATION CHAINS
   =========================================================== */

app_term:
      app_term simple_term
        {
            $$ = TermNode::AppTerm($1, $2);
        }
    | simple_term   { $$ = $1; }
    ;

/* ===========================================================
   SIMPLE TERMS
   =========================================================== */

simple_term:
      atom              { $$ = $1; }
    | LPAREN term RPAREN { $$ = $2; }
    ;

/* ===========================================================
   ATOMS
   =========================================================== */

atom:
      TRUE       { $$ = TermNode::Bool(true); }
    | FALSE      { $$ = TermNode::Bool(false); }
    | INTLIT     { $$ = TermNode::Int($1); }
    | FLOATLIT   { $$ = TermNode::Float($1); }
    | STRINGLIT  { $$ = TermNode::String($1); }
    | LPAREN RPAREN { $$ = TermNode::Unit(); }

    | ID
        { 
            $$ = TermNode::VarTerm($1, /*index*/0, TypeNode::Unknown());
        }

    | LPAREN term COMMA term RPAREN
        {
            $$ = TermNode::TupleTerm($2, $4);
        }
    ;

/* ===========================================================
   TYPES
   =========================================================== */

type:
      arrow_type   { $$ = $1; }
    ;

arrow_type:
      tuple_type ARROW arrow_type
        { $$ = TypeNode::ArrowType($1, $3); }

    | tuple_type
        { $$ = $1; }
    ;


tuple_type:
      tuple_type STAR base_type
        { $$ = TypeNode::TupleType($1, $3); }

    | base_type
        { $$ = $1; }
    ;

/* ===========================================================
   BASE TYPES
   =========================================================== */

base_type:
      LPAREN type RPAREN   { $$ = $2; }

    | UNIT   { $$ = TypeNode::Unit(); }
    | BOOL   { $$ = TypeNode::Bool(); }
    | INT    { $$ = TypeNode::Int(); }
    | FLOAT  { $$ = TypeNode::Float(); }
    | STRING { $$ = TypeNode::String(); }
    ;

%%

void MC::MC_Parser::error(const location_type &l, const std::string &msg)
{
    std::cerr << "Error: " << msg << " at " << l << "\n";
}
