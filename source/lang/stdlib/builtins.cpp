#include "stdlib.h"
#include <iostream>

bool isPrimitive(Term term) {
  return term->kind == TermNode::TmVar &&
         primitives.count(std::get<TermNode::Var>(term->payload).name) > 0;
}

Primitive print_string = {
  .f = [](Term arg) -> Term {
        std::string s = std::get<std::string>(arg->payload);
        std::cout << s;
        return TermNode::Unit();
    },
  .t = TypeNode::ArrowType(TypeNode::String(), TypeNode::Unit())
};

Primitive print_endline = {
  .f = [](Term arg) -> Term {
    std::string s = std::get<std::string>(arg->payload);
    std::cout << s << std::endl;
    return TermNode::Unit();
  },
  .t = TypeNode::ArrowType(TypeNode::String(), TypeNode::Unit())
};

Primitive print_int = {
  .f = [](Term arg) -> Term {
    int i = std::get<int>(arg->payload);
    std::cout << i;
    return TermNode::Unit();
  },
  .t = TypeNode::ArrowType(TypeNode::Int(), TypeNode::Unit())
};

Primitive print_float = {
  .f = [](Term arg) -> Term {
    float f = std::get<float>(arg->payload);
    std::cout << f;
    return TermNode::Unit();
  },
  .t = TypeNode::ArrowType(TypeNode::Float(), TypeNode::Unit())
};

Primitive print_bool = {
  .f = [](Term arg) -> Term {
    bool b = std::get<bool>(arg->payload);
    std::cout << std::boolalpha << b << std::noboolalpha;
    return TermNode::Unit();
  },
  .t = TypeNode::ArrowType(TypeNode::Bool(), TypeNode::Unit())
};

const std::array<std::pair<std::string_view, Primitive>, 5> primitive_list {{
    { "print_string",  print_string },
    { "print_endline", print_endline },
    { "print_int",     print_int },
    { "print_bool",    print_bool },
    { "print_float",   print_float },
}};

const std::unordered_map<std::string, Primitive> primitives = [] {
    std::unordered_map<std::string, Primitive> m;
    for (auto &p : primitive_list)
        m.emplace(std::string(p.first), p.second);
    return m;
}();
