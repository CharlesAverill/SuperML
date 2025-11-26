#include "syntax.h"

// precedence: 0 = top, 1 = arrow, 2 = tuple (higher number => tighter binding)
static std::string stringOfTypeWithPrec(Type t, int prec) {
  if (!t)
    return "<none>";
  std::ostringstream out;
  switch (t->kind) {
  case TypeNode::TUnit:
    out << "unit";
    return out.str();
  case TypeNode::TBool:
    out << "bool";
    return out.str();
  case TypeNode::TInt:
    out << "int";
    return out.str();
  case TypeNode::TFloat:
    out << "float";
    return out.str();
  case TypeNode::TString:
    out << "string";
    return out.str();
  case TypeNode::TVar: {
    out << "<";
    auto var = std::get<TypeNode::TypeVar>(t->payload);
    if (var.stored.has_value())
      out << stringOfTypeWithPrec(var.stored.value(), prec);
    else
      out << "none";
    out << ">";
    return out.str();
  }

  case TypeNode::TTuple: {
    // tuple has tighter precedence than arrow
    auto const &tt = std::get<TypeNode::Tuple>(t->payload);
    std::string left = stringOfTypeWithPrec(tt.left, 2);
    std::string right = stringOfTypeWithPrec(tt.right, 2);
    std::string s = left + " * " + right;
    if (prec > 2) { // if surrounding context binds tighter, parenthesize
      out << wrap(s);
    } else {
      out << s;
    }
    return out.str();
  }

  case TypeNode::TArrow: {
    auto const &at = std::get<TypeNode::Arrow>(t->payload);
    // For arrow, print "A -> B". Right-hand side should be printed
    // with arrow-prec so that "a -> b -> c" becomes "a -> b -> c" (right assoc)
    std::string left =
        stringOfTypeWithPrec(at.param, 2); // param binds at least as tuple
    std::string right = stringOfTypeWithPrec(at.result, 1); // allow right assoc
    std::string s = left + " -> " + right;
    if (prec > 1) { // if we need to group due to outer operator, parenthesize
      out << wrap(s);
    } else {
      out << s;
    }
    return out.str();
  }

  case TypeNode::TUnknown:
    return std::get<std::string>(t->payload);
  }
}

std::string stringOfType(Type t) { return stringOfTypeWithPrec(t, 0); }

// Term pretty-printer
std::string stringOfTerm(Term t, int depth) {
  if (!t)
    return "<nil>";
  std::ostringstream out;
  out << std::string(depth, ' ');
  switch (t->kind) {
  case TermNode::TmUnit:
    out << "()";
    break;

  case TermNode::TmBool:
    out << (std::get<bool>(t->payload) ? "true" : "false");
    break;

  case TermNode::TmInt:
    out << std::get<int>(t->payload);
    break;

  case TermNode::TmFloat:
    out << std::get<double>(t->payload);
    break;

  case TermNode::TmString:
    out << "\"" << std::get<std::string>(t->payload) << "\"";
    break;

  case TermNode::TmVar: {
    auto const &vn = std::get<TermNode::Var>(t->payload);
    out << vn.name;
    break;
  }

  case TermNode::TmTuple: {
    auto const &tp = std::get<TermNode::Tuple>(t->payload);
    out << wrap(stringOfTerm(tp.left) + ", " + stringOfTerm(tp.right));
    break;
  }

  case TermNode::TmLet: {
    auto const &lt = std::get<TermNode::Let>(t->payload);
    out << "let " + wrap(lt.name + " : " + stringOfType(lt.type)) << " =\n"
        << std::string(depth + 1, ' ') << wrap(stringOfTerm(lt.e1, 0))
        << " in\n"
        << wrap(stringOfTerm(lt.e2, depth));
    break;
  }

  case TermNode::TmAbs: {
    auto const &fn = std::get<TermNode::Abs>(t->payload);
    out << wrap("fun " + wrap(fn.param + " : " + stringOfType(fn.paramType)) +
                " -> " + stringOfTerm(fn.body, 0));
    break;
  }

  case TermNode::TmApp: {
    auto const &ap = std::get<TermNode::App>(t->payload);
    out << wrap(stringOfTerm(ap.f, 0) + " " + stringOfTerm(ap.arg, 0));
    break;
  }
  }
  return out.str();
}
