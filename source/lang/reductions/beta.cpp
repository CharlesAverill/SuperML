#include "../syntax.h"
#include "reductions.h"

/* ============================================================
   Look up a variable in the environment
   ============================================================ */
Term lookup(const std::string& x, const Env& env) {
    auto it = env.find(x);
    return (it == env.end() ? nullptr : it->second);
}

/* ============================================================
   Substitute inside a term (capture-avoiding)
   substitute(t, x ↦ v)
   ============================================================ */
Term substitute(Term t, const std::string& x, Term v) {
    switch (t->kind) {

        case TermNode::TmVar: {
            auto &var = std::get<TermNode::Var>(t->payload);
            // _ is a special placeholder: never substitute it
            if (var.name == "_") return t;
            return (var.name == x ? v : t);
        }


        case TermNode::TmApp: {
            auto &ap = std::get<TermNode::App>(t->payload);
            return TermNode::AppTerm(
                substitute(ap.f, x, v),
                substitute(ap.arg, x, v)
            );
        }

        case TermNode::TmAbs: {
            auto &fn = std::get<TermNode::Abs>(t->payload);

            // If binder shadows x, leave unchanged
            if (fn.param == x)
                return t;

            return TermNode::AbsTerm(
                fn.param,
                fn.paramType,
                substitute(fn.body, x, v)
            );
        }

        case TermNode::TmLet: {
            auto &lt = std::get<TermNode::Let>(t->payload);

            // Shadowing: let x = ... in ...
            if (lt.name == x) {
                // substitute only in e1
                return TermNode::LetTerm(
                    lt.name,
                    lt.type,
                    substitute(lt.e1, x, v),
                    lt.e2          // unchanged
                );
            }

            return TermNode::LetTerm(
                lt.name,
                lt.type,
                substitute(lt.e1, x, v),
                substitute(lt.e2, x, v)
            );
        }

        case TermNode::TmTuple: {
            auto &tp = std::get<TermNode::Tuple>(t->payload);
            return TermNode::TupleTerm(
                substitute(tp.left, x, v),
                substitute(tp.right, x, v)
            );
        }

        default:
            return t;
    }
}

/* ============================================================
   One-step β-reduction
   ============================================================ */
Term beta_step(Term t, Env& env) {
    switch (t->kind) {

        /* ---------- Base values ---------- */
        case TermNode::TmUnit:
        case TermNode::TmBool:
        case TermNode::TmInt:
        case TermNode::TmFloat:
        case TermNode::TmString:
            return t;

        /* ---------- Variable ---------- */
        case TermNode::TmVar: {
            auto &v = std::get<TermNode::Var>(t->payload);
            Term r = lookup(v.name, env);
            return (r ? r : t);
        }

        /* ---------- Tuple ---------- */
        case TermNode::TmTuple: {
            auto &tp = std::get<TermNode::Tuple>(t->payload);
            Term l2 = beta_step(tp.left, env);
            Term r2 = beta_step(tp.right, env);
            if (l2 == tp.left && r2 == tp.right) return t;
            return TermNode::TupleTerm(l2, r2);
        }

        /* ---------- Lambda abstraction ---------- */
        case TermNode::TmAbs:
            return t;

        /* ---------- Application ---------- */
        case TermNode::TmApp: {
            auto &ap = std::get<TermNode::App>(t->payload);

            // First reduce left side
            Term lf = beta_step(ap.f, env);
            if (lf != ap.f)
                return TermNode::AppTerm(lf, ap.arg);

            // Then reduce right side
            Term arg2 = beta_step(ap.arg, env);
            if (arg2 != ap.arg)
                return TermNode::AppTerm(ap.f, arg2);

            // Beta-redex?
            if (lf->kind == TermNode::TmAbs) {
                auto &fn = std::get<TermNode::Abs>(lf->payload);

                if (fn.param == "_") {
                    // Wildcard lambda: evaluate arg, discard body
                    return TermNode::AppTerm(lf, arg2);
                } else {
                    // Regular beta-reduction
                    return substitute(fn.body, fn.param, arg2);
                }
            }

            return t;
        }

        /* ---------- Let binding ---------- */
        case TermNode::TmLet: {
            auto &lt = std::get<TermNode::Let>(t->payload);

            // First, reduce e1
            Term e1r = beta_step(lt.e1, env);

            if (lt.name == "_") {
                // Let _ = e1 in e2
                // If e1 is fully reduced, just continue with e2
                if (e1r == lt.e1) {
                    return TermNode::AppTerm(
                        TermNode::AbsTerm("_", lt.type, lt.e2),
                        e1r
                    );
                } else {
                    // e1 reduced, continue
                    return TermNode::LetTerm("_", lt.type, e1r, lt.e2);
                }
            }

            // For named bindings
            if (e1r != lt.e1)
                return TermNode::LetTerm(lt.name, lt.type, e1r, lt.e2);

            // If e1 is a value (not a var or app), substitute
            if (e1r->kind != TermNode::TmVar &&
                e1r->kind != TermNode::TmApp) {
                return substitute(lt.e2, lt.name, e1r);
            }

            // Otherwise, reduce e2 under extended environment
            Env env2 = env;
            env2[lt.name] = e1r;
            Term e2r = beta_step(lt.e2, env2);
            if (e2r == lt.e2) return t;
            return TermNode::LetTerm(lt.name, lt.type, e1r, e2r);
        }
    }

    return t;
}

Term beta(Term t) {
    Env env;
    return beta_step(t, env);
}
