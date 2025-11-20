#include "syntax.h"

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

Type makeStringType(void) {
    return {.typeName=TString};
}

Type makeTupleType(Type *left, Type *right) {
    TupleType *tupleType = new TupleType();
    tupleType->leftType = left;
    tupleType->rightType = right;

    Type out = {
        .typeName=TTuple
    };

    out.typeData.tupleType = tupleType;
    return out;
}

Type makeArrowType(Type *param, Type *ret) {
    ArrowType *arrowType = new ArrowType;
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

Term makeString(std::string str) {
    Term out = {.kind=TmString, .type=makeStringType()};
    out.value.stringValue = new std::string(str);
    return out;
}

Term makeTuple(Term* left, Term* right) {
    Term out = {.kind=TmTuple, .type=makeTupleType(&left->type, &right->type)};
    Tuple *tuple = new Tuple();
    tuple->leftValue = left;
    tuple->rightValue = right;
    out.value.tupleValue = tuple;
    return out;
}

Term makeLet(std::string name, Type type, Term* e1, Term* e2) {
    Term out = {.kind=TmLet, .type=type};
    Let *let = new Let();
    let->name = name;
    let->type = type;
    let->e1 = e1;
    let->e2 = e2;
    out.value.letValue = let;
    return out;
}

Term makeFunc(std::string paramName, Type paramType, Term* body) {
    Func *func = new Func();
    func->paramName = paramName;
    func->paramType = paramType;
    func->body = body;
    Term out = {.kind=TmAbs, .type=makeArrowType(&func->paramType, &body->type)};
    out.value.funcValue = func;
    return out;
}

Term makeApp(Term* function, Term* argument, Type type) {
    Term out = {.kind=TmApp, .type=type};
    Application *app = new Application();
    app->left = function;
    app->right = argument;
    out.value.appValue = app;
    
    return out;
}

Term makeVar(std::string name, int index, Type t) {
    Term out = {.kind=TmVar, .type=t};
    Variable *var = new Variable();
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
        case TString:
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

    return false;
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
        case TmString:
            return *a->value.stringValue == *b->value.stringValue;
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

    return false;
}

void freeTerm(Term* term) {
    freeOnlyTerm(term);

    freeType(&term->type);
}

void freeOnlyTerm(Term* term) {
    switch (term->kind) {
        case TmString:
            delete term->value.stringValue;
            break;
        case TmTuple:
            freeTerm(term->value.tupleValue->leftValue);
            freeTerm(term->value.tupleValue->rightValue);
            delete term->value.tupleValue;
            break;
        case TmAbs:
            freeTerm(term->value.funcValue->body);
            delete term->value.funcValue;
            break;
        case TmApp:
            freeTerm(term->value.appValue->left);
            freeTerm(term->value.appValue->right);
            delete term->value.appValue;
            break;
        case TmVar:
            delete term->value.varValue;
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
            delete type->typeData.tupleType;
            break;
        case TArrow:
            freeType(type->typeData.arrowType->leftType);
            freeType(type->typeData.arrowType->rightType);
            delete type->typeData.arrowType;
            break;
        default:
            break;
    }
}

Term* copyTerm(Term* t) {
    if (!t) return nullptr;

    switch (t->kind) {

        // --------------------------------------
        // Unit
        // --------------------------------------
        case TmUnit: {
            Term* r = new Term(makeUnit());
            r->type = t->type;
            return r;
        }

        // --------------------------------------
        // Bool
        // --------------------------------------
        case TmBool: {
            Term* r = new Term(makeBool(t->value.boolValue));
            r->type = t->type;
            return r;
        }

        // --------------------------------------
        // Int
        // --------------------------------------
        case TmInt: {
            Term* r = new Term(makeInt(t->value.intValue));
            r->type = t->type;
            return r;
        }

        // --------------------------------------
        // Float
        // --------------------------------------
        case TmFloat: {
            Term* r = new Term(makeFloat(t->value.floatValue));
            r->type = t->type;
            return r;
        }

        // --------------------------------------
        // Variable
        // --------------------------------------
        case TmVar: {
            Variable* v = t->value.varValue;
            Term* r = new Term(
                makeVar(v->name, v->index, t->type)
            );
            return r;
        }

        // --------------------------------------
        // Tuple
        // --------------------------------------
        case TmTuple: {
            Tuple* tup = t->value.tupleValue;

            Term* left2  = copyTerm(tup->leftValue);
            Term* right2 = copyTerm(tup->rightValue);

            Term r = makeTuple(left2, right2);
            r.type = t->type;
            return new Term(r);
        }

        // --------------------------------------
        // Let
        // --------------------------------------
        case TmLet: {
            Let* letv = t->value.letValue;

            Term* e1 = copyTerm(letv->e1);
            Term* e2 = copyTerm(letv->e2);

            Term r = makeLet(letv->name, letv->type, e1, e2);
            r.type = t->type;

            return new Term(r);
        }

        // --------------------------------------
        // Lambda abstraction
        // --------------------------------------
        case TmAbs: {
            Func* f = t->value.funcValue;

            Term* body2 = copyTerm(f->body);

            Term r = makeFunc(
                f->paramName,
                f->paramType,
                body2
            );
            r.type = t->type;

            return new Term(r);
        }

        // --------------------------------------
        // Application
        // --------------------------------------
        case TmApp: {
            Application* a = t->value.appValue;

            Term* left2  = copyTerm(a->left);
            Term* right2 = copyTerm(a->right);

            Term r = makeApp(left2, right2, t->type);
            r.type = t->type;

            return new Term(r);
        }

        default:
            return nullptr;
    }
}

Type* copyType(Type* t) {
    if (!t) return nullptr;

    Type* r = new Type;
    r->typeName = t->typeName;

    switch (t->typeName) {

        case TTuple: {
            r->typeData.tupleType = new TupleType;
            r->typeData.tupleType->leftType  = copyType(t->typeData.tupleType->leftType);
            r->typeData.tupleType->rightType = copyType(t->typeData.tupleType->rightType);
            return r;
        }

        case TArrow: {
            r->typeData.arrowType = new ArrowType;
            r->typeData.arrowType->leftType  = copyType(t->typeData.arrowType->leftType);
            r->typeData.arrowType->rightType = copyType(t->typeData.arrowType->rightType);
            return r;
        }

        default:
            // TUnit, TBool, TInt, TFloat, TUnknown
            r->typeData.arrowType = nullptr;
            return r;
    }
}
