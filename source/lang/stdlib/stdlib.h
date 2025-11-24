#ifndef STDLIB_H
#define STDLIB_H

#include "../syntax.h"
#include <unordered_map>
#include <array>
#include <string>

using PrimitiveFunc = Term (*)(Term arg);

typedef struct Primitive {
    PrimitiveFunc f;
    Type t;
} Primitive;

extern const std::array<std::pair<std::string_view, Primitive>, 5> primitive_list;
extern const std::unordered_map<std::string, Primitive> primitives;

bool isPrimitive(Term tm);

#endif
