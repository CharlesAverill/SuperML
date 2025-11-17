#ifndef REDUCTIONS
#define REDUCTIONS

#include <map>
#include "../syntax.h"

typedef std::map<std::string, Term*> env;

void beta(Term* term);
void assoc(Term* term);

#endif /* REDUCTIONS */
