#include "stdlib.h"
#include <iostream>

bool isPrimitive(Term term) {
  return term->kind == TermNode::TmVar &&
         primitives.count(std::get<TermNode::Var>(term->payload).name) > 0;
}

#define _UNARY(name, type, ret, in, out)                                       \
  Primitive name = {.f = [](Term arg) -> Term {                                \
                      type val = std::get<type>(arg->payload);                 \
                      ret;                                                     \
                    },                                                         \
                    .t = TypeNode::ArrowType(TypeNode::in, TypeNode::out)};

#define UNARY(name, type, ret, in, out) _UNARY(name, type, ret, in(), out())

#define BINARY(name, type1, type2, ret, in1, in2, out)                         \
  Primitive name = {.f = [](Term arg) -> Term {                                \
                      TermNode::Tuple tup =                                    \
                          std::get<TermNode::Tuple>(arg->payload);             \
                      type1 val1 = std::get<type1>(tup.left->payload);         \
                      type2 val2 = std::get<type2>(tup.right->payload);        \
                      ret;                                                     \
                    },                                                         \
                    .t = TypeNode::ArrowType(                                  \
                        TypeNode::TupleType(TypeNode::in1(), TypeNode::in2()), \
                        TypeNode::out())};

// ------------------ Output functions ------------------

UNARY(
    print_string, std::string,
    {
      std::cout << val;
      return TermNode::Unit();
    },
    String, Unit)

UNARY(
    print_endline, std::string,
    {
      std::cout << val << std::endl;
      return TermNode::Unit();
    },
    String, Unit)

UNARY(
    print_int, int,
    {
      std::cout << val;
      return TermNode::Unit();
    },
    Int, Unit)

UNARY(
    print_float, double,
    {
      std::streamsize ss = std::cout.precision();
      std::cout << std::setprecision(15) << val << std::setprecision(ss);
      return TermNode::Unit();
    },
    Float, Unit)

UNARY(
    print_bool, bool,
    {
      std::cout << std::boolalpha << val << std::noboolalpha;
      return TermNode::Unit();
    },
    Bool, Unit)

// ------------------ Input functions ------------------

// Maximum characters to read in at a time is 1kb
#define READ_MAX 1024

UNARY(
    read_line, std::monostate,
    {

#ifdef __3DS__
      char buf[READ_MAX];
      normalKeyboardInit();
      setupKeyboard("read_line", "");
      swkbdInputText(&swkbd, buf, READ_MAX);
      //   std::cout << buf << std::endl;
      return TermNode::String(buf);
#else
      std::string in;
      std::getline(std::cin, in);
      return TermNode::String(in);
#endif
    },
    Unit, String)

UNARY(
    read_int, std::monostate,
    {

#ifdef __3DS__
      char buf[READ_MAX];
      intKeyboardInit();
      setupKeyboard("read_int", "");
      swkbdInputText(&swkbd, buf, READ_MAX);
      int x = std::stoi(buf);
      //   std::cout << x << std::endl;
      normalKeyboardInit();
      return TermNode::Int(x);
#else
      std::string in;
      std::getline(std::cin, in);
      return TermNode::Int(std::stoi(in));
#endif
    },
    Unit, Int)

UNARY(
    read_float, std::monostate,
    {

#ifdef __3DS__
      char buf[READ_MAX];
      floatKeyboardInit();
      setupKeyboard("read_float", "");
      swkbdInputText(&swkbd, buf, READ_MAX);
      double x = std::stod(buf);
      //   std::cout << x << std::endl;
      normalKeyboardInit();
      return TermNode::Float(x);
#else
      std::string in;
      std::getline(std::cin, in);
      return TermNode::Float(std::stod(in));
#endif
    },
    Unit, Float)

// ------------------ Boolean functions ------------------

UNARY(_not, bool, { return TermNode::Bool(!val); }, Bool, Bool)

BINARY(
    _and, bool, bool, { return TermNode::Bool(val1 && val2); }, Bool, Bool,
    Bool)

BINARY(
    _or, bool, bool, { return TermNode::Bool(val1 || val2); }, Bool, Bool, Bool)

// ------------------ Integer functions ------------------

UNARY(neg, int, { return TermNode::Int(-val); }, Int, Int)

UNARY(succ, int, { return TermNode::Int(val + 1); }, Int, Int)

UNARY(pred, int, { return TermNode::Int(val - 1); }, Int, Int)

BINARY(add, int, int, { return TermNode::Int(val1 + val2); }, Int, Int, Int)

BINARY(sub, int, int, { return TermNode::Int(val1 - val2); }, Int, Int, Int)

BINARY(mul, int, int, { return TermNode::Int(val1 * val2); }, Int, Int, Int)

BINARY(_div, int, int, { return TermNode::Int(val1 / val2); }, Int, Int, Int)

BINARY(mod, int, int, { return TermNode::Int(val1 % val2); }, Int, Int, Int)

UNARY(_abs, int, { return TermNode::Int(abs(val)); }, Int, Int)

BINARY(land, int, int, { return TermNode::Int(val1 & val2); }, Int, Int, Int)

BINARY(lor, int, int, { return TermNode::Int(val1 | val2); }, Int, Int, Int)

BINARY(lxor, int, int, { return TermNode::Int(val1 ^ val2); }, Int, Int, Int)

UNARY(lnot, int, { return TermNode::Int(~val); }, Int, Int)

BINARY(lsl, int, int, { return TermNode::Int(val1 << val2); }, Int, Int, Int)

BINARY(
    lsr, int, int,
    { return TermNode::Int(static_cast<unsigned int>(val1) >> val2); }, Int,
    Int, Int)

BINARY(asr, int, int, { return TermNode::Int(val1 >> val2); }, Int, Int, Int)

// ------------------ Float functions ------------------

UNARY(fneg, double, { return TermNode::Float(-val); }, Float, Float)

UNARY(fpos, double, { return TermNode::Float(+val); }, Float, Float)

BINARY(
    _fadd, double, double, { return TermNode::Float(val1 + val2); }, Float,
    Float, Float)

BINARY(
    _fsub, double, double, { return TermNode::Float(val1 - val2); }, Float,
    Float, Float)

BINARY(
    _fmul, double, double, { return TermNode::Float(val1 * val2); }, Float,
    Float, Float)

BINARY(
    _fdiv, double, double, { return TermNode::Float(val1 / val2); }, Float,
    Float, Float)

BINARY(
    fpow, double, double, { return TermNode::Float(std::pow(val1, val2)); },
    Float, Float, Float)

UNARY(_fsqrt, double, { return TermNode::Float(std::sqrt(val)); }, Float, Float)

UNARY(_fexp, double, { return TermNode::Float(std::exp(val)); }, Float, Float)

UNARY(flog, double, { return TermNode::Float(std::log(val)); }, Float, Float)

UNARY(
    flog10, double, { return TermNode::Float(std::log10(val)); }, Float, Float)

UNARY(
    fexpm1, double, { return TermNode::Float(std::expm1(val)); }, Float, Float)

UNARY(
    flog1p, double, { return TermNode::Float(std::log1p(val)); }, Float, Float)

UNARY(fcos, double, { return TermNode::Float(std::cos(val)); }, Float, Float)

UNARY(fsin, double, { return TermNode::Float(std::sin(val)); }, Float, Float)

UNARY(ftan, double, { return TermNode::Float(std::tan(val)); }, Float, Float)

UNARY(facos, double, { return TermNode::Float(std::acos(val)); }, Float, Float)

UNARY(fasin, double, { return TermNode::Float(std::asin(val)); }, Float, Float)

UNARY(fatan, double, { return TermNode::Float(std::atan(val)); }, Float, Float)

BINARY(
    fatan2, double, double, { return TermNode::Float(std::atan2(val1, val2)); },
    Float, Float, Float)

UNARY(fcosh, double, { return TermNode::Float(std::cosh(val)); }, Float, Float)

UNARY(fsinh, double, { return TermNode::Float(std::sinh(val)); }, Float, Float)

UNARY(ftanh, double, { return TermNode::Float(std::tanh(val)); }, Float, Float)

UNARY(
    facosh, double, { return TermNode::Float(std::acosh(val)); }, Float, Float)

UNARY(
    fasinh, double, { return TermNode::Float(std::asinh(val)); }, Float, Float)

UNARY(
    fatanh, double, { return TermNode::Float(std::atanh(val)); }, Float, Float)

BINARY(
    fhypot, double, double, { return TermNode::Float(std::hypot(val1, val2)); },
    Float, Float, Float)

BINARY(
    fcopysign, double, double,
    { return TermNode::Float(std::copysign(val1, val2)); }, Float, Float, Float)

BINARY(
    fmod_float, double, double,
    { return TermNode::Float(std::fmod(val1, val2)); }, Float, Float, Float)

_UNARY(
    ffrexp, double,
    {
      int exp;
      double sig = std::frexp(val, &exp);
      return TermNode::TupleTerm(TermNode::Float(sig), TermNode::Int(exp));
    },
    Float(), TupleType(TypeNode::Float(), TypeNode::Int()))

BINARY(
    fldexp, double, int, { return TermNode::Float(std::ldexp(val1, val2)); },
    Float, Int, Float)

_UNARY(
    _fmodf, double,
    {
      double intpart;
      double frac = std::modf(val, &intpart);
      return TermNode::TupleTerm(TermNode::Float(frac),
                                 TermNode::Float(intpart));
    },
    Float(), TupleType(TypeNode::Float(), TypeNode::Float()))

UNARY(
    float_of_int, int, { return TermNode::Float(static_cast<double>(val)); },
    Int, Float)

UNARY(
    int_of_float, double, { return TermNode::Int(static_cast<int>(val)); },
    Float, Int)

// ------------------ String functions ------------------

BINARY(
    concat, std::string, std::string, { return TermNode::String(val1 + val2); },
    String, String, String)

const std::vector<std::pair<std::string_view, Primitive>> primitive_list{
    {{"print_string", print_string},
     {"print_endline", print_endline},
     {"print_int", print_int},
     {"print_bool", print_bool},
     {"print_float", print_float},

     {"read_line", read_line},
     {"read_int", read_int},
     {"read_float", read_float},

     {"not", _not},
     {"and", _and},
     {"or", _or},

     {"neg", neg},
     {"succ", succ},
     {"pred", pred},
     {"add", add},
     {"sub", sub},
     {"mul", mul},
     {"div", _div},
     {"mod", mod},
     {"abs", _abs},
     {"land", land},
     {"lor", lor},
     {"lxor", lxor},
     {"lnot", lnot},
     {"lsl", lsl},
     {"lsr", lsr},
     {"asr", asr},

     {"fneg", fneg},
     {"fpos", fpos},
     {"fadd", _fadd},
     {"fsub", _fsub},
     {"fmul", _fmul},
     {"fdiv", _fdiv},
     {"pow", fpow},
     {"sqrt", _fsqrt},
     {"exp", _fexp},
     {"log", flog},
     {"log10", flog10},
     {"expm1", fexpm1},
     {"log1p", flog1p},
     {"cos", fcos},
     {"sin", fsin},
     {"tan", ftan},
     {"acos", facos},
     {"asin", fasin},
     {"atan", fatan},
     {"atan2", fatan2},
     {"cosh", fcosh},
     {"sinh", fsinh},
     {"tanh", ftanh},
     {"acosh", facosh},
     {"asinh", fasinh},
     {"atanh", fatanh},
     {"hypot", fhypot},
     {"copysign", fcopysign},
     {"mod_float", fmod_float},
     {"frexp", ffrexp},
     {"ldexp", fldexp},
     {"modf", _fmodf},
     {"float_of_int", float_of_int},
     {"float", float_of_int},
     {"int_of_float", int_of_float},
     {"truncate", int_of_float},

     {"concat", concat}}};

const std::unordered_map<std::string, Primitive> primitives = [] {
  std::unordered_map<std::string, Primitive> m;
  for (auto &p : primitive_list)
    m.emplace(std::string(p.first), p.second);
  return m;
}();
