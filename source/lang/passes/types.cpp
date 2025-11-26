#include "../syntax.h"
#include "passes.h"

Type deref_type(Type t) {
  switch (t->kind) {
  case TypeNode::TArrow: {
    auto f = std::get<TypeNode::Arrow>(t->payload);
    return TypeNode::ArrowType(deref_type(f.param), deref_type(f.result));
  }

  case TypeNode::TTuple: {
    auto tup = std::get<TypeNode::Tuple>(t->payload);
    return TypeNode::TupleType(deref_type(tup.left), deref_type(tup.right));
  }

  case TypeNode::TVar: {
    auto opt = std::get<TypeNode::TypeVar>(t->payload);
    if (!opt.stored.has_value()) {
      throw TypeError("uninstantiated type variable detected");
    }

    Type t2 = deref_type(opt.stored.value());
    opt.stored = std::make_optional<Type>(TypeNode::gentyp(t2));
    t->payload = opt;
    return t2;
  }

  default:
    return t;
  }
}

Term deref_term(Term t) {
  switch (t->kind) {
  case TermNode::TmLet: {
    auto let = std::get<TermNode::Let>(t->payload);
    Type t2 = deref_type(let.type);
    return TermNode::LetTerm(let.name, deref_type(let.type), deref_term(let.e1),
                             deref_term(let.e2));
  }

  case TermNode::TmApp: {
    auto app = std::get<TermNode::App>(t->payload);
    return TermNode::AppTerm(deref_term(app.f), deref_term(app.arg));
  }

  case TermNode::TmTuple: {
    auto tup = std::get<TermNode::Tuple>(t->payload);
    return TermNode::AppTerm(deref_term(tup.left), deref_term(tup.right));
  }

  default:
    return t;
  }
}

bool occur(TypeNode::TypeVar r, Type t) {
  switch (t->kind) {
  case TypeNode::TArrow: {
    auto arrow = std::get<TypeNode::Arrow>(t->payload);
    return occur(r, arrow.param) || occur(r, arrow.result);
  }

  case TypeNode::TTuple: {
    auto tup = std::get<TypeNode::Tuple>(t->payload);
    return occur(r, tup.left) || occur(r, tup.right);
  }

  case TypeNode::TVar: {
    auto var = std::get<TypeNode::TypeVar>(t->payload);
    if (!var.stored.has_value())
      return false;
    if (r.stored.has_value() && r.stored.value() == var.stored.value())
      return true;
    return occur(r, var.stored.value());
  }

  default:
    return false;
  }
}

void unify(Type t1, Type t2) {
  // | Type.Var(r1), Type.Var(r2) when r1 == r2 -> ()
  if (t1->kind == TypeNode::TVar && t2->kind == TypeNode::TVar) {
    if (std::get<TypeNode::TypeVar>(t1->payload).stored ==
        std::get<TypeNode::TypeVar>(t2->payload).stored)
      return;
  }
  // | Type.Var({ contents = Some(t1') }), _ -> unify t1' t2
  if (t1->kind == TypeNode::TVar) {
    auto v1 = std::get<TypeNode::TypeVar>(t1->payload);
    if (v1.stored.has_value()) {
      unify(v1.stored.value(), t2);
      return;
    }
  }
  // | _, Type.Var({ contents = Some(t2') }) -> unify t1 t2'
  if (t2->kind == TypeNode::TVar) {
    auto v2 = std::get<TypeNode::TypeVar>(t2->payload);
    if (v2.stored.has_value()) {
      unify(t1, v2.stored.value());
      return;
    }
  }
  // | Type.Var({ contents = None } as r1), _ -> (* 一方が未定義の型変数の場合
  // (caml2html: typing_undef) *)
  //     if occur r1 t2 then raise (Unify(t1, t2));
  //     r1 := Some(t2)
  if (t1->kind == TypeNode::TVar) {
    auto v1 = std::get<TypeNode::TypeVar>(t1->payload);
    if (!v1.stored.has_value() && occur(v1, t2)) {
      throw UnifyError(t1, t2);
    } else
      t1->payload = TypeNode::TypeVar{std::make_optional<Type>(t2)};
    return;
  }
  // | _, Type.Var({ contents = None } as r2) ->
  //     if occur r2 t1 then raise (Unify(t1, t2));
  //     r2 := Some(t1)
  if (t2->kind == TypeNode::TVar) {
    auto v2 = std::get<TypeNode::TypeVar>(t2->payload);
    if (!v2.stored.has_value() && occur(v2, t1)) {
      throw UnifyError(t1, t2);
    } else
      t2->payload = TypeNode::TypeVar{std::make_optional<Type>(t1)};
    return;
  }

  if (t1->kind == t2->kind) {
    switch (t1->kind) {
    case TypeNode::TUnit:
    case TypeNode::TBool:
    case TypeNode::TInt:
    case TypeNode::TFloat:
    case TypeNode::TString:
      return;

    case TypeNode::TArrow: {
      auto f1 = std::get<TypeNode::Arrow>(t1->payload);
      auto f2 = std::get<TypeNode::Arrow>(t2->payload);

      unify(f1.param, f2.param);
      unify(f1.result, f2.result);
      return;
    }

    case TypeNode::TTuple: {
      auto tup1 = std::get<TypeNode::Tuple>(t1->payload);
      auto tup2 = std::get<TypeNode::Tuple>(t2->payload);

      unify(tup1.left, tup2.left);
      unify(tup1.right, tup2.right);
      return;
    }

    default:
      return;
    }
  }
}

Type infer(Term t, EnvType env) {
  try {
    switch (t->kind) {
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
    case TermNode::TmLet: {
      auto let = std::get<TermNode::Let>(t->payload);
      unify(let.type, infer(let.e1, env));

      EnvType env2 = env;
      env2[let.name] = let.type;
      return infer(let.e2, env2);
    }
    case TermNode::TmVar: {
      auto var = std::get<TermNode::Var>(t->payload);
      if (0 < env.count(var.name))
        return env[var.name];
      if (isPrimitive(t))
        return primitives.at(var.name).t;
      throw TypeError("infer: unexpected free variable " + var.name);
    }
    case TermNode::TmApp: {
      auto app = std::get<TermNode::App>(t->payload);

      Type t = TypeNode::gentyp();
      // std::cout << stringOfTerm(app.f) << " " << stringOfTerm(app.arg) <<
      // std::endl;
      unify(infer(app.f, env), TypeNode::ArrowType(infer(app.arg, env), t));
      return t;
    }
    case TermNode::TmAbs: {
      auto abs = std::get<TermNode::Abs>(t->payload);
      EnvType env2 = env;
      env2[abs.param] = abs.paramType;
      return TypeNode::ArrowType(abs.paramType, infer(abs.body, env2));
    }
    case TermNode::TmTuple: {
      auto tup = std::get<TermNode::Tuple>(t->payload);
      return TypeNode::TupleType(infer(tup.left, env), infer(tup.right, env));
    }
    }
  } catch (UnifyError &e) {
    throw TypeError("infer {" + stringOfTerm(t) + "} {" + stringOfType(e.t1) +
                    "} <> {" + stringOfType(e.t2) + "}");
  }
}

Term typecheck(const Term &program) {
  EnvType env;
  infer(program, env);
  return deref_term(program);
}
