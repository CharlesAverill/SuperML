#ifndef STDLIB_H
#define STDLIB_H

#include <map>
#include <string>
#include "../syntax.h"

using PrimitiveImpl = Term*(*)(Term* arg);

extern std::map<std::string, PrimitiveImpl> primitives;

void initPrimitives();

bool isPrimitive(Term* tm);

#endif
