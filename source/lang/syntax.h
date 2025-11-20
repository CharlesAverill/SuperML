#ifndef SYNTAX
#define SYNTAX

#include <string>
#include <stdexcept>

struct Term;
struct Type;
struct Tuple;
struct TupleType;
struct Arrow;
struct ArrowType;

enum TypeName {
    TUnknown,
    TUnit,
    TBool,
    TInt,
    TFloat,
    TString,
    TTuple,
    TArrow
};

struct Type {
    TypeName typeName;
    union {
        TupleType *tupleType;
        ArrowType *arrowType;
    } typeData;
};

struct ArrowType {
    Type *leftType;
    Type *rightType;
};

struct TupleType {
    Type *leftType;
    Type *rightType;
};

enum TermKind {
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
};

struct Tuple {
    Term *leftValue;
    Term *rightValue;
};

struct Let {
    std::string name;
    Type type;
    Term *e1;
    Term *e2;
};

struct Func {
    std::string paramName;
    Type paramType;
    Term *body;
};

struct Application {
    Term *left;
    Term *right;
};

struct Variable {
    std::string name;
    int index;
};

struct Term {
    TermKind kind;
    Type type;

    union {
        bool boolValue;
        int intValue;
        float floatValue;
        std::string *stringValue;
        Tuple *tupleValue;
        Let *letValue;
        Func *funcValue;
        Application *appValue;
        Variable *varValue;
    } value;
};

Term makeUnit(void);
Term makeBool(bool b);
Term makeInt(int i);
Term makeFloat(float f);
Term makeString(std::string str);
Term makeTuple(Term* left, Term* right);
Term makeLet(std::string name, Type type, Term* e1, Term* e2);
Term makeFunc(std::string paramName, Type paramType, Term* body);
Term makeApp(Term* function, Term* argument, Type type);
Term makeVar(std::string name, int index, Type t);

Type makeUnknownType(void);
Type makeUnitType(void);
Type makeBoolType(void);
Type makeIntType(void);
Type makeFloatType(void);
Type makeStringType(void);
Type makeTupleType(Type* left, Type* right);
Type makeArrowType(Type* param, Type* ret);

Term *copyTerm(Term* term);
Type *copyType(Type* type);

bool typesEqual(Type* a, Type* b);
bool termsEqual(Term* a, Term* b);

void freeTerm(Term* term);
void freeOnlyTerm(Term* term);
void freeType(Type* type);

#endif /* SYNTAX */
