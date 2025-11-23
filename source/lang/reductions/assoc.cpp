#include "reductions.h"

inline const TermNode::Let &asLet(const Term &t) {
  return std::get<TermNode::Let>(t->payload);
}

inline const TermNode::Tuple &asTuple(const Term &t) {
  return std::get<TermNode::Tuple>(t->payload);
}

inline const TermNode::Abs &asAbs(const Term &t) {
  return std::get<TermNode::Abs>(t->payload);
}

inline const TermNode::App &asApp(const Term &t) {
  return std::get<TermNode::App>(t->payload);
}

Term insert(const std::string &name, Type type, Term body, Term continuation) {
  switch (body->kind) {

  case TermNode::TmLet: {
    const auto &inner = asLet(body);
    Term newE2 = insert(name, type, inner.e2, continuation);

    return TermNode::LetTerm(inner.name, inner.type, inner.e1, newE2);
  }

  default:
    // Base case: body is not a let — create `let name = body in continuation`
    return TermNode::LetTerm(name, type, body, continuation);
  }
}

Term assoc(Term term) {
  switch (term->kind) {

  // -------------------------
  // Values stay unchanged
  // -------------------------
  case TermNode::TmUnit:
  case TermNode::TmBool:
  case TermNode::TmInt:
  case TermNode::TmFloat:
  case TermNode::TmString:
  case TermNode::TmVar:
    return term;

  // -------------------------
  // Tuple
  // -------------------------
  case TermNode::TmTuple: {
    const auto &tup = asTuple(term);
    return TermNode::TupleTerm(assoc(tup.left), assoc(tup.right));
  }

  // -------------------------
  // Abstraction
  // -------------------------
  case TermNode::TmAbs: {
    const auto &abs = asAbs(term);
    return TermNode::AbsTerm(abs.param, abs.paramType, assoc(abs.body));
  }

  // -------------------------
  // Application
  // -------------------------
  case TermNode::TmApp: {
    const auto &app = asApp(term);
    return TermNode::AppTerm(assoc(app.f), assoc(app.arg));
  }

  // -------------------------
  // Let
  // -------------------------
  case TermNode::TmLet: {
    const auto &let = asLet(term);

    Term e1p = assoc(let.e1);
    Term e2p = assoc(let.e2);

    // Insert deeply nested lets
    return insert(let.name, let.type, e1p, e2p);
  }

  default:
    throw std::runtime_error("assoc: unknown term kind");
  }
}
