#ifndef STDLIB_H
#define STDLIB_H

#include "../syntax.h"
#include <map>
#include <string>

using PrimitiveImpl = Term (*)(Term arg);

extern std::map<std::string, PrimitiveImpl> primitives;

void initPrimitives();

bool isPrimitive(Term tm);

#endif
