#ifndef INTERPRETER
#define INTERPRETER

#include "reductions/reductions.h"

struct State {
    Term* program;
    std::string outChannel;
    unsigned long long int fuel;
};

void step(State* state);

#endif /* INTERPRETER */
