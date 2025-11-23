#include "../syntax.h"
#include "reductions.h"

Term step(Term term) { return assoc(beta(term)); }

Term reduce(Term program) {
  Term out = program;
  DEBUG(std::cout << "START REDUCE" << std::endl);
  unsigned long long int fuel;
  for (fuel = __UINT16_MAX__; 0 < fuel; fuel--) {
    Term temp = step(out);
    if (*temp == *out)
      break;
    out = temp;
  }
  DEBUG(std::cout << "END REDUCE AFTER " << __UINT16_MAX__ - fuel << " STEPS"
                  << std::endl);
  return out;
}
