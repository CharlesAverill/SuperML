#ifndef REDUCTIONS
#define REDUCTIONS

#include <map>
#include "../syntax.h"
#include "../../globals.h"

typedef std::map<std::string, Term> Env;

Term substitute(Term t, const std::string& x, Term v);

Term beta(Term term);
Term assoc(Term term);
Term reduce(Term term);

#endif /* REDUCTIONS */
