#define _GNU_SOURCE
#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static AstNode *ast_create(AstNodeType type, size_t line, size_t column) {
    AstNode *node = (AstNode *)calloc(1, sizeof(AstNode));
    if (!node) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    node->type = type;
    node->line = line;
    node->column = column;
    return node;
}

/* Statements */

AstNode *ast_program(AstNode *body, size_t line, size_t column) {
    AstNode *n = ast_create(AST_PROGRAM, line, column);
    n->as.program.body = body;
    return n;
}

AstNode *ast_block(AstNode **stmts, size_t count, size_t line, size_t column) {
    AstNode *n = ast_create(AST_BLOCK, line, column);
    n->as.block.stmts = stmts;
    n->as.block.count = count;
    return n;
}

AstNode *ast_expression_stmt(AstNode *expr, size_t line, size_t column) {
    AstNode *n = ast_create(AST_EXPRESSION_STMT, line, column);
    n->as.expression_stmt.expr = expr;
    return n;
}

AstNode *ast_assign(AstNode *target, AstNode *value, size_t line, size_t column) {
    AstNode *n = ast_create(AST_ASSIGN, line, column);
    n->as.assign.target = target;
    n->as.assign.value = value;
    return n;
}

AstNode *ast_if(AstNode *cond, AstNode *then_branch, AstNode *else_branch, size_t line, size_t column) {
    AstNode *n = ast_create(AST_IF, line, column);
    n->as.if_stmt.cond = cond;
    n->as.if_stmt.then_branch = then_branch;
    n->as.if_stmt.else_branch = else_branch;
    return n;
}

AstNode *ast_while(AstNode *cond, AstNode *body, size_t line, size_t column) {
    AstNode *n = ast_create(AST_WHILE, line, column);
    n->as.while_stmt.cond = cond;
    n->as.while_stmt.body = body;
    return n;
}

AstNode *ast_for(const char *var, AstNode *iter, AstNode *body, size_t line, size_t column) {
    AstNode *n = ast_create(AST_FOR, line, column);
    n->as.for_stmt.var = strdup(var);
    n->as.for_stmt.iter = iter;
    n->as.for_stmt.body = body;
    return n;
}

AstNode *ast_return(AstNode *value, size_t line, size_t column) {
    AstNode *n = ast_create(AST_RETURN, line, column);
    n->as.return_stmt.value = value;
    return n;
}

AstNode *ast_yield(AstNode *value, size_t line, size_t column) {
    AstNode *n = ast_create(AST_YIELD, line, column);
    n->as.yield_stmt.value = value;
    return n;
}

AstNode *ast_break(size_t line, size_t column) {
    return ast_create(AST_BREAK, line, column);
}

AstNode *ast_continue(size_t line, size_t column) {
    return ast_create(AST_CONTINUE, line, column);
}

AstNode *ast_function(const char *name, char **params, size_t param_count, AstNode *body, int is_async, size_t line, size_t column) {
    AstNode *n = ast_create(AST_FUNCTION, line, column);
    n->as.function.name = name ? strdup(name) : NULL;
    n->as.function.params = params;
    n->as.function.param_count = param_count;
    n->as.function.body = body;
    n->as.function.is_async = is_async;
    return n;
}

AstNode *ast_class(const char *name, AstNode **methods, size_t method_count, size_t line, size_t column) {
    AstNode *n = ast_create(AST_CLASS, line, column);
    n->as.class_def.name = name ? strdup(name) : NULL;
    if (method_count > 0) {
        n->as.class_def.methods = (AstNode **)malloc(sizeof(AstNode *) * method_count);
        memcpy(n->as.class_def.methods, methods, sizeof(AstNode *) * method_count);
    } else {
        n->as.class_def.methods = NULL;
    }
    n->as.class_def.method_count = method_count;
    return n;
}

AstNode *ast_try(AstNode *try_body, const char *catch_var, AstNode *catch_body, AstNode *finally_body, size_t line, size_t column) {
    AstNode *n = ast_create(AST_TRY, line, column);
    n->as.try_stmt.try_body = try_body;
    n->as.try_stmt.catch_var = catch_var ? strdup(catch_var) : NULL;
    n->as.try_stmt.catch_body = catch_body;
    n->as.try_stmt.finally_body = finally_body;
    return n;
}

AstNode *ast_match(AstNode *expr, AstNode **cases, size_t case_count, size_t line, size_t column) {
    AstNode *n = ast_create(AST_MATCH, line, column);
    n->as.match_stmt.expr = expr;
    n->as.match_stmt.cases = cases;
    n->as.match_stmt.case_count = case_count;
    return n;
}

AstNode *ast_import(char **names, size_t count, size_t line, size_t column) {
    AstNode *n = ast_create(AST_IMPORT, line, column);
    n->as.import.names = names;
    n->as.import.count = count;
    return n;
}

/* Expressions */

AstNode *ast_number(double value, size_t line, size_t column) {
    AstNode *n = ast_create(AST_NUMBER, line, column);
    n->as.number.value = value;
    return n;
}

AstNode *ast_string(const char *value, size_t line, size_t column) {
    AstNode *n = ast_create(AST_STRING, line, column);
    n->as.string.value = strdup(value);
    return n;
}

AstNode *ast_bool(int value, size_t line, size_t column) {
    AstNode *n = ast_create(AST_BOOL, line, column);
    n->as.boolean.value = value;
    return n;
}

AstNode *ast_nil(size_t line, size_t column) {
    return ast_create(AST_NIL, line, column);
}

AstNode *ast_identifier(const char *name, size_t line, size_t column) {
    AstNode *n = ast_create(AST_IDENTIFIER, line, column);
    n->as.identifier.name = strdup(name);
    return n;
}

AstNode *ast_binary(int op, AstNode *left, AstNode *right, size_t line, size_t column) {
    AstNode *n = ast_create(AST_BINARY, line, column);
    n->as.binary.op = op;
    n->as.binary.left = left;
    n->as.binary.right = right;
    return n;
}

AstNode *ast_unary(int op, AstNode *operand, size_t line, size_t column) {
    AstNode *n = ast_create(AST_UNARY, line, column);
    n->as.unary.op = op;
    n->as.unary.operand = operand;
    return n;
}

AstNode *ast_call(AstNode *callee, AstNode **args, size_t arg_count, size_t line, size_t column) {
    AstNode *n = ast_create(AST_CALL, line, column);
    n->as.call.callee = callee;
    n->as.call.args = args;
    n->as.call.arg_count = arg_count;
    return n;
}

AstNode *ast_member(AstNode *object, const char *name, size_t line, size_t column) {
    AstNode *n = ast_create(AST_MEMBER, line, column);
    n->as.member.object = object;
    n->as.member.name = strdup(name);
    return n;
}

AstNode *ast_index(AstNode *object, AstNode *index, size_t line, size_t column) {
    AstNode *n = ast_create(AST_INDEX, line, column);
    n->as.index.object = object;
    n->as.index.index = index;
    return n;
}

AstNode *ast_conditional(AstNode *cond, AstNode *then_branch, AstNode *else_branch, size_t line, size_t column) {
    AstNode *n = ast_create(AST_CONDITIONAL, line, column);
    n->as.conditional.cond = cond;
    n->as.conditional.then_branch = then_branch;
    n->as.conditional.else_branch = else_branch;
    return n;
}

AstNode *ast_await(AstNode *value, size_t line, size_t column) {
    AstNode *n = ast_create(AST_AWAIT, line, column);
    n->as.await_expr.value = value;
    return n;
}

AstNode *ast_lambda(char **params, size_t param_count, AstNode *body, size_t line, size_t column) {
    AstNode *n = ast_create(AST_LAMBDA, line, column);
    n->as.lambda.params = params;
    n->as.lambda.param_count = param_count;
    n->as.lambda.body = body;
    return n;
}

AstNode *ast_list(AstNode **elements, size_t count, size_t line, size_t column) {
    AstNode *n = ast_create(AST_LIST, line, column);
    n->as.list.elements = elements;
    n->as.list.count = count;
    return n;
}

AstNode *ast_tuple(AstNode **elements, size_t count, size_t line, size_t column) {
    AstNode *n = ast_create(AST_TUPLE, line, column);
    n->as.tuple.elements = elements;
    n->as.tuple.count = count;
    return n;
}

AstNode *ast_dict(AstNode **entries, size_t count, size_t line, size_t column) {
    AstNode *n = ast_create(AST_DICT, line, column);
    n->as.dict.entries = entries;
    n->as.dict.count = count;
    return n;
}

AstNode *ast_dict_entry(AstNode *key, AstNode *value, size_t line, size_t column) {
    AstNode *n = ast_create(AST_DICT_ENTRY, line, column);
    n->as.dict_entry.key = key;
    n->as.dict_entry.value = value;
    return n;
}

AstNode *ast_comprehension(AstNode *expr, AstNode **clauses, size_t clause_count, size_t line, size_t column) {
    AstNode *n = ast_create(AST_COMPREHENSION, line, column);
    n->as.comprehension.expr = expr;
    n->as.comprehension.clauses = clauses;
    n->as.comprehension.clause_count = clause_count;
    return n;
}

AstNode *ast_for_clause(const char *var, AstNode *iter, size_t line, size_t column) {
    AstNode *n = ast_create(AST_FOR_CLAUSE, line, column);
    n->as.for_clause.var = var ? strdup(var) : NULL;
    n->as.for_clause.iter = iter;
    return n;
}

AstNode *ast_if_clause(AstNode *cond, size_t line, size_t column) {
    AstNode *n = ast_create(AST_IF_CLAUSE, line, column);
    n->as.if_clause.cond = cond;
    return n;
}

AstNode *ast_hpc(const char *kind, AstNode **specs, size_t spec_count, AstNode *body, size_t line, size_t column) {
    AstNode *n = ast_create(AST_HPC, line, column);
    n->as.hpc.kind = kind ? strdup(kind) : NULL;
    n->as.hpc.specs = specs;
    n->as.hpc.spec_count = spec_count;
    n->as.hpc.body = body;
    return n;
}

/* -------------------------------------------------------------------------- */
/* Free                                                                       */
/* -------------------------------------------------------------------------- */

void ast_free(AstNode *node) {
    if (!node) return;
    switch (node->type) {
        case AST_PROGRAM:
            ast_free(node->as.program.body);
            break;
        case AST_BLOCK:
            for (size_t i = 0; i < node->as.block.count; i++) ast_free(node->as.block.stmts[i]);
            free(node->as.block.stmts);
            break;
        case AST_EXPRESSION_STMT:
            ast_free(node->as.expression_stmt.expr);
            break;
        case AST_ASSIGN:
            ast_free(node->as.assign.target);
            ast_free(node->as.assign.value);
            break;
        case AST_IF:
            ast_free(node->as.if_stmt.cond);
            ast_free(node->as.if_stmt.then_branch);
            ast_free(node->as.if_stmt.else_branch);
            break;
        case AST_WHILE:
            ast_free(node->as.while_stmt.cond);
            ast_free(node->as.while_stmt.body);
            break;
        case AST_FOR:
            free(node->as.for_stmt.var);
            ast_free(node->as.for_stmt.iter);
            ast_free(node->as.for_stmt.body);
            break;
        case AST_RETURN:
            ast_free(node->as.return_stmt.value);
            break;
        case AST_YIELD:
            ast_free(node->as.yield_stmt.value);
            break;
        case AST_BREAK:
        case AST_CONTINUE:
            break;
        case AST_FUNCTION:
            free(node->as.function.name);
            for (size_t i = 0; i < node->as.function.param_count; i++) free(node->as.function.params[i]);
            free(node->as.function.params);
            ast_free(node->as.function.body);
            break;
        case AST_CLASS:
            free(node->as.class_def.name);
            for (size_t i = 0; i < node->as.class_def.method_count; i++) ast_free(node->as.class_def.methods[i]);
            free(node->as.class_def.methods);
            break;
        case AST_TRY:
            ast_free(node->as.try_stmt.try_body);
            free(node->as.try_stmt.catch_var);
            ast_free(node->as.try_stmt.catch_body);
            ast_free(node->as.try_stmt.finally_body);
            break;
        case AST_MATCH:
            ast_free(node->as.match_stmt.expr);
            for (size_t i = 0; i < node->as.match_stmt.case_count; i++) ast_free(node->as.match_stmt.cases[i]);
            free(node->as.match_stmt.cases);
            break;
        case AST_IMPORT:
            for (size_t i = 0; i < node->as.import.count; i++) free(node->as.import.names[i]);
            free(node->as.import.names);
            break;
        case AST_NUMBER:
            break;
        case AST_STRING:
            free(node->as.string.value);
            break;
        case AST_BOOL:
        case AST_NIL:
            break;
        case AST_IDENTIFIER:
            free(node->as.identifier.name);
            break;
        case AST_BINARY:
            ast_free(node->as.binary.left);
            ast_free(node->as.binary.right);
            break;
        case AST_UNARY:
            ast_free(node->as.unary.operand);
            break;
        case AST_CALL:
            ast_free(node->as.call.callee);
            for (size_t i = 0; i < node->as.call.arg_count; i++) ast_free(node->as.call.args[i]);
            free(node->as.call.args);
            break;
        case AST_MEMBER:
            ast_free(node->as.member.object);
            free(node->as.member.name);
            break;
        case AST_INDEX:
            ast_free(node->as.index.object);
            ast_free(node->as.index.index);
            break;
        case AST_CONDITIONAL:
            ast_free(node->as.conditional.cond);
            ast_free(node->as.conditional.then_branch);
            ast_free(node->as.conditional.else_branch);
            break;
        case AST_AWAIT:
            ast_free(node->as.await_expr.value);
            break;
        case AST_LAMBDA:
            for (size_t i = 0; i < node->as.lambda.param_count; i++) free(node->as.lambda.params[i]);
            free(node->as.lambda.params);
            ast_free(node->as.lambda.body);
            break;
        case AST_LIST:
            for (size_t i = 0; i < node->as.list.count; i++) ast_free(node->as.list.elements[i]);
            free(node->as.list.elements);
            break;
        case AST_TUPLE:
            for (size_t i = 0; i < node->as.tuple.count; i++) ast_free(node->as.tuple.elements[i]);
            free(node->as.tuple.elements);
            break;
        case AST_DICT:
            for (size_t i = 0; i < node->as.dict.count; i++) ast_free(node->as.dict.entries[i]);
            free(node->as.dict.entries);
            break;
        case AST_DICT_ENTRY:
            ast_free(node->as.dict_entry.key);
            ast_free(node->as.dict_entry.value);
            break;
        case AST_COMPREHENSION:
            ast_free(node->as.comprehension.expr);
            for (size_t i = 0; i < node->as.comprehension.clause_count; i++) ast_free(node->as.comprehension.clauses[i]);
            free(node->as.comprehension.clauses);
            break;
        case AST_FOR_CLAUSE:
            free(node->as.for_clause.var);
            ast_free(node->as.for_clause.iter);
            break;
        case AST_IF_CLAUSE:
            ast_free(node->as.if_clause.cond);
            break;
        case AST_HPC:
            free(node->as.hpc.kind);
            for (size_t i = 0; i < node->as.hpc.spec_count; i++) ast_free(node->as.hpc.specs[i]);
            free(node->as.hpc.specs);
            ast_free(node->as.hpc.body);
            break;
    }
    free(node);
}
