#ifndef __MCSCANNER_HPP__
#define __MCSCANNER_HPP__ 1

#if !defined(yyFlexLexerOnce)
#include "FlexLexer.h"
#endif

#ifdef __3DS__
#include "parser_3ds.hpp"
#else
#include "parser.hpp"
#endif
#include "location.hh"

namespace MC {

class MC_Scanner : public yyFlexLexer {
public:
  MC_Scanner(std::istream *in) : yyFlexLexer(in) {
    loc = new MC::MC_Parser::location_type();
  };

  // get rid of override virtual function warning
  using FlexLexer::yylex;

  virtual int yylex(MC::MC_Parser::semantic_type *const lval,
                    MC::MC_Parser::location_type *location);
  // YY_DECL defined in mc_lexer.l
  // Method body created by flex in mc_lexer.yy.cc

private:
  /* yyval ptr */
  MC::MC_Parser::semantic_type *yylval = nullptr;
  /* location ptr */
  MC::MC_Parser::location_type *loc = nullptr;
};

} /* end namespace MC */

#endif /* END __MCSCANNER_HPP__ */
