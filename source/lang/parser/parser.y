%skeleton "lalr1.cc"
%require "3.0"
%debug
%defines
%define api.namespace {MC}
%define api.parser.class {MC_Parser}

%code requires {
    namespace MC {
        class MC_Driver;
        class MC_Scanner;
    }

    #include "../syntax.h"
    #include "../../globals.h"
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
%token LET IN FUN COLON SEMICOLON ARROW STAR COMMA EQUAL
%token LPAREN RPAREN
%token TRUE FALSE
%token INTLIT FLOATLIT STRINGLIT
%token UNIT BOOL INT FLOAT STRING
%token ID

/* ---------- Modern C++ Types ---------- */
%type <Term> term nonlet_term app_term atom
%type <Type> type arrow_type tuple_type base_type

%type <std::string> ID STRINGLIT
%type <int> INTLIT
%type <float> FLOATLIT

%right SEMICOLON
%left EQUAL
%nonassoc IN
%right ARROW

%locations

%%

program:
    term  { driver.root_term = $1; }
    ;

term:
      nonlet_term
    | LET ID COLON type EQUAL term IN term
        { $$ = TermNode::LetTerm($2, $4, $6, $8); }
    | nonlet_term SEMICOLON term
        { $$ = TermNode::LetTerm("_", TypeNode::Unit(), $1, $3); }
    ;

nonlet_term:
      FUN ID COLON type EQUAL term
        { $$ = TermNode::AbsTerm($2, $4, $6); }
    | app_term
        { $$ = $1; }

app_term:
      app_term atom
        {
            $$ = TermNode::AppTerm($1, $2);
        }
    | atom   { $$ = $1; }
    ;

atom:
      TRUE       { $$ = TermNode::Bool(true); }
    | FALSE      { $$ = TermNode::Bool(false); }
    | INTLIT     { $$ = TermNode::Int($1); }
    | FLOATLIT   { $$ = TermNode::Float($1); }
    | STRINGLIT  { $$ = TermNode::String($1); }
    | ID         { $$ = TermNode::VarTerm($1, 0, TypeNode::Unknown()); }

    | LPAREN RPAREN
        { $$ = TermNode::Unit(); }

    | LPAREN term COMMA term RPAREN
        { $$ = TermNode::TupleTerm($2, $4); }

    | LPAREN term RPAREN
        { $$ = $2; }   // group, now unambiguous
    ;

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
    // Get the line number and column range
    int line_no = l.begin.line;
    int col_start = l.begin.column;
    int col_end = l.end.column;

    std::cerr << msg << ": " << driver.filename << ":" << line_no << ":" << col_start << "-" << col_end << std::endl;

    if (line_no <= 0 || line_no > (int)driver.file.lines.size())
        return; // invalid line

    // Get the source line
    auto iter = driver.file.lines.begin();
    advance(iter, line_no - 1);
    std::string line = char_vec_to_string(*iter);
    line = strip(line); // remove leading/trailing whitespace

    // Print line number and line contents
    std::cerr << line_no << " | " << line << "\n";

    // Print caret line
    std::cerr << std::string(std::to_string(line_no).length() + 3, ' '); // align under content
    for (int i = 1; i < col_start; ++i)
        std::cerr << ' ';
    for (int i = col_start; i < col_end; ++i)
        std::cerr << '^';
    std::cerr << "\n";
}
