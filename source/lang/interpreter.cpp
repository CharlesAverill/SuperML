#include "interpreter.h"
#include "parser/driver.hpp"
#include "syntax.h"
#include <fstream>
#include <iostream>
#include <optional>

bool isValue(Term term) {
  switch (term->kind) {
  case TermNode::TmApp:
    return false;
  default:
    return true;
  }
}

std::optional<std::pair<Term, State>> step(const Term &program,
                                           const State &state) {

  switch (program->kind) {

  case TermNode::TmApp: {
    const auto &ap = std::get<TermNode::App>(program->payload);
    Term fun = ap.f;
    Term arg = ap.arg;

    /* ----------------------------------------
       Step function position
       ---------------------------------------- */
    if (!isValue(fun)) {
      auto r = step(fun, state);
      if (!r)
        return std::nullopt;

      auto &[fun2, state2] = *r;
      Term newProgram = TermNode::AppTerm(fun2, arg);
      return std::make_optional(std::make_pair(newProgram, state2));
    }

    /* ----------------------------------------
       Step argument position
       ---------------------------------------- */
    if (!isValue(arg)) {
      auto r = step(arg, state);
      if (!r)
        return std::nullopt;

      auto &[arg2, state2] = *r;
      Term newProgram = TermNode::AppTerm(fun, arg2);
      return std::make_optional(std::make_pair(newProgram, state2));
    }

    /* ----------------------------------------
       Now both fun and arg are values
       ---------------------------------------- */

    // Case 1: primitive function
    if (fun->kind == TermNode::TmVar && isPrimitive(fun)) {
      auto &v = std::get<TermNode::Var>(fun->payload);
      auto prim = primitives.find(v.name);

      if (prim != primitives.end()) {
        Term newTerm = prim->second.f(arg); // pure
        return std::make_optional(std::make_pair(newTerm, state));
      }
    }

    // Case 2: lambda application:  (fun x -> body) arg -> body[x := arg]
    if (fun->kind == TermNode::TmAbs) {
      const auto &abs = std::get<TermNode::Abs>(fun->payload);
      Term newTerm = substitute(abs.body, abs.param, arg);
      return std::make_optional(std::make_pair(newTerm, state));
    }

    return std::nullopt;
  }

  default:
    return std::nullopt;
  }
}

#ifdef __3DS__
#include "../Notepad3DS/source/display.h"
#endif

void interpreterMain(std::string filename) {
  DO_3DS(status_message("Parsing..."); consoleSelect(&topScreen));
  MC::MC_Driver driver;
  if (driver.parse(filename.c_str())) {
    return;
  }
  Term prog = driver.root_term;

  DEBUG(std::cout << "PARSED:\n" << stringOfTerm(prog) << std::endl);

  try {
    prog = typecheck(prog);
  } catch (TypeError &e) {
    std::cerr << e.what() << std::endl;
    return;
  }

  DO_3DS(status_message("Reducing..."));
  prog = reduce(prog);

  DEBUG(std::cout << "REDUCED:\n" << stringOfTerm(prog) << std::endl);

  std::string outChannel;
  Env emptyEnv;
  State state = {.outChannel = outChannel, .env = emptyEnv};

  DO_3DS(status_message("Interpreting..."); clear_top_screen(););
  DEBUG(std::cout << "START INTERPRET\n==================" << std::endl);

  while (true) {
    auto result = step(prog, state);
    if (!result)
      break;
    auto [nextTerm, nextState] = *result;
    prog = nextTerm;
    state = nextState;

    stepCallback(state);
  }

  DO_3DS(status_message("Done!"));
  DEBUG(std::cout << "\n==================\nEND INTERPRET" << std::endl);
}
