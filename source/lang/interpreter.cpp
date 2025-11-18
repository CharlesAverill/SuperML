#include "interpreter.h"

bool step(State* state) {
    if (state->fuel == 0)
        return false;

    Term old = *state->program;

    beta(state->program);
    assoc(state->program);

    if (termsEqual(&old, state->program))
        return false;

    state->fuel--;

    return true;
}

ReturnCode run(Term* program) {
    std::string outChannel = "";

    State state = {
        .program=program,
        .outChannel=outChannel,
        .fuel=UINT64_MAX
    };

    while (step(&state)) {
        stepCallback(&state);
    }

    if (state.fuel == 0) {
        return OutOfFuel;
    } else {
        return Ok;
    }
}

ReturnCode interpreterMain(std::string programText) {
    Term program = {.kind=TmUnit, .type=TUnit};
    return run(&program);
}
