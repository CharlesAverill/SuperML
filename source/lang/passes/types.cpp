#include "../syntax.h"
#include "passes.h"

Type fresh_type_var() { return TypeNode::Unknown(); }

// dereference a type via substitution map
Type deref(Type t, Subst &S) {
  if (!t)
    return t;
  // follow substitution chain: while t is an unknown and there's an entry in S
  const TypeNode *p = t.get();
  auto it = S.find(p);
  if (it == S.end())
    return t;
  // path compression
  Type r = deref(it->second, S);
  S[p] = r;
  return r;
}

// occurs-check: ensure varp does not occur inside type t (after following S)
void occur_check(const TypeNode *varp, const Type &t, Subst &S) {
  Type td = deref(t, S);
  if (!td)
    return;
  if (td.get() == varp) {
    throw TypeError("occurs check failed: creating infinite type");
  }
  switch (td->kind) {
  case TypeNode::TTuple: {
    auto &tuple = std::get<TypeNode::Tuple>(td->payload);
    occur_check(varp, tuple.left, S);
    occur_check(varp, tuple.right, S);
    break;
  }
  case TypeNode::TArrow: {
    auto &arrow = std::get<TypeNode::Arrow>(td->payload);
    occur_check(varp, arrow.param, S);
    occur_check(varp, arrow.result, S);
    break;
  }
  default:
    break;
  }
}

// unify two types (modifies S)
void unify(Type a, Type b, Subst &S) {
  Type A = deref(a, S);
  Type B = deref(b, S);

  // pointer identity check (fast path)
  if (*A == *B)
    return;

  // if either is unknown var, bind it
  if (A->kind == TypeNode::TUnknown) {
    const TypeNode *varp = A.get();
    occur_check(varp, B, S);
    S[varp] = B;
    return;
  }
  if (B->kind == TypeNode::TUnknown) {
    const TypeNode *varp = B.get();
    occur_check(varp, A, S);
    S[varp] = A;
    return;
  }

  // both concrete kinds: must match and unify components
  if (A->kind != B->kind) {
    throw TypeError("unify failure: " + stringOfType(A) + " <> " +
                    stringOfType(B));
  }

  switch (A->kind) {
  case TypeNode::TUnit:
  case TypeNode::TBool:
  case TypeNode::TInt:
  case TypeNode::TFloat:
  case TypeNode::TString:
    // same kind, nothing else to do
    return;
  case TypeNode::TTuple: {
    auto &At = std::get<TypeNode::Tuple>(A->payload);
    auto &Bt = std::get<TypeNode::Tuple>(B->payload);
    unify(At.left, Bt.left, S);
    unify(At.right, Bt.right, S);
    return;
  }
  case TypeNode::TArrow: {
    auto &At = std::get<TypeNode::Arrow>(A->payload);
    auto &Bt = std::get<TypeNode::Arrow>(B->payload);
    unify(At.param, Bt.param, S);
    unify(At.result, Bt.result, S);
    return;
  }
  case TypeNode::TUnknown: {
    throw TypeError("Unification failure: unreachable TUnknown branch");
  }
  }
}

// Get an inferred Type for a Term
Type infer(const EnvType &env, Term term, Subst &S) {
  if (!term)
    throw TypeError("infer: null term");

  switch (term->kind) {
  case TermNode::TmUnit:
    return TypeNode::Unit();

  case TermNode::TmBool:
    return TypeNode::Bool();

  case TermNode::TmInt:
    return TypeNode::Int();

  case TermNode::TmFloat:
    return TypeNode::Float();

  case TermNode::TmString:
    return TypeNode::String();

  case TermNode::TmVar: {
    auto &vn = std::get<TermNode::Var>(term->payload);
    auto it = env.find(vn.name);
    if (it != env.end())
      return it->second;

    // Check for primitives
    if (isPrimitive(term)) {
      auto it = primitives.find(vn.name);
      if (it != primitives.end())
        return it->second.t;
    }

    // Unknown variable type
    Type tv = fresh_type_var();
    return tv;
  }

  case TermNode::TmTuple: {
    auto &tp = std::get<TermNode::Tuple>(term->payload);
    Type l = infer(env, tp.left, S);
    Type r = infer(env, tp.right, S);
    return TypeNode::TupleType(deref(l, S), deref(r, S));
  }

  case TermNode::TmApp: {
    auto &ap = std::get<TermNode::App>(term->payload);
    Type tf = infer(env, ap.f, S);
    Type ta = infer(env, ap.arg, S);
    Type tr = fresh_type_var();
    // unify tf with arrow type (ta -> tr)
    Type expectedFun = TypeNode::ArrowType(ta, tr);
    unify(tf, expectedFun, S);
    return deref(tr, S);
  }

  case TermNode::TmAbs: {
    auto &fn = std::get<TermNode::Abs>(term->payload);
    // param type: if user annotated, use it; otherwise fresh var
    Type paramT = fn.paramType ? fn.paramType : fresh_type_var();
    // Extend env and infer body
    EnvType env2 = env;
    env2[fn.param] = paramT;
    Type bodyT = infer(env2, fn.body, S);
    return TypeNode::ArrowType(deref(paramT, S), deref(bodyT, S));
  }

  case TermNode::TmLet: {
    auto &lt = std::get<TermNode::Let>(term->payload);
    // let x : annotatedType = e1 in e2
    Type t_annot = lt.type; // may be nullptr (i.e. unknown)
    Type t1 = infer(env, lt.e1, S);
    if (t_annot) {
      // This is load bearing wtf
      // Remove it and 3ds target will start giving unification errors
      std::string x = stringOfTerm(lt.e1);
      // unify annotated type with inferred type
      unify(t_annot, t1, S);
    }
    // extend env with the annotated type if present else inferred t1
    EnvType env2 = env;
    env2[lt.name] = t_annot ? deref(t_annot, S) : deref(t1, S);
    Type t2 = infer(env2, lt.e2, S);
    return deref(t2, S);
  }

  default:
    throw TypeError("Inference error: unhandled term kind");
  }
}

// Rebuild the term tree and replace Term->type fields by deref(type,S)
Term annotate_term_with_types(const Term &term, Subst &S) {
  if (!term)
    return Term();

  Type new_type = term->type ? deref(term->type, S) : TypeNode::Unknown();

  switch (term->kind) {
  case TermNode::TmUnit:
    return TermNode::Unit(); // Unit already has correct type

  case TermNode::TmBool:
    return TermNode::Bool(std::get<bool>(term->payload));

  case TermNode::TmInt:
    return TermNode::Int(std::get<int>(term->payload));

  case TermNode::TmFloat:
    return TermNode::Float(std::get<double>(term->payload));

  case TermNode::TmString:
    return TermNode::String(std::get<std::string>(term->payload));

  case TermNode::TmVar: {
    auto &v = std::get<TermNode::Var>(term->payload);
    Term nt = TermNode::VarTerm(v.name, v.index, new_type);
    return nt;
  }

  case TermNode::TmTuple: {
    auto &tp = std::get<TermNode::Tuple>(term->payload);
    Term l = annotate_term_with_types(tp.left, S);
    Term r = annotate_term_with_types(tp.right, S);
    Term nt = TermNode::TupleTerm(l, r);
    // set its type to the deref'd type from before
    // recreate with proper type if needed
    return nt;
  }

  case TermNode::TmAbs: {
    auto &fn = std::get<TermNode::Abs>(term->payload);
    Term body2 = annotate_term_with_types(fn.body, S);
    Type pty = fn.paramType ? deref(fn.paramType, S) : fresh_type_var();
    Term nt = TermNode::AbsTerm(fn.param, pty, body2);
    return nt;
  }

  case TermNode::TmApp: {
    auto &ap = std::get<TermNode::App>(term->payload);
    Term f2 = annotate_term_with_types(ap.f, S);
    Term a2 = annotate_term_with_types(ap.arg, S);
    Term nt = TermNode::AppTerm(f2, a2);
    return nt;
  }

  case TermNode::TmLet: {
    auto &lt = std::get<TermNode::Let>(term->payload);
    Term e1n = annotate_term_with_types(lt.e1, S);
    Term e2n = annotate_term_with_types(lt.e2, S);
    Type ann_t = deref(lt.type, S);
    Term nt = TermNode::LetTerm(lt.name, ann_t, e1n, e2n);
    return nt;
  }

  default:
    throw TypeError("annotate: unhandled term kind");
  }
}

// run inference and returns an annotated term (or throws TypeError)
Term typecheck(const Term &program) {
  Subst S;
  EnvType env;

  Type t = infer(env, program, S);
  // ensure top-level is unit (like min-caml)
  try {
    unify(t, TypeNode::Unit(), S);
  } catch (TypeError &e) {
    throw TypeError(std::string("Top level does not have type unit: ") +
                    e.what());
  }

  Term annotated = annotate_term_with_types(program, S);
  return annotated;
}
