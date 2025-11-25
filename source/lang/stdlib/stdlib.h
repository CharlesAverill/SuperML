#ifndef STDLIB_H
#define STDLIB_H

#include "../../globals.h"
#include "../syntax.h"
#include <cmath>
#include <iomanip>
#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

#ifdef __3DS__
#include "../../ui.h"
#endif

using PrimitiveFunc = Term (*)(Term arg);

typedef struct Primitive {
  PrimitiveFunc f;
  Type t;
} Primitive;

extern const std::vector<std::pair<std::string_view, Primitive>> primitive_list;
extern const std::unordered_map<std::string, Primitive> primitives;

bool isPrimitive(Term tm);

#endif
