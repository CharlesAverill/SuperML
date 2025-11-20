#include "interpreter.h"
#include "parser/driver.hpp"
#include "syntax.h"
#include <fstream>
#include <iostream>

bool isValue(Term* term) {
    switch (term->kind) {
        case TmApp:
            return false;
        default:
            return true;
    }
}

Term* substitute(Term* term, const std::string& name, Term* replacement) {
    switch (term->kind) {

        // ----------------------------
        // Variable
        // ----------------------------
        case TmVar: {
            Variable* v = term->value.varValue;

            if (v->name == name) {
                // Return a deep copy of replacement
                return copyTerm(replacement);
            } else {
                return copyTerm(term);
            }
        }

        // ----------------------------
        // Lambda abstraction
        // (we assume param shadowing blocks substitution)
        // ----------------------------
        case TmAbs: {
            Func* f = term->value.funcValue;

            // If the lambda parameter is the same name we are
            // substituting, substitution is blocked by shadowing.
            if (f->paramName == name) {
                return copyTerm(term);
            }

            // Otherwise, substitute inside the body
            Term* newBody = substitute(f->body, name, replacement);
            return new Term(makeFunc(f->paramName, f->paramType, newBody));
        }

        // ----------------------------
        // Application
        // ----------------------------
        case TmApp: {
            Application* a = term->value.appValue;

            Term* left2  = substitute(a->left,  name, replacement);
            Term* right2 = substitute(a->right, name, replacement);

            return new Term(makeApp(left2, right2, term->type));
        }

        // ----------------------------
        // Tuple
        // ----------------------------
        case TmTuple: {
            Tuple* tup = term->value.tupleValue;

            Term* left2  = substitute(tup->leftValue,  name, replacement);
            Term* right2 = substitute(tup->rightValue, name, replacement);

            return new Term(makeTuple(left2, right2));
        }

        // ----------------------------
        // Let-binding
        // let x : T = e1 in e2
        // ----------------------------
        case TmLet: {
            Let* letv = term->value.letValue;

            // Substitute into e1 always
            Term* e1_new = substitute(letv->e1, name, replacement);

            // Substitute into e2 only if the binding name does not shadow
            Term* e2_new = nullptr;
            if (letv->name == name) {
                e2_new = copyTerm(letv->e2);
            } else {
                e2_new = substitute(letv->e2, name, replacement);
            }

            return new Term(makeLet(letv->name, letv->type, e1_new, e2_new));
        }

        // ----------------------------
        // Unit / Bool / Int / Float
        // primitive literal values: no change
        // ----------------------------
        case TmUnit:
        case TmBool:
        case TmInt:
        case TmFloat:
            return copyTerm(term);

        default:
            return nullptr;
    }
}

bool step(Term* program, State* state) {
    switch (program->kind) {
        case TmApp: {
            Application* app = program->value.appValue;
            Term* fun = app->left;
            Term* arg = app->right;

            if (!isValue(fun)) {
                if (step(fun, state)) return true;
                return false;
            }

            if (!isValue(arg)) {
                if (step(arg, state)) return true;
                return false;
            }

            if (fun->kind == TmVar) {
                if (isPrimitive(fun)) {
                    Term* result = primitives[fun->value.varValue->name](arg);
                    freeTerm(program);
                    *program = *result;
                    return true;
                }
            } else if (fun->kind == TmAbs) {
                Func* f = fun->value.funcValue;
                Term* newBody = substitute(f->body, f->paramName, arg);
                freeTerm(program);
                *program = *newBody;
                return true;
            }

            return false;
        }
        default:
            return false;
    }
}


void interpreterMain(std::string filename) {
    initPrimitives();

    MC::MC_Driver driver;
    driver.parse(filename.c_str());
    Term prog = *driver.root_term;

    // Type str = makeStringType();
    // Type unit = makeUnitType();
    // Type ty = makeArrowType(&str, &unit);
    // Term helloFunc = makeVar("print_endline", -1, ty);
    // Term strarg = makeString("Hello, World!");
    // Term prog = makeApp(&helloFunc, &strarg, unit);

    reduce(&prog);

    std::string outChannel;
    Env emptyEnv;
    State state = {.outChannel=outChannel, .env=emptyEnv};

    while(step(&prog, &state)) {
        stepCallback(&state);
    }
}
