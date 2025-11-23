#ifndef INTERPRETER
#define INTERPRETER

#include "reductions/reductions.h"
#include "../Notepad3DS/source/file.h"
#include "stdlib/stdlib.h"
#include "../globals.h"

struct State {
    std::string outChannel;
    Env env;
};

enum ReturnCode {
    Ok,
    OutOfFuel
};

void stepCallback(State state);
void interpreterMain(std::string filename);

#endif /* INTERPRETER */
