#include "stdlib.h"
#include <iostream>

std::map<std::string, PrimitiveImpl> primitives;

Term* print_string(Term* arg) {
    std::cout << *arg->value.stringValue;
    return new Term(makeUnit());
}

Term* print_endline(Term* arg) {
    std::cout << *arg->value.stringValue << std::endl;
    return new Term(makeUnit());
}

Term* print_int(Term* arg) {
    std::cout << arg->value.intValue;
    return new Term(makeUnit());
}

bool isPrimitive(Term* term) {
    return term->kind == TmVar && primitives.count(term->value.varValue->name) > 0;
}

void initPrimitives() {
    primitives["print_string"] = print_string;
    primitives["print_endline"] = print_endline;
    primitives["print_int"] = print_int;
}
