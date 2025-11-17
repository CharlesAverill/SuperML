#include "interpreter.h"

void step(State* state) {
    if (state->fuel == 0)
        return;

    Term old = *state->program;

    beta(state->program);
    assoc(state->program);

    if (termsEqual(&old, state->program))
        return;

    state->fuel--;
    step(state);
}
