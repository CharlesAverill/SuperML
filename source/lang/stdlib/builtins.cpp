#include "stdlib.h"
#include <iostream>

std::map<std::string, PrimitiveImpl> primitives;

Term print_string(Term arg) {
    const std::string& s = std::get<std::string>(arg->payload);
    std::cout << s;
    return TermNode::Unit();
}

Term print_endline(Term arg) {
    const std::string& s = std::get<std::string>(arg->payload);
    std::cout << s << std::endl;
    return TermNode::Unit();
}

Term print_int(Term arg) {
    int i = std::get<int>(arg->payload);
    std::cout << i;
    return TermNode::Unit();
}

Term print_bool(Term arg) {
    bool b = std::get<bool>(arg->payload);
    std::cout << std::boolalpha << b << std::noboolalpha;
    return TermNode::Unit();
}

bool isPrimitive(Term term) {
    return term->kind == TermNode::TmVar && primitives.count(std::get<TermNode::Var>(term->payload).name) > 0;
}

void initPrimitives() {
    primitives["print_string"] = print_string;
    primitives["print_endline"] = print_endline;
    primitives["print_int"] = print_int;
    primitives["print_bool"] = print_bool;
}
