#ifndef INTERPRETER
#define INTERPRETER

#include "reductions/reductions.h"

struct State {
    Term* program;
    std::string outChannel;
    unsigned long long int fuel;
};

enum ReturnCode {
    Ok,
    OutOfFuel
};

bool step(State* state);
ReturnCode run(Term* program);
void stepCallback(State* state);

ReturnCode interpreterMain(std::string programText);

#endif /* INTERPRETER */
