#include "syntax.h"

Term* makeTerm(void) {
    return (Term*)malloc(sizeof(Term));
}

Type* makeType(void) {
    return (Type*)malloc(sizeof(Type));
}

Type makeUnknownType(void) {
    return {.typeName=TUnknown};
}

Type makeUnitType(void) {
    return {.typeName=TUnit};
}

Type makeBoolType(void) {
    return {.typeName=TBool};
}

Type makeIntType(void) {
    return {.typeName=TInt};
}

Type makeFloatType(void) {
    return {.typeName=TFloat};
}

Type makeTupleType(Type *left, Type *right) {
    TupleType *tupleType = (TupleType*)malloc(sizeof(TupleType));
    tupleType->leftType = left;
    tupleType->rightType = right;

    Type out = {
        .typeName=TTuple
    };

    out.typeData.tupleType = tupleType;
    return out;
}

Type makeArrowType(Type *param, Type *ret) {
    ArrowType *arrowType = (ArrowType*)malloc(sizeof(ArrowType));
    arrowType->leftType = param;
    arrowType->rightType = ret;

    Type out = {
        .typeName=TArrow
    };

    out.typeData.arrowType = arrowType;
    return out;
}

Term makeUnit(void) {
    return {.kind=TmUnit, .type=makeUnitType()};
}

Term makeBool(bool b) {
    Term out = {.kind=TmBool, .type=makeBoolType()};
    out.value.boolValue = b;
    return out;
}

Term makeInt(int i) {
    Term out = {.kind=TmInt, .type=makeIntType()};
    out.value.intValue = i;
    return out;
}

Term makeFloat(float f) {
    Term out = {.kind=TmFloat, .type=makeFloatType()};
    out.value.floatValue = f;
    return out;
}

Term makeTuple(Term* left, Term* right) {
    Term out = {.kind=TmTuple, .type=makeTupleType(&left->type, &right->type)};
    Tuple *tuple = (Tuple*)malloc(sizeof(Tuple));
    tuple->leftValue = left;
    tuple->rightValue = right;
    out.value.tupleValue = tuple;
    return out;
}

Term makeLet(std::string name, Type* type, Term* e1, Term* e2) {
    Term out = {.kind=TmLet, .type=*type};
    Let *let = (Let*)malloc(sizeof(Let));
    let->name = name;
    let->type = type;
    let->e1 = e1;
    let->e2 = e2;
    out.value.letValue = let;
    return out;
}

Term makeFunc(std::string paramName, Type* paramType, Term* body) {
    Term out = {.kind=TmAbs, .type=makeArrowType(paramType, &body->type)};
    Func *func = (Func*)malloc(sizeof(Func));
    func->paramName = paramName;
    func->paramType = paramType;
    func->body = body;
    out.value.funcValue = func;
    return out;
}

Term makeApp(Term* function, Term* argument, Type* type) {
    Term out = {.kind=TmApp, .type=*type};
    Application *app = (Application*)malloc(sizeof(Application));
    app->left = function;
    app->right = argument;
    out.value.appValue = app;
    
    return out;
}

Term makeVar(std::string name, int index, Type *t) {
    Term out = {.kind=TmVar, .type=*t};
    Variable *var = (Variable*)malloc(sizeof(Variable));
    var->name = name;
    var->index = index;
    out.value.varValue = var;
    return out;
}

bool typesEqual(Type* a, Type* b) {
    if (a->typeName != b->typeName)
        return false;
    switch (a->typeName) {
        case TUnit:
        case TBool:
        case TInt:
        case TFloat:
        case TUnknown:
            return true;
        case TTuple:
            return typesEqual(a->typeData.tupleType->leftType, 
                              b->typeData.tupleType->leftType) &&
                   typesEqual(a->typeData.tupleType->rightType, 
                              b->typeData.tupleType->rightType);
        case TArrow:
            return typesEqual(a->typeData.arrowType->leftType, 
                              b->typeData.arrowType->leftType) &&
                   typesEqual(a->typeData.arrowType->rightType,
                              b->typeData.arrowType->rightType);
    }
}

bool termsEqual(Term* a, Term* b) {
    if (a->kind != b->kind)
        return false;

    switch (a->kind) {
        case TmUnit:
            return true;
        case TmBool:
            return a->value.boolValue == b->value.boolValue;
        case TmInt:
            return a->value.intValue == b->value.intValue;
        case TmFloat:
            return a->value.floatValue == b->value.floatValue;
        case TmTuple:
            return termsEqual(a->value.tupleValue->leftValue, b->value.tupleValue->leftValue) &&
                   termsEqual(a->value.tupleValue->rightValue, b->value.tupleValue->rightValue);
        case TmLet:
        case TmAbs:
        case TmApp:
            return false;
        case TmVar:
            return a->value.varValue->name == b->value.varValue->name;
    }
}

void freeTerm(Term* term) {
    freeOnlyTerm(term);

    freeType(&term->type);
}

void freeOnlyTerm(Term* term) {
    switch (term->kind) {
        case TmTuple:
            freeTerm(term->value.tupleValue->leftValue);
            freeTerm(term->value.tupleValue->rightValue);
            free(term->value.tupleValue);
            break;
        case TmAbs:
            freeType(term->value.funcValue->paramType);
            freeTerm(term->value.funcValue->body);
            free(term->value.funcValue);
            break;
        case TmApp:
            freeTerm(term->value.appValue->left);
            freeTerm(term->value.appValue->right);
            free(term->value.appValue);
            break;
        case TmVar:
            free(term->value.varValue);
            break;
        default:
            break;
    }
}

void freeType(Type* type) {
    switch (type->typeName) {
        case TTuple:
            freeType(type->typeData.tupleType->leftType);
            freeType(type->typeData.tupleType->rightType);
            free(type->typeData.tupleType);
        case TArrow:
            freeType(type->typeData.arrowType->leftType);
            freeType(type->typeData.arrowType->rightType);
            free(type->typeData.arrowType);
        default:
            break;
    }
}
