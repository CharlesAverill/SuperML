#include "syntax.h"

bool typeEqual(Type a, Type b) {
    if (a == b) return true;
    if (!a || !b) return false;
    if (a->kind != b->kind) return false;

    switch (a->kind) {
        case TypeNode::TTuple: {
            auto &A = std::get<TypeNode::Tuple>(a->payload);
            auto &B = std::get<TypeNode::Tuple>(b->payload);
            return typeEqual(A.left, B.left) && typeEqual(A.right, B.right);
        }
        case TypeNode::TArrow: {
            auto &A = std::get<TypeNode::Arrow>(a->payload);
            auto &B = std::get<TypeNode::Arrow>(b->payload);
            return typeEqual(A.param, B.param) && typeEqual(A.result, B.result);
        }
        default:
            return true;
    }
}

bool termsEqual(Term a, Term b) {
    if (a == b) return true;
    if (!a || !b) return false;
    if (a->kind != b->kind) return false;

    switch (a->kind) {
        case TermNode::TmUnit: return true;
        case TermNode::TmBool: return std::get<bool>(a->payload) == std::get<bool>(b->payload);
        case TermNode::TmInt: return std::get<int>(a->payload) == std::get<int>(b->payload);
        case TermNode::TmFloat: return std::get<double>(a->payload) == std::get<double>(b->payload);
        case TermNode::TmString: return std::get<std::string>(a->payload) == std::get<std::string>(b->payload);

        case TermNode::TmVar: {
            auto &A = std::get<TermNode::Var>(a->payload);
            auto &B = std::get<TermNode::Var>(b->payload);
            return A.name == B.name && A.index == B.index;
        }

        case TermNode::TmTuple: {
            auto &A = std::get<TermNode::Tuple>(a->payload);
            auto &B = std::get<TermNode::Tuple>(b->payload);
            return termsEqual(A.left, B.left) && termsEqual(A.right, B.right);
        }

        case TermNode::TmLet: {
            auto &A = std::get<TermNode::Let>(a->payload);
            auto &B = std::get<TermNode::Let>(b->payload);
            return A.name == B.name &&
                   typeEqual(A.type, B.type) &&
                   termsEqual(A.e1, B.e1) &&
                   termsEqual(A.e2, B.e2);
        }

        case TermNode::TmAbs: {
            auto &A = std::get<TermNode::Abs>(a->payload);
            auto &B = std::get<TermNode::Abs>(b->payload);
            return A.param == B.param &&
                   typeEqual(A.paramType, B.paramType) &&
                   termsEqual(A.body, B.body);
        }

        case TermNode::TmApp: {
            auto &A = std::get<TermNode::App>(a->payload);
            auto &B = std::get<TermNode::App>(b->payload);
            return termsEqual(A.f, B.f) && termsEqual(A.arg, B.arg);
        }
    }

    return false;
}

#include <sstream>

// precedence: 0 = top, 1 = arrow, 2 = tuple (higher number => tighter binding)
static std::string stringOfTypeWithPrec(Type t, int prec) {
    if (!t) return "<none>";
    std::ostringstream out;
    switch (t->kind) {
        case TypeNode::TUnit:  out << "unit"; return out.str();
        case TypeNode::TBool:  out << "bool"; return out.str();
        case TypeNode::TInt:   out << "int";  return out.str();
        case TypeNode::TFloat: out << "float"; return out.str();
        case TypeNode::TString:out << "string"; return out.str();

        case TypeNode::TTuple: {
            // tuple has tighter precedence than arrow
            auto const &tt = std::get<TypeNode::Tuple>(t->payload);
            std::string left  = stringOfTypeWithPrec(tt.left, 2);
            std::string right = stringOfTypeWithPrec(tt.right, 2);
            std::string s = left + " * " + right;
            if (prec > 2) { // if surrounding context binds tighter, parenthesize
                out << "(" << s << ")";
            } else {
                out << s;
            }
            return out.str();
        }

        case TypeNode::TArrow: {
            auto const &at = std::get<TypeNode::Arrow>(t->payload);
            // For arrow, print "A -> B". Right-hand side should be printed
            // with arrow-prec so that "a -> b -> c" becomes "a -> b -> c" (right assoc)
            std::string left  = stringOfTypeWithPrec(at.param, 2);   // param binds at least as tuple
            std::string right = stringOfTypeWithPrec(at.result, 1);  // allow right assoc
            std::string s = left + " -> " + right;
            if (prec > 1) { // if we need to group due to outer operator, parenthesize
                out << "(" << s << ")";
            } else {
                out << s;
            }
            return out.str();
        }

        default:
            return "<unknown>";
    }
}

std::string stringOfType(Type t) {
    return stringOfTypeWithPrec(t, 0);
}


// Term pretty-printer
std::string stringOfTerm(Term t) {
    if (!t) return "<nil>";
    std::ostringstream out;
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
            out << "(" << stringOfTerm(tp.left) << ", " << stringOfTerm(tp.right) << ")";
            break;
        }

        case TermNode::TmLet: {
            auto const &lt = std::get<TermNode::Let>(t->payload);
            out << "let " << lt.name << " : " << stringOfType(lt.type)
                << " = " << stringOfTerm(lt.e1)
                << " in " << stringOfTerm(lt.e2);
            break;
        }

        case TermNode::TmAbs: {
            auto const &fn = std::get<TermNode::Abs>(t->payload);
            out << "(fun " << fn.param << " : " << stringOfType(fn.paramType)
                << " -> " << stringOfTerm(fn.body) << ")";
            break;
        }

        case TermNode::TmApp: {
            auto const &ap = std::get<TermNode::App>(t->payload);
            // Print application with parentheses to be explicit and avoid ambiguity.
            out << "(" << stringOfTerm(ap.f) << ") (" << stringOfTerm(ap.arg) << ")";
            break;
        }

        default:
            out << "<unknown-term>";
            break;
    }
    return out.str();
}
