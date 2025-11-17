#include "reductions.h"

Term* find(const std::string& name, const env& env) {
    auto it = env.find(name);
    if (it != env.end() && it->second != nullptr) {
        return it->second;
    }
    return nullptr; 
}

Term* copyTerm(Term* t) {
    Term* copy = (Term*)malloc(sizeof(Term));
    copy->kind = t->kind;
    copy->type = t->type;
    copy->value = t->value;
    return copy;
}

void _beta(Term* term, env& env) {
    switch (term->kind) {
        case TmUnit:
        case TmBool:
        case TmInt:
        case TmFloat:
            break;
        case TmVar: {
            // Replace variable with its binding if found
            Variable* var = term->value.varValue;
            Term* replacement = find(var->name, env);
            if (replacement != nullptr) {
                freeTerm(term);
                *term = *replacement;
            }
            break;
        }
        case TmTuple:
            _beta(term->value.tupleValue->leftValue, env);
            _beta(term->value.tupleValue->rightValue, env);
            break;
        case TmAbs:
            break;
        case TmApp: {
            Application* app = term->value.appValue;
            _beta(app->left, env);
            _beta(app->right, env);

            if (app->left->kind == TmAbs) {
                Func* arrow = app->left->value.funcValue;
                Term* old = env[arrow->paramName];
                env[arrow->paramName] = app->right;
                _beta(arrow->body, env);
                env[arrow->paramName] = old;

                freeTerm(term);
                *term = *arrow->body;
            }
            break;
        }         
        case TmLet: {
            Let* let = term->value.letValue;

            _beta(let->e1, env);
            
            if (let->e1->kind == TmVar) {
                Term* old = env[let->name];
                env[let->name] = let->e1;
                _beta(let->e2, env);
                env[let->name] = old;
                
                freeTerm(term);
                *term = *let->e2;
            } else {
                Term* old = env[let->name];
                env[let->name] = let->e1;
                _beta(let->e2, env);
                env[let->name] = old;
            }
            break;
        }

        default:
            break;
    }
}

void beta(Term* term) {
    env emptyEnv;
    _beta(term, emptyEnv);
}
