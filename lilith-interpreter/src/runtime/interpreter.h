#ifndef LILITH_INTERPRETER_H
#define LILITH_INTERPRETER_H

#include "value.h"
#include "environment.h"
#include "parser/ast.h"

/* -------------------------------------------------------------------------- */
/* Interpreter state                                                         */
/* -------------------------------------------------------------------------- */

typedef struct Interpreter {
    Environment *globals;
    Environment *env;

    /* Control-flow flags */
    int return_flag;
    int break_flag;
    int continue_flag;
    int throw_flag;
    char *error_msg;

    /* Current function for return-type checking */
    ObjFunction *current_function;
} Interpreter;

/* -------------------------------------------------------------------------- */
/* Public API                                                                */
/* -------------------------------------------------------------------------- */

void interpreter_init(Interpreter *interp);
void interpreter_free(Interpreter *interp);
Value interpreter_run(Interpreter *interp, AstNode *program);

Value eval_expr(Interpreter *interp, AstNode *node);
Value eval_stmt(Interpreter *interp, AstNode *node);

/* Error handling */
void runtime_error(Interpreter *interp, const char *fmt, ...);

#endif
