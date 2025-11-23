#pragma once
#include <iostream>
#include <memory>
#include <string>
#include <variant>

struct TypeNode;
struct TermNode;

using Type = std::shared_ptr<const TypeNode>;
using Term = std::shared_ptr<const TermNode>;

struct TypeNode {
  enum Kind {
    TUnknown,
    TUnit,
    TBool,
    TInt,
    TFloat,
    TString,
    TTuple,
    TArrow
  } kind;

  struct Tuple {
    Type left, right;
  };
  struct Arrow {
    Type param, result;
  };

  using Payload = std::variant<std::monostate, Tuple, Arrow>;
  Payload payload;

  // ---- Factory constructors ----
  static Type Unknown() {
    return std::make_shared<TypeNode>(TypeNode{TUnknown, {}});
  }
  static Type Unit() { return std::make_shared<TypeNode>(TypeNode{TUnit, {}}); }
  static Type Bool() { return std::make_shared<TypeNode>(TypeNode{TBool, {}}); }
  static Type Int() { return std::make_shared<TypeNode>(TypeNode{TInt, {}}); }
  static Type Float() {
    return std::make_shared<TypeNode>(TypeNode{TFloat, {}});
  }
  static Type String() {
    return std::make_shared<TypeNode>(TypeNode{TString, {}});
  }

  static Type TupleType(Type a, Type b) {
    return std::make_shared<TypeNode>(TypeNode{TTuple, Tuple{a, b}});
  }

  static Type ArrowType(Type p, Type r) {
    return std::make_shared<TypeNode>(TypeNode{TArrow, Arrow{p, r}});
  }

  bool operator==(const TypeNode &other) const {
    if (this->kind != other.kind)
      return false;

    switch (this->kind) {
    case TypeNode::TTuple: {
      auto &A = std::get<TypeNode::Tuple>(this->payload);
      auto &B = std::get<TypeNode::Tuple>(other.payload);
      return *A.left == *B.left && *A.right == *B.right;
    }
    case TypeNode::TArrow: {
      auto &A = std::get<TypeNode::Arrow>(this->payload);
      auto &B = std::get<TypeNode::Arrow>(other.payload);
      return *A.param == *B.param && *A.result == *B.result;
    }
    default:
      return true;
    }
  }

  bool operator!=(const TypeNode &other) const { return !(*this == other); }
};

struct TermNode {
  enum Kind {
    TmUnit,
    TmBool,
    TmInt,
    TmFloat,
    TmString,
    TmTuple,
    TmLet,
    TmAbs,
    TmApp,
    TmVar
  } kind;

  struct Tuple {
    Term left, right;
  };
  struct Let {
    std::string name;
    Type type;
    Term e1, e2;
  };
  struct Abs {
    std::string param;
    Type paramType;
    Term body;
  };
  struct App {
    Term f, arg;
  };
  struct Var {
    std::string name;
    int index;
  };

  using Payload = std::variant<std::monostate, bool, int, float, std::string,
                               Tuple, Let, Abs, App, Var>;

  Payload payload;
  Type type; // optional annotated type

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
  static Term Float(float f) {
    return std::make_shared<TermNode>(TermNode{TmFloat, f, TypeNode::Float()});
  }
  static Term String(std::string s) {
    return std::make_shared<TermNode>(
        TermNode{TmString, std::move(s), TypeNode::String()});
  }

  static Term VarTerm(std::string name, int index,
                      Type t = TypeNode::Unknown()) {
    return std::make_shared<TermNode>(TermNode{TmVar, Var{name, index}, t});
  }

  static Term TupleTerm(Term a, Term b) {
    return std::make_shared<TermNode>(
        TermNode{TmTuple, Tuple{a, b}, TypeNode::TupleType(a->type, b->type)});
  }

  static Term LetTerm(std::string name, Type type, Term e1, Term e2) {
    return std::make_shared<TermNode>(
        TermNode{TmLet, Let{name, type, e1, e2}, e2->type});
  }

  static Term AbsTerm(std::string param, Type pty, Term body) {
    Type t = TypeNode::ArrowType(pty, body->type);
    return std::make_shared<TermNode>(
        TermNode{TmAbs, Abs{param, pty, body}, t});
  }

  static Term AppTerm(Term f, Term arg) {
    return std::make_shared<TermNode>(TermNode{TmApp, App{f, arg}, nullptr}
                                      // type assigned after inference
    );
  }

  bool operator==(const TermNode &other) const {
    if (this->kind != other.kind)
      return false;

    switch (this->kind) {
    case TermNode::TmUnit:
      return true;
    case TermNode::TmBool:
      return std::get<bool>(this->payload) == std::get<bool>(other.payload);
    case TermNode::TmInt:
      return std::get<int>(this->payload) == std::get<int>(other.payload);
    case TermNode::TmFloat:
      return std::get<float>(this->payload) == std::get<float>(other.payload);
    case TermNode::TmString:
      return std::get<std::string>(this->payload) ==
             std::get<std::string>(other.payload);

    case TermNode::TmVar: {
      auto &A = std::get<TermNode::Var>(this->payload);
      auto &B = std::get<TermNode::Var>(other.payload);
      return A.name == B.name && A.index == B.index;
    }

    case TermNode::TmTuple: {
      auto &A = std::get<TermNode::Tuple>(this->payload);
      auto &B = std::get<TermNode::Tuple>(other.payload);
      return *A.left == *B.left && *A.right == *B.right;
    }

    case TermNode::TmLet: {
      auto &A = std::get<TermNode::Let>(this->payload);
      auto &B = std::get<TermNode::Let>(other.payload);
      return A.name == B.name && A.type == B.type && *A.e1 == *B.e1 &&
             *A.e2 == *B.e2;
    }

    case TermNode::TmAbs: {
      auto &A = std::get<TermNode::Abs>(this->payload);
      auto &B = std::get<TermNode::Abs>(other.payload);
      return A.param == B.param && A.paramType == B.paramType &&
             *A.body == *B.body;
    }

    case TermNode::TmApp: {
      auto &A = std::get<TermNode::App>(this->payload);
      auto &B = std::get<TermNode::App>(other.payload);
      return *A.f == *B.f && *A.arg == *B.arg;
    }
    }

    return false;
  }

  bool operator!=(const TermNode &other) const { return !(*this == other); }
};

std::string stringOfType(Type t, int depth = 0);
std::string stringOfTerm(Term t, int depth = 0);
