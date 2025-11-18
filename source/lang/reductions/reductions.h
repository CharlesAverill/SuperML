#ifndef REDUCTIONS
#define REDUCTIONS

#include <map>
#include "../syntax.h"

typedef std::map<std::string, Term*> Env;

void beta(Term* term);
void assoc(Term* term);
void reduce(Term* term);

#endif /* REDUCTIONS */
