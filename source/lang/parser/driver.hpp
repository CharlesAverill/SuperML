#ifndef __MCDRIVER_HPP__
#define __MCDRIVER_HPP__ 1

#include <string>
#include <cstddef>
#include <istream>

#include "scanner.hpp"

#ifdef __3DS__
#include "parser_3ds.hpp"
#else
#include "parser.hpp"
#endif

namespace MC{

class MC_Driver{
public:
   MC_Driver() = default;

   virtual ~MC_Driver();
   
   /** 
    * parse - parse from a file
    * @param filename - valid string with input file
    */
   void parse( const char *filename );
   /** 
    * parse - parse from a c++ input stream
    * @param is - std::istream&, valid input stream
    */
   void parse( std::istream &iss );

   void add_upper();
   void add_lower();
   void add_word( const std::string &word );
   void add_newline();
   void add_char();

   Term root_term;

   std::ostream& print(std::ostream &stream);
   MC::MC_Parser  *parser  = nullptr;
   MC::MC_Scanner *scanner = nullptr;
private:

   void parse_helper( std::istream &stream );

   std::size_t  chars      = 0;
   std::size_t  words      = 0;
   std::size_t  lines      = 0;
   std::size_t  uppercase  = 0;
   std::size_t  lowercase  = 0;
   
   /** define some pretty colors **/
   const std::string red   = "\033[1;31m";
   const std::string blue  = "\033[1;36m";
   const std::string norm  = "\033[0m";
};

} /* end namespace MC */
#endif /* END __MCDRIVER_HPP__ */
