#include "../syntax.h"
#include "reductions.h"

Term step(Term term) {
    return assoc(beta(term));
}

Term reduce(Term program) {
    Term out = program;
    DEBUG(std::cout << "START REDUCE" << std::endl);
    for(unsigned long long int fuel = __UINT16_MAX__; 0 < fuel; fuel--) {
        Term temp = step(out);
        if (termsEqual(temp, out))
            break;
        out = temp;
    }
    DEBUG(std::cout << "END REDUCE" << std::endl);
    return out;
}
