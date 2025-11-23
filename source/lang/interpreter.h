#ifndef INTERPRETER
#define INTERPRETER

#include "../Notepad3DS/source/file.h"
#include "../globals.h"
#include "reductions/reductions.h"
#include "stdlib/stdlib.h"

struct State {
  std::string outChannel;
  Env env;
};

enum ReturnCode { Ok, OutOfFuel };

void stepCallback(State state);
void interpreterMain(std::string filename);

#endif /* INTERPRETER */
