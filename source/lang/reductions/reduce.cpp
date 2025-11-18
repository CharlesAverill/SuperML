#include "reductions.h"

bool step(Term* term) {
    Term old = *term;

    beta(term);
    assoc(term);

    return termsEqual(&old, term);
}

void reduce(Term* program) {
    unsigned long long int fuel = __UINT16_MAX__;

    while (fuel && step(program)) {
        fuel--;
    }
}
