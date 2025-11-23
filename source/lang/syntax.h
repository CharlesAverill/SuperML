#pragma once
#include <memory>
#include <string>
#include <variant>
#include <iostream>

// ======================================================
// Forward Declarations
// ======================================================
struct TypeNode;
struct TermNode;

using Type = std::shared_ptr<const TypeNode>;
using Term = std::shared_ptr<const TermNode>;

// ======================================================
// Types
// =======================================================
struct TypeNode {
    enum Kind { TUnknown, TUnit, TBool, TInt, TFloat, TString, TTuple, TArrow } kind;

    struct Tuple { Type left, right; };
    struct Arrow { Type param, result; };

    using Payload = std::variant<std::monostate, Tuple, Arrow>;
    Payload payload;

    // ---- Factory constructors ----
    static Type Unknown() { return std::make_shared<TypeNode>(TypeNode{TUnknown, {}}); }
    static Type Unit()    { return std::make_shared<TypeNode>(TypeNode{TUnit, {}}); }
    static Type Bool()    { return std::make_shared<TypeNode>(TypeNode{TBool, {}}); }
    static Type Int()     { return std::make_shared<TypeNode>(TypeNode{TInt, {}}); }
    static Type Float()   { return std::make_shared<TypeNode>(TypeNode{TFloat, {}}); }
    static Type String()  { return std::make_shared<TypeNode>(TypeNode{TString, {}}); }

    static Type TupleType(Type a, Type b) {
        return std::make_shared<TypeNode>(
            TypeNode{ TTuple, Tuple{a,b} }
        );
    }

    static Type ArrowType(Type p, Type r) {
        return std::make_shared<TypeNode>(
            TypeNode{ TArrow, Arrow{p,r} }
        );
    }
};

// ======================================================
// Terms
// ======================================================
struct TermNode {
    enum Kind {
        TmUnit, TmBool, TmInt, TmFloat, TmString,
        TmTuple, TmLet, TmAbs, TmApp, TmVar
    } kind;

    struct Tuple { Term left, right; };
    struct Let   { std::string name; Type type; Term e1, e2; };
    struct Abs   { std::string param; Type paramType; Term body; };
    struct App   { Term f, arg; };
    struct Var   { std::string name; int index; };

    using Payload = std::variant<
        std::monostate, bool, int, double, std::string,
        Tuple, Let, Abs, App, Var
    >;

    Payload payload;
    Type type;  // optional annotated type

    // ---- Factory functions ----
    static Term Unit() {
        return std::make_shared<TermNode>(TermNode{TmUnit, {}, TypeNode::Unit()});
    }
    static Term Bool(bool b) {
        return std::make_shared<TermNode>(TermNode{TmBool, b, TypeNode::Bool()});
    }
    static Term Int(int i) {
        return std::make_shared<TermNode>(TermNode{TmInt, i, TypeNode::Int()});
    }
    static Term Float(double f) {
        return std::make_shared<TermNode>(TermNode{TmFloat, f, TypeNode::Float()});
    }
    static Term String(std::string s) {
        return std::make_shared<TermNode>(TermNode{TmString, std::move(s), TypeNode::String()});
    }

    static Term VarTerm(std::string name, int index, Type t = TypeNode::Unknown()) {
        return std::make_shared<TermNode>(
            TermNode{TmVar, Var{name,index}, t}
        );
    }

    static Term TupleTerm(Term a, Term b) {
        return std::make_shared<TermNode>(
            TermNode{TmTuple, Tuple{a,b}, TypeNode::TupleType(a->type, b->type)}
        );
    }

    static Term LetTerm(std::string name, Type type, Term e1, Term e2) {
        return std::make_shared<TermNode>(
            TermNode{TmLet, Let{name,type,e1,e2}, e2->type}
        );
    }

    static Term AbsTerm(std::string param, Type pty, Term body) {
        Type t = TypeNode::ArrowType(pty, body->type);
        return std::make_shared<TermNode>(
            TermNode{TmAbs, Abs{param,pty,body}, t}
        );
    }

    static Term AppTerm(Term f, Term arg) {
        return std::make_shared<TermNode>(
            TermNode{TmApp, App{f,arg}, nullptr} // type assigned after inference
        );
    }
};

bool termsEqual(Term a, Term b);

std::string stringOfType(Type t);
std::string stringOfTerm(Term t);
