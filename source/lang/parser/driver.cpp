#include <cctype>
#include <fstream>
#include <cassert>

#include "driver.hpp"

MC::MC_Driver::~MC_Driver()
{
   delete(scanner);
   scanner = nullptr;
   delete(parser);
   parser = nullptr;
}

int 
MC::MC_Driver::parse( const char * const filename_cstr )
{
   /**
    * Remember, if you want to have checks in release mode
    * then this needs to be an if statement 
    */
   assert( filename_cstr != nullptr );
   std::ifstream in_file( filename_cstr );
   if( ! in_file.good() )
   {
       return 1;
   }

   std::string fn = filename_cstr;
   file = open_file(fn);

   filename = filename_cstr;

   return parse_helper( in_file );
}

int
MC::MC_Driver::parse( std::istream &stream )
{
   if( ! stream.good()  && stream.eof() )
   {
       return 1;
   }
   return parse_helper( stream ); 
}


int 
MC::MC_Driver::parse_helper( std::istream &stream )
{
   
   delete(scanner);
   try
   {
      scanner = new MC::MC_Scanner( &stream );
   }
   catch( std::bad_alloc &ba )
   {
      std::cerr << "Failed to allocate scanner: (" <<
         ba.what() << "), exiting!!\n";
      exit( EXIT_FAILURE );
   }
   
   delete(parser); 
   try
   {
      parser = new MC::MC_Parser( (*scanner) /* scanner */, 
                                  (*this) /* driver */ );
   }
   catch( std::bad_alloc &ba )
   {
      std::cerr << "Failed to allocate parser: (" << 
         ba.what() << "), exiting!!\n";
      exit( EXIT_FAILURE );
   }
   return parser->parse();
}

void 
MC::MC_Driver::add_upper()
{ 
   uppercase++; 
   chars++; 
   words++; 
}

void 
MC::MC_Driver::add_lower()
{ 
   lowercase++; 
   chars++; 
   words++; 
}

void 
MC::MC_Driver::add_word( const std::string &word )
{
   words++; 
   chars += word.length();
   for(const char &c : word ){
      if( islower( c ) )
      { 
         lowercase++; 
      }
      else if ( isupper( c ) ) 
      { 
         uppercase++; 
      }
   }
}

void 
MC::MC_Driver::add_newline()
{ 
   lines++; 
   chars++; 
}

void 
MC::MC_Driver::add_char()
{ 
   chars++; 
}


std::ostream& 
MC::MC_Driver::print( std::ostream &stream )
{
   /** NOTE: Colors are defined as class variables w/in MC_Driver **/
   stream << red  << "Results: " << norm << "\n";
   stream << blue << "Uppercase: " << norm << uppercase << "\n";
   stream << blue << "Lowercase: " << norm << lowercase << "\n";
   stream << blue << "Lines: " << norm << lines << "\n";
   stream << blue << "Words: " << norm << words << "\n";
   stream << blue << "Characters: " << norm << chars << "\n";
   return(stream);
}
