#include "reductions.h"

// Helper function: insert a let binding at the deepest point
// This recursively finds nested Let/LetRec/LetTuple and goes to the bottom
Term* insert(const std::string& name, Type* type, Term* body, Term* continuation) {
    switch (body->kind) {
        case TmLet: {
            // If body is a Let, recurse deeper
            Let* innerLet = body->value.letValue;
            Term* newE2 = insert(name, type, innerLet->e2, continuation);
            
            // Rebuild the Let with the inserted continuation
            Term* result = (Term*)malloc(sizeof(Term));
            result->kind = TmLet;
            result->type = body->type;
            
            Let* newLet = new Let();
            newLet->name = innerLet->name;
            newLet->type = innerLet->type;
            newLet->e1 = innerLet->e1;
            newLet->e2 = newE2;
            result->value.letValue = newLet;
            
            return result;
        }

        default: {
            // Base case: body is not a Let/LetRec/LetTuple
            // Create a new Let with this body and the continuation
            Term* result = (Term*)malloc(sizeof(Term));
            result->kind = TmLet;
            
            Let* newLet = new Let();
            newLet->name = name;
            newLet->type = type;
            newLet->e1 = body;
            assoc(continuation);
            freeTerm(newLet->e2);
            *newLet->e2 = *continuation;
            result->value.letValue = newLet;
            
            // Set the type (would need proper type inference here)
            result->type = continuation->type;
            
            return result;
        }
    }
}

void assoc(Term* term) {    
    switch (term->kind) {
        case TmUnit:
        case TmBool:
        case TmInt:
        case TmFloat:
        case TmVar:
            break;
        
        case TmLet: {
            Let* let = term->value.letValue;
            assoc(let->e1);
            Term* newTerm = insert(let->name, let->type, let->e1, let->e2);
            freeTerm(term);
            *term = *newTerm;
            free(newTerm);
        }
        
        case TmTuple: {
            Tuple* tuple = term->value.tupleValue;
            assoc(tuple->leftValue);
            assoc(tuple->rightValue);
        }
        
        case TmAbs: {
            Func* func = term->value.funcValue;            
            assoc(func->body);
        }
        
        case TmApp: {
            Application* app = term->value.appValue;
            assoc(app->left);
            assoc(app->right);
        }
        
        default:
            return;
    }
}
