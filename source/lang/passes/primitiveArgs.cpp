#include "passes.h"

std::vector<Type> typeListOfTuple(TypeNode::Tuple t) {
  std::vector<Type> out = {};
  while (true) {
    out.push_back(t.left);
    if (t.right->kind != TypeNode::TTuple) {
      out.push_back(t.right);
      break;
    }
    t = std::get<TypeNode::Tuple>(t.right->payload);
  }
  return out;
}

Term primitiveArgs(Term t) {
  switch (t->kind) {
  case TermNode::TmVar: {
    if (!isPrimitive(t))
      return t;

    auto var = std::get<TermNode::Var>(t->payload);
    Type inType = primitives.at(var.name).t;
    if (inType->kind != TypeNode::TArrow)
      return t;
    inType = std::get<TypeNode::Arrow>(inType->payload).param;

    // 1. Decompose type into a vector
    std::vector<Type> types;
    if (inType->kind == TypeNode::TTuple) {
      types = typeListOfTuple(std::get<TypeNode::Tuple>(inType->payload));
    } else {
      types.push_back(inType);
    }

    // 2. Generate fresh variable names
    std::vector<std::string> names;
    for (size_t i = 0; i < types.size(); i++)
      names.push_back("_arg" + std::to_string(i));

    // 3. Build a tuple of VarTerms
    Term tupleArgs = TermNode::VarTerm(names[0], -1, types[0]);
    for (size_t i = 1; i < types.size(); i++) {
      tupleArgs = TermNode::TupleTerm(
          tupleArgs, TermNode::VarTerm(names[i], -1, types[i]));
    }

    // 4. Apply primitive to tuple of arguments
    Term body = TermNode::AppTerm(t, tupleArgs);

    // 5. Wrap in one lambda per argument
    for (size_t i = types.size(); i-- > 0;) {
      body = TermNode::AbsTerm(names[i], types[i], body);
    }

    return body;
  }

  case TermNode::TmAbs: {
    auto abs = std::get<TermNode::Abs>(t->payload);
    return TermNode::AbsTerm(abs.param, abs.paramType, primitiveArgs(abs.body));
  }

  case TermNode::TmApp: {
    auto app = std::get<TermNode::App>(t->payload);
    return TermNode::AppTerm(primitiveArgs(app.f), primitiveArgs(app.arg));
  }

  case TermNode::TmTuple: {
    auto tup = std::get<TermNode::Tuple>(t->payload);
    return TermNode::TupleTerm(primitiveArgs(tup.left),
                               primitiveArgs(tup.right));
  }

  case TermNode::TmLet: {
    auto let = std::get<TermNode::Let>(t->payload);
    return TermNode::LetTerm(let.name, let.type, primitiveArgs(let.e1),
                             primitiveArgs(let.e2));
  }

  default:
    return t;
  }
}
