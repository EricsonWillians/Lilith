#ifndef LILITH_AST_H
#define LILITH_AST_H

#include <stddef.h>

/* -------------------------------------------------------------------------- */
/* Node Types                                                                 */
/* -------------------------------------------------------------------------- */

typedef enum {
    AST_PROGRAM,
    AST_BLOCK,
    AST_EXPRESSION_STMT,
    AST_ASSIGN,
    AST_IF,
    AST_WHILE,
    AST_FOR,
    AST_RETURN,
    AST_YIELD,
    AST_BREAK,
    AST_CONTINUE,
    AST_FUNCTION,
    AST_CLASS,
    AST_TRY,
    AST_MATCH,
    AST_IMPORT,

    AST_NUMBER,
    AST_STRING,
    AST_BOOL,
    AST_NIL,
    AST_IDENTIFIER,
    AST_BINARY,
    AST_UNARY,
    AST_CALL,
    AST_MEMBER,
    AST_INDEX,
    AST_CONDITIONAL,
    AST_AWAIT,
    AST_LAMBDA,
    AST_LIST,
    AST_TUPLE,
    AST_DICT,
    AST_DICT_ENTRY,
    AST_COMPREHENSION,
    AST_FOR_CLAUSE,
    AST_IF_CLAUSE,
    AST_HPC,
} AstNodeType;

typedef enum {
    OP_ADD,    /* ++ */
    OP_SUB,    /* -- */
    OP_MUL,    /* ** */
    OP_DIV,    /* // */
    OP_MOD,    /* %% */
    OP_EQ,     /* == */
    OP_NE,     /* != */
    OP_LT,     /* << */
    OP_GT,     /* >> */
} BinaryOp;

typedef enum {
    OP_NEG,    /* :-: */
} UnaryOp;

/* -------------------------------------------------------------------------- */
/* Node Structure                                                             */
/* -------------------------------------------------------------------------- */

typedef struct AstNode AstNode;

struct AstNode {
    AstNodeType type;
    size_t line;
    size_t column;
    const char *filename;

    union {
        struct { AstNode *body; } program;
        struct { AstNode **stmts; size_t count; } block;
        struct { AstNode *expr; } expression_stmt;
        struct { AstNode *target; AstNode *value; } assign;
        struct { AstNode *cond; AstNode *then_branch; AstNode *else_branch; } if_stmt;
        struct { AstNode *cond; AstNode *body; } while_stmt;
        struct { char *var; AstNode *iter; AstNode *body; } for_stmt;
        struct { AstNode *value; } return_stmt;
        struct { AstNode *value; } yield_stmt;
        struct { char *name; char **params; size_t param_count; AstNode *body; int is_async; } function;
        struct { char *name; AstNode **methods; size_t method_count; } class_def;
        struct { AstNode *try_body; char *catch_var; AstNode *catch_body; AstNode *finally_body; } try_stmt;
        struct { AstNode *expr; AstNode **cases; size_t case_count; } match_stmt;
        struct { char **names; size_t count; } import;

        struct { double value; } number;
        struct { char *value; } string;
        struct { int value; } boolean;
        struct { char *name; } identifier;
        struct { int op; AstNode *left; AstNode *right; } binary;
        struct { int op; AstNode *operand; } unary;
        struct { AstNode *callee; AstNode **args; size_t arg_count; } call;
        struct { AstNode *object; char *name; } member;
        struct { AstNode *object; AstNode *index; } index;
        struct { AstNode *cond; AstNode *then_branch; AstNode *else_branch; } conditional;
        struct { AstNode *value; } await_expr;
        struct { char **params; size_t param_count; AstNode *body; } lambda;
        struct { AstNode **elements; size_t count; } list;
        struct { AstNode **elements; size_t count; } tuple;
        struct { AstNode **entries; size_t count; } dict;
        struct { AstNode *key; AstNode *value; } dict_entry;
        struct { AstNode *expr; AstNode **clauses; size_t clause_count; } comprehension;
        struct { char *var; AstNode *iter; } for_clause;
        struct { AstNode *cond; } if_clause;
        struct { char *kind; AstNode **specs; size_t spec_count; AstNode *body; } hpc;
    } as;
};

/* -------------------------------------------------------------------------- */
/* Constructors                                                               */
/* -------------------------------------------------------------------------- */

AstNode *ast_program(AstNode *body, size_t line, size_t column);
AstNode *ast_block(AstNode **stmts, size_t count, size_t line, size_t column);
AstNode *ast_expression_stmt(AstNode *expr, size_t line, size_t column);
AstNode *ast_assign(AstNode *target, AstNode *value, size_t line, size_t column);
AstNode *ast_if(AstNode *cond, AstNode *then_branch, AstNode *else_branch, size_t line, size_t column);
AstNode *ast_while(AstNode *cond, AstNode *body, size_t line, size_t column);
AstNode *ast_for(const char *var, AstNode *iter, AstNode *body, size_t line, size_t column);
AstNode *ast_return(AstNode *value, size_t line, size_t column);
AstNode *ast_yield(AstNode *value, size_t line, size_t column);
AstNode *ast_break(size_t line, size_t column);
AstNode *ast_continue(size_t line, size_t column);
AstNode *ast_function(const char *name, char **params, size_t param_count, AstNode *body, int is_async, size_t line, size_t column);
AstNode *ast_class(const char *name, AstNode **methods, size_t method_count, size_t line, size_t column);
AstNode *ast_try(AstNode *try_body, const char *catch_var, AstNode *catch_body, AstNode *finally_body, size_t line, size_t column);
AstNode *ast_match(AstNode *expr, AstNode **cases, size_t case_count, size_t line, size_t column);
AstNode *ast_import(char **names, size_t count, size_t line, size_t column);

AstNode *ast_number(double value, size_t line, size_t column);
AstNode *ast_string(const char *value, size_t line, size_t column);
AstNode *ast_bool(int value, size_t line, size_t column);
AstNode *ast_nil(size_t line, size_t column);
AstNode *ast_identifier(const char *name, size_t line, size_t column);
AstNode *ast_binary(int op, AstNode *left, AstNode *right, size_t line, size_t column);
AstNode *ast_unary(int op, AstNode *operand, size_t line, size_t column);
AstNode *ast_call(AstNode *callee, AstNode **args, size_t arg_count, size_t line, size_t column);
AstNode *ast_member(AstNode *object, const char *name, size_t line, size_t column);
AstNode *ast_index(AstNode *object, AstNode *index, size_t line, size_t column);
AstNode *ast_conditional(AstNode *cond, AstNode *then_branch, AstNode *else_branch, size_t line, size_t column);
AstNode *ast_await(AstNode *value, size_t line, size_t column);
AstNode *ast_lambda(char **params, size_t param_count, AstNode *body, size_t line, size_t column);
AstNode *ast_list(AstNode **elements, size_t count, size_t line, size_t column);
AstNode *ast_tuple(AstNode **elements, size_t count, size_t line, size_t column);
AstNode *ast_dict(AstNode **entries, size_t count, size_t line, size_t column);
AstNode *ast_dict_entry(AstNode *key, AstNode *value, size_t line, size_t column);
AstNode *ast_comprehension(AstNode *expr, AstNode **clauses, size_t clause_count, size_t line, size_t column);
AstNode *ast_for_clause(const char *var, AstNode *iter, size_t line, size_t column);
AstNode *ast_if_clause(AstNode *cond, size_t line, size_t column);
AstNode *ast_hpc(const char *kind, AstNode **specs, size_t spec_count, AstNode *body, size_t line, size_t column);

/* -------------------------------------------------------------------------- */
/* Memory Management                                                          */
/* -------------------------------------------------------------------------- */

void ast_free(AstNode *node);

#endif
