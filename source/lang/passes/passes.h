#ifndef REDUCTIONS
#define REDUCTIONS

#include "../../globals.h"
#include "../stdlib/stdlib.h"
#include "../syntax.h"
#include <functional>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

typedef std::unordered_map<std::string, Term> Env;

bool isValue(Term term);

Term substitute(Term t, const std::string &x, Term v);

/*
    primitive argument rewriting
    `<primitive> a b c d ...` -> `primitive (a, (b, (c, (d, ...))))`
*/
Term primitiveArgs(Term t);

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

// Perform all reduction passes
Term reduce(Term term);

// Errors
struct TypeError : public std::runtime_error {
  TypeError(const std::string &msg) : std::runtime_error(msg) {}
};

struct UnifyError : public std::runtime_error {
  Type t1, t2;
  UnifyError(Type &_t1, Type &_t2)
      : std::runtime_error("unify: " + stringOfType(_t1) + " <> " +
                           stringOfType(_t2)) {
    t1 = _t1;
    t2 = _t2;
  }
};

// Substitution mapping: maps unknown-TypeNode pointer -> Type
// (we key by pointer identity of TypeNode representing unknown type variables)
using Subst = std::unordered_map<const TypeNode *, Type>;

// Helper: an environment mapping variable names to Types
using EnvType = std::unordered_map<std::string, Type>;

// Type check and infer types for the program
Term typecheck(const Term &program);

#endif /* REDUCTIONS */
