#ifndef REDUCTIONS
#define REDUCTIONS

#include "../../globals.h"
#include "../syntax.h"
#include <map>

typedef std::map<std::string, Term> Env;

bool isValue(Term term);

Term substitute(Term t, const std::string &x, Term v);

/*
    beta-reduction
    --------------
    `let x = y in x + z` -> `y + z`
    `let _ = x in y` -> `(fun _ -> y) x`
*/
Term beta(Term term);

/*
    Reduction of nested-let
    -----------------------
    `let x = (let y = e1 in e2) in e3` -> `let y = e1 in let x = e2 in e3`
*/
Term assoc(Term term);
Term reduce(Term term);

#endif /* REDUCTIONS */
