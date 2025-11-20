%{
#include <string>
#include <cstring>
#include <cstdlib>

/* include the bison-generated header so token enum and semantic API are visible */
#include "scanner.hpp"

#undef  YY_DECL
#define YY_DECL int MC::MC_Scanner::yylex( MC::MC_Parser::semantic_type * const lval, MC::MC_Parser::location_type *location )

using token = MC::MC_Parser::token;

/* update location on matching (parser passes 'location' pointer) */
#define YY_USER_ACTION if(location) { location->step(); location->columns(yyleng); }

/* helper to build values into the parser semantic variant */
#define BUILD_INT(v)    lval->build<int>(v)
#define BUILD_FLOAT(v)  lval->build<float>(v)
#define BUILD_STR(s)    lval->build< std::string >(s)

%}

%option c++
%option noyywrap
%option nodefault
%option yyclass="MC::MC_Scanner"

%%

[ \t\r]+                { /* skip whitespace */ }
\n                      { if (location) location->lines(); /* skip newline */ }

"(*"                    { /* simple comment: skip until "*)" */
                          int depth = 1;
                          while (depth > 0) {
                              int c = yyinput();
                              if (c == EOF) break;
                              if (c == '(') {
                                  int c2 = yyinput();
                                  if (c2 == '*') depth++;
                                  else unput(c2);
                              } else if (c == '*') {
                                  int c2 = yyinput();
                                  if (c2 == ')') depth--;
                                  else unput(c2);
                              }
                          }
                          continue;
                        }

"("                     { return token::LPAREN; }
")"                     { return token::RPAREN; }
","                     { return token::COMMA; }
":"                     { return token::COLON; }
"*"                     { return token::STAR; }
"->"                    { return token::ARROW; }

"let"                   { return token::LET; }
"in"                    { return token::IN; }
"fun"                   { return token::FUN; }

[0-9]+"."[0-9]*([eE][+-]?[0-9]+)?   {
                          BUILD_FLOAT(atof(yytext));
                          return token::FLOATLIT;
                        }
[0-9]+([eE][+-]?[0-9]+)?             {
                          BUILD_INT(atoi(yytext));
                          return token::INTLIT;
                        }

\"([^"\\]|\\.)*\"       {
                          std::string s(yytext+1, yyleng-2);
                          BUILD_STR(s);
                          return token::STRINGLIT;
                        }

[a-zA-Z_][a-zA-Z0-9_]*  {
                          BUILD_STR(std::string(yytext));
                          return token::ID;
                        }

.                       { /* ignore unknown */ }

%%
