#include "stdlib.h"
#include <iostream>

std::map<std::string, PrimitiveImpl> primitives;

Term* print_endline(Term* arg) {
    std::cout << *arg->value.stringValue << std::endl;
    return new Term(makeUnit());
}

bool isPrimitive(Term* term) {
    return term->kind == TmVar && primitives.count(term->value.varValue->name) > 0;
}

void initPrimitives() {
    primitives["print_endline"] = print_endline;
}
