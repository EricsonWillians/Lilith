#define _GNU_SOURCE
#include "parser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

/* ========================================================================= */
/* Utility                                                                   */
/* ========================================================================= */

static char *token_to_string(Token token) {
    char *s = (char *)malloc(token.length + 1);
    if (!s) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    memcpy(s, token.lexeme, token.length);
    s[token.length] = '\0';
    return s;
}

/* ========================================================================= */
/* Error handling                                                            */
/* ========================================================================= */

static void error_at(Parser *p, Token *token, const char *message) {
    if (p->panic_mode) return;
    p->panic_mode = 1;
    p->had_error = 1;
    fprintf(stderr, "[parse error] line %zu, col %zu: %s\n",
            token->line, token->column, message);
}

static void error_at_current(Parser *p, const char *message) {
    error_at(p, &p->current, message);
}

/* ========================================================================= */
/* Token movement                                                            */
/* ========================================================================= */

static void advance(Parser *p) {
    p->previous = p->current;
    for (;;) {
        p->current = lexer_next_token(p->lexer);
        if (p->current.type != LILITH_TOKEN_ERROR) break;
        error_at_current(p, "Invalid token.");
    }
}

static int check(Parser *p, LilithTokenType type) {
    return p->current.type == type;
}

static int match(Parser *p, LilithTokenType type) {
    if (!check(p, type)) return 0;
    advance(p);
    return 1;
}

static Token consume(Parser *p, LilithTokenType type, const char *message) {
    if (check(p, type)) {
        advance(p);
        return p->previous;
    }
    error_at_current(p, message);
    return p->current;
}

static Token peek_next_token(Parser *p) {
    return lexer_peek_token(p->lexer);
}

static void synchronize(Parser *p) {
    p->panic_mode = 0;
    while (p->current.type != LILITH_TOKEN_EOF) {
        if (p->previous.type == LILITH_TOKEN_STMT_SEP) return;
        switch (p->current.type) {
            case LILITH_TOKEN_FUNC_DEF_START:
            case LILITH_TOKEN_CLASS_DEF_START:
            case LILITH_TOKEN_IF_START:
            case LILITH_TOKEN_WHILE_START:
            case LILITH_TOKEN_FOR_START:
            case LILITH_TOKEN_RETURN_START:
            case LILITH_TOKEN_TRY_START:
            case LILITH_TOKEN_MATCH_START:
            case LILITH_TOKEN_IMPORT_START:
                return;
            default:
                break;
        }
        advance(p);
    }
}

/* ========================================================================= */
/* Forward declarations                                                      */
/* ========================================================================= */

static AstNode *parse_expression(Parser *p);
static AstNode *parse_statement(Parser *p);
static AstNode *parse_block(Parser *p);
static AstNode *parse_primary(Parser *p);
static AstNode *parse_call(Parser *p);
static AstNode *parse_unary(Parser *p);
static AstNode *parse_factor(Parser *p);
static AstNode *parse_term(Parser *p);
static AstNode *parse_comparison(Parser *p);
static AstNode *parse_equality(Parser *p);
static AstNode *parse_and(Parser *p);
static AstNode *parse_or(Parser *p);
static AstNode *parse_conditional(Parser *p);

/* Unescape a string literal: \n, \t, \", \\\n   Input is the raw content without surrounding quotes. */
static char *unescape_string(const char *src) {
    size_t len = strlen(src);
    char *out = (char *)malloc(len + 1);
    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        if (src[i] == '\\' && i + 1 < len) {
            switch (src[i + 1]) {
                case 'n': out[j++] = '\n'; i++; break;
                case 't': out[j++] = '\t'; i++; break;
                case 'r': out[j++] = '\r'; i++; break;
                case '"': out[j++] = '"'; i++; break;
                case '\\': out[j++] = '\\'; i++; break;
                default: out[j++] = src[i]; break;
            }
        } else {
            out[j++] = src[i];
        }
    }
    out[j] = '\0';
    return out;
}

/* ========================================================================= */
/* Expression parsing                                                        */
/* ========================================================================= */

static AstNode *parse_expression(Parser *p) {
    return parse_conditional(p);
}

static AstNode *parse_conditional(Parser *p) {
    if (match(p, LILITH_TOKEN_IF_START)) {
        size_t line = p->previous.line;
        size_t col = p->previous.column;
        consume(p, LILITH_TOKEN_EXPR_GROUP_START, "Expected '((' after '[?' in conditional.");
        AstNode *cond = parse_expression(p);
        consume(p, LILITH_TOKEN_EXPR_GROUP_END, "Expected '))' after condition.");
        consume(p, LILITH_TOKEN_COND_THEN_GROUP_START, "Expected '[(' for then-branch.");
        AstNode *then_branch = parse_expression(p);
        consume(p, LILITH_TOKEN_COND_THEN_GROUP_END, "Expected ')]' after then-branch.");
        AstNode *else_branch = NULL;
        if (match(p, LILITH_TOKEN_ELSE_INTRO)) {
            consume(p, LILITH_TOKEN_COND_THEN_GROUP_START, "Expected '[(' for else-branch.");
            else_branch = parse_expression(p);
            consume(p, LILITH_TOKEN_COND_THEN_GROUP_END, "Expected ')]' after else-branch.");
        }
        consume(p, LILITH_TOKEN_IF_END, "Expected '?]' at end of conditional expression.");
        return ast_conditional(cond, then_branch, else_branch, line, col);
    }
    return parse_or(p);
}

static AstNode *parse_or(Parser *p) {
    return parse_and(p);
}

static AstNode *parse_and(Parser *p) {
    return parse_equality(p);
}

static AstNode *parse_equality(Parser *p) {
    AstNode *expr = parse_comparison(p);
    while (match(p, LILITH_TOKEN_EQEQ) || match(p, LILITH_TOKEN_BANG_EQ)) {
        int op = (p->previous.type == LILITH_TOKEN_EQEQ) ? OP_EQ : OP_NE;
        AstNode *right = parse_comparison(p);
        expr = ast_binary(op, expr, right, expr->line, expr->column);
    }
    return expr;
}

static AstNode *parse_comparison(Parser *p) {
    AstNode *expr = parse_term(p);
    while (match(p, LILITH_TOKEN_LESS_LESS) || match(p, LILITH_TOKEN_GREATER_GREATER)) {
        int op = (p->previous.type == LILITH_TOKEN_LESS_LESS) ? OP_LT : OP_GT;
        AstNode *right = parse_term(p);
        expr = ast_binary(op, expr, right, expr->line, expr->column);
    }
    return expr;
}

static AstNode *parse_term(Parser *p) {
    AstNode *expr = parse_factor(p);
    while (match(p, LILITH_TOKEN_PLUSPLUS) || match(p, LILITH_TOKEN_MINUSMINUS)) {
        int op = (p->previous.type == LILITH_TOKEN_PLUSPLUS) ? OP_ADD : OP_SUB;
        AstNode *right = parse_factor(p);
        expr = ast_binary(op, expr, right, expr->line, expr->column);
    }
    return expr;
}

static AstNode *parse_factor(Parser *p) {
    AstNode *expr = parse_unary(p);
    while (match(p, LILITH_TOKEN_ASTERISK_ASTERISK) ||
           match(p, LILITH_TOKEN_SLASH_SLASH) ||
           match(p, LILITH_TOKEN_PERCENT_PERCENT)) {
        int op;
        if (p->previous.type == LILITH_TOKEN_ASTERISK_ASTERISK) op = OP_MUL;
        else if (p->previous.type == LILITH_TOKEN_SLASH_SLASH) op = OP_DIV;
        else op = OP_MOD;
        AstNode *right = parse_unary(p);
        expr = ast_binary(op, expr, right, expr->line, expr->column);
    }
    return expr;
}

static AstNode *parse_unary(Parser *p) {
    if (match(p, LILITH_TOKEN_UNARY)) {
        AstNode *operand = parse_unary(p);
        return ast_unary(OP_NEG, operand, p->previous.line, p->previous.column);
    }
    return parse_call(p);
}

static AstNode *parse_call(Parser *p) {
    AstNode *expr = parse_primary(p);
    for (;;) {
        if (match(p, LILITH_TOKEN_EXPR_GROUP_START)) {
            AstNode **args = NULL;
            size_t count = 0;
            size_t cap = 0;
            while (!check(p, LILITH_TOKEN_EXPR_GROUP_END) && !check(p, LILITH_TOKEN_EOF)) {
                if (count >= cap) {
                    cap = cap < 4 ? 4 : cap * 2;
                    args = (AstNode **)realloc(args, sizeof(AstNode *) * cap);
                }
                args[count++] = parse_expression(p);
                if (!match(p, LILITH_TOKEN_COMMA_COMMA)) break;
            }
            consume(p, LILITH_TOKEN_EXPR_GROUP_END, "Expected '))' after arguments.");
            expr = ast_call(expr, args, count, expr->line, expr->column);
        } else if (match(p, LILITH_TOKEN_LBRACKET)) {
            AstNode *index = parse_expression(p);
            consume(p, LILITH_TOKEN_RBRACKET, "Expected ']' after index expression.");
            expr = ast_index(expr, index, expr->line, expr->column);
        } else if (match(p, LILITH_TOKEN_DOT)) {
            Token name = consume(p, LILITH_TOKEN_IDENTIFIER, "Expected property name after '.'.");
            char *name_str = token_to_string(name);
            expr = ast_member(expr, name_str, expr->line, expr->column);
            free(name_str);
        } else {
            break;
        }
    }
    return expr;
}

/* ========================================================================= */
/* Primary parsing                                                           */
/* ========================================================================= */

static AstNode *parse_list_literal(Parser *p);
static AstNode *parse_tuple_literal(Parser *p);
static AstNode *parse_dict_literal(Parser *p);
static AstNode *parse_set_literal(Parser *p);
static AstNode *parse_lambda(Parser *p);

static AstNode *parse_primary(Parser *p) {
    size_t line = p->current.line;
    size_t col = p->current.column;

    if (match(p, LILITH_TOKEN_NUMBER)) {
        char *s = token_to_string(p->previous);
        double val = strtod(s, NULL);
        free(s);
        return ast_number(val, line, col);
    }

    if (match(p, LILITH_TOKEN_STRING)) {
        char *s = token_to_string(p->previous);
        size_t len = strlen(s);
        char *inner;
        if (len >= 2 && s[0] == '"' && s[len - 1] == '"') {
            s[len - 1] = '\0';
            inner = unescape_string(s + 1);
        } else {
            inner = unescape_string(s);
        }
        free(s);
        AstNode *node = ast_string(inner, line, col);
        free(inner);
        return node;
    }

    if (match(p, LILITH_TOKEN_IDENTIFIER)) {
        char *name = token_to_string(p->previous);
        AstNode *node;
        if (strcmp(name, "true") == 0 || strcmp(name, "#true") == 0) {
            node = ast_bool(1, line, col);
        } else if (strcmp(name, "false") == 0 || strcmp(name, "#false") == 0) {
            node = ast_bool(0, line, col);
        } else if (strcmp(name, "nil") == 0) {
            node = ast_nil(line, col);
        } else {
            node = ast_identifier(name, line, col);
        }
        free(name);
        return node;
    }

    if (match(p, LILITH_TOKEN_EXPR_GROUP_START)) {
        AstNode *expr = parse_expression(p);
        consume(p, LILITH_TOKEN_EXPR_GROUP_END, "Expected '))' after expression.");
        return expr;
    }

    if (match(p, LILITH_TOKEN_LIST_LITERAL_START)) return parse_list_literal(p);
    if (match(p, LILITH_TOKEN_TUPLE_LITERAL_START)) return parse_tuple_literal(p);
    if (match(p, LILITH_TOKEN_DICT_LITERAL_START)) return parse_dict_literal(p);
    if (match(p, LILITH_TOKEN_SET_LITERAL_START)) return parse_set_literal(p);
    if (match(p, LILITH_TOKEN_LAMBDA_DEF_START)) return parse_lambda(p);
    if (match(p, LILITH_TOKEN_AWAIT_START)) {
        AstNode *expr = parse_expression(p);
        consume(p, LILITH_TOKEN_AWAIT_END, "Expected ')~' after await expression.");
        return ast_await(expr, line, col);
    }

    error_at_current(p, "Expected expression.");
    return ast_nil(line, col);
}

static AstNode *parse_comprehension_clause(Parser *p) {
    size_t line = p->current.line;
    size_t col = p->current.column;
    if (match(p, LILITH_TOKEN_FOR_CLAUSE_START)) {
        Token var = consume(p, LILITH_TOKEN_IDENTIFIER, "Expected iteration variable in for clause.");
        consume(p, LILITH_TOKEN_IN_OPERATOR, "Expected '[%]' after iteration variable.");
        AstNode *iter = parse_expression(p);
        consume(p, LILITH_TOKEN_FOR_CLAUSE_END, "Expected '>:]' after for clause iterable.");
        char *var_str = token_to_string(var);
        AstNode *node = ast_for_clause(var_str, iter, line, col);
        free(var_str);
        return node;
    }
    if (match(p, LILITH_TOKEN_IF_CLAUSE_START)) {
        AstNode *cond = parse_expression(p);
        consume(p, LILITH_TOKEN_IF_CLAUSE_END, "Expected ':?]' after if clause condition.");
        return ast_if_clause(cond, line, col);
    }
    if (match(p, LILITH_TOKEN_WHERE_CLAUSE_START)) {
        AstNode *cond = parse_expression(p);
        consume(p, LILITH_TOKEN_WHERE_CLAUSE_END, "Expected '|?]' after where clause condition.");
        return ast_if_clause(cond, line, col);
    }
    if (match(p, LILITH_TOKEN_GROUP_CLAUSE_START)) {
        AstNode *cond = parse_expression(p);
        consume(p, LILITH_TOKEN_GROUP_CLAUSE_END, "Expected '|&]' after group clause condition.");
        return ast_if_clause(cond, line, col);
    }
    return NULL;
}

static AstNode *parse_list_literal(Parser *p) {
    size_t line = p->previous.line;
    size_t col = p->previous.column;
    AstNode *first = parse_expression(p);
    AstNode **clauses = NULL;
    size_t clause_count = 0;
    size_t clause_cap = 0;
    while (!check(p, LILITH_TOKEN_LIST_LITERAL_END) && !check(p, LILITH_TOKEN_EOF)) {
        AstNode *clause = parse_comprehension_clause(p);
        if (clause) {
            if (clause_count >= clause_cap) {
                clause_cap = clause_cap < 4 ? 4 : clause_cap * 2;
                clauses = (AstNode **)realloc(clauses, sizeof(AstNode *) * clause_cap);
            }
            clauses[clause_count++] = clause;
        } else {
            break;
        }
    }
    if (clause_count > 0) {
        consume(p, LILITH_TOKEN_LIST_LITERAL_END, "Expected '>]' after list comprehension.");
        return ast_comprehension(first, clauses, clause_count, 0, line, col);
    }
    /* Regular list literal */
    AstNode **elements = NULL;
    size_t count = 0;
    size_t cap = 0;
    if (first) {
        elements = (AstNode **)malloc(sizeof(AstNode *) * 4);
        cap = 4;
        elements[count++] = first;
    }
    while (!check(p, LILITH_TOKEN_LIST_LITERAL_END) && !check(p, LILITH_TOKEN_EOF)) {
        if (!match(p, LILITH_TOKEN_COMMA_COMMA)) break;
        if (count >= cap) {
            cap = cap * 2;
            elements = (AstNode **)realloc(elements, sizeof(AstNode *) * cap);
        }
        elements[count++] = parse_expression(p);
    }
    consume(p, LILITH_TOKEN_LIST_LITERAL_END, "Expected '>]' after list elements.");
    return ast_list(elements, count, line, col);
}

static AstNode *parse_tuple_literal(Parser *p) {
    size_t line = p->previous.line;
    size_t col = p->previous.column;
    AstNode *first = parse_expression(p);
    AstNode **clauses = NULL;
    size_t clause_count = 0;
    size_t clause_cap = 0;
    while (!check(p, LILITH_TOKEN_TUPLE_LITERAL_END) && !check(p, LILITH_TOKEN_EOF)) {
        AstNode *clause = parse_comprehension_clause(p);
        if (clause) {
            if (clause_count >= clause_cap) {
                clause_cap = clause_cap < 4 ? 4 : clause_cap * 2;
                clauses = (AstNode **)realloc(clauses, sizeof(AstNode *) * clause_cap);
            }
            clauses[clause_count++] = clause;
        } else {
            break;
        }
    }
    if (clause_count > 0) {
        consume(p, LILITH_TOKEN_TUPLE_LITERAL_END, "Expected '>)' after tuple comprehension.");
        return ast_comprehension(first, clauses, clause_count, 1, line, col);
    }
    AstNode **elements = NULL;
    size_t count = 0;
    size_t cap = 0;
    if (first) {
        elements = (AstNode **)malloc(sizeof(AstNode *) * 4);
        cap = 4;
        elements[count++] = first;
    }
    while (!check(p, LILITH_TOKEN_TUPLE_LITERAL_END) && !check(p, LILITH_TOKEN_EOF)) {
        if (!match(p, LILITH_TOKEN_COMMA_COMMA)) break;
        if (count >= cap) {
            cap = cap * 2;
            elements = (AstNode **)realloc(elements, sizeof(AstNode *) * cap);
        }
        elements[count++] = parse_expression(p);
    }
    consume(p, LILITH_TOKEN_TUPLE_LITERAL_END, "Expected '>)' after tuple elements.");
    return ast_tuple(elements, count, line, col);
}

static AstNode *parse_dict_literal(Parser *p) {
    size_t line = p->previous.line;
    size_t col = p->previous.column;
    AstNode *first_key = parse_expression(p);
    consume(p, LILITH_TOKEN_DICT_MAP, "Expected '[:]' after dictionary key.");
    AstNode *first_value = parse_expression(p);
    AstNode **clauses = NULL;
    size_t clause_count = 0;
    size_t clause_cap = 0;
    while (!check(p, LILITH_TOKEN_DICT_LITERAL_END) && !check(p, LILITH_TOKEN_EOF)) {
        AstNode *clause = parse_comprehension_clause(p);
        if (clause) {
            if (clause_count >= clause_cap) {
                clause_cap = clause_cap < 4 ? 4 : clause_cap * 2;
                clauses = (AstNode **)realloc(clauses, sizeof(AstNode *) * clause_cap);
            }
            clauses[clause_count++] = clause;
        } else {
            break;
        }
    }
    if (clause_count > 0) {
        consume(p, LILITH_TOKEN_DICT_LITERAL_END, "Expected '>}' after dict comprehension.");
        return ast_dict_comprehension(first_key, first_value, clauses, clause_count, line, col);
    }
    AstNode **entries = NULL;
    size_t count = 0;
    size_t cap = 0;
    entries = (AstNode **)malloc(sizeof(AstNode *) * 4);
    cap = 4;
    entries[count++] = ast_dict_entry(first_key, first_value, line, col);
    while (!check(p, LILITH_TOKEN_DICT_LITERAL_END) && !check(p, LILITH_TOKEN_EOF)) {
        if (!match(p, LILITH_TOKEN_COMMA_COMMA)) break;
        AstNode *key = parse_expression(p);
        consume(p, LILITH_TOKEN_DICT_MAP, "Expected '[:]' after dictionary key.");
        AstNode *value = parse_expression(p);
        if (count >= cap) {
            cap = cap * 2;
            entries = (AstNode **)realloc(entries, sizeof(AstNode *) * cap);
        }
        entries[count++] = ast_dict_entry(key, value, line, col);
        if (!match(p, LILITH_TOKEN_COMMA_COMMA)) break;
    }
    consume(p, LILITH_TOKEN_DICT_LITERAL_END, "Expected '>}' after dictionary entries.");
    return ast_dict(entries, count, line, col);
}

static AstNode *parse_set_literal(Parser *p) {
    size_t line = p->previous.line;
    size_t col = p->previous.column;
    AstNode *first = parse_expression(p);
    AstNode **clauses = NULL;
    size_t clause_count = 0;
    size_t clause_cap = 0;
    while (!check(p, LILITH_TOKEN_SET_LITERAL_END) && !check(p, LILITH_TOKEN_EOF)) {
        AstNode *clause = parse_comprehension_clause(p);
        if (clause) {
            if (clause_count >= clause_cap) {
                clause_cap = clause_cap < 4 ? 4 : clause_cap * 2;
                clauses = (AstNode **)realloc(clauses, sizeof(AstNode *) * clause_cap);
            }
            clauses[clause_count++] = clause;
        } else {
            break;
        }
    }
    if (clause_count > 0) {
        consume(p, LILITH_TOKEN_SET_LITERAL_END, "Expected '}]' after set comprehension.");
        return ast_comprehension(first, clauses, clause_count, 2, line, col);
    }
    AstNode **elements = NULL;
    size_t count = 0;
    size_t cap = 0;
    if (first) {
        elements = (AstNode **)malloc(sizeof(AstNode *) * 4);
        cap = 4;
        elements[count++] = first;
    }
    while (!check(p, LILITH_TOKEN_SET_LITERAL_END) && !check(p, LILITH_TOKEN_EOF)) {
        if (!match(p, LILITH_TOKEN_COMMA_COMMA)) break;
        if (count >= cap) {
            cap = cap * 2;
            elements = (AstNode **)realloc(elements, sizeof(AstNode *) * cap);
        }
        elements[count++] = parse_expression(p);
    }
    consume(p, LILITH_TOKEN_SET_LITERAL_END, "Expected '}]' after set elements.");
    return ast_list(elements, count, line, col);
}

static AstNode *parse_lambda(Parser *p) {
    size_t line = p->previous.line;
    size_t col = p->previous.column;
    consume(p, LILITH_TOKEN_EXPR_GROUP_START, "Expected '((' to start lambda parameters.");
    char **params = NULL;
    size_t count = 0;
    size_t cap = 0;
    while (!check(p, LILITH_TOKEN_EXPR_GROUP_END) && !check(p, LILITH_TOKEN_EOF)) {
        Token param = consume(p, LILITH_TOKEN_IDENTIFIER, "Expected parameter name.");
        if (count >= cap) {
            cap = cap < 4 ? 4 : cap * 2;
            params = (char **)realloc(params, sizeof(char *) * cap);
        }
        params[count++] = token_to_string(param);
        if (!match(p, LILITH_TOKEN_COMMA_COMMA)) break;
    }
    consume(p, LILITH_TOKEN_EXPR_GROUP_END, "Expected '))' after lambda parameters.");
    AstNode *body = parse_block(p);
    consume(p, LILITH_TOKEN_LAMBDA_DEF_END, "Expected '>:)' after lambda body.");
    return ast_lambda(params, count, body, line, col);
}

/* ========================================================================= */
/* Statement parsing                                                         */
/* ========================================================================= */

static AstNode *parse_assignment(Parser *p);
static AstNode *parse_if_statement(Parser *p);
static AstNode *parse_while_statement(Parser *p);
static AstNode *parse_for_statement(Parser *p);
static AstNode *parse_return_statement(Parser *p);
static AstNode *parse_yield_statement(Parser *p);
static AstNode *parse_break_statement(Parser *p);
static AstNode *parse_continue_statement(Parser *p);
static AstNode *parse_function_statement(Parser *p);
static AstNode *parse_class_statement(Parser *p);
static AstNode *parse_try_statement(Parser *p);
static AstNode *parse_match_statement(Parser *p);
static AstNode *parse_import_statement(Parser *p);
static AstNode *parse_expression_statement(Parser *p);
static AstNode *parse_hpc_statement(Parser *p, LilithTokenType end_token, const char *kind);

static AstNode *parse_statement(Parser *p) {
    if (match(p, LILITH_TOKEN_IF_START)) return parse_if_statement(p);
    if (match(p, LILITH_TOKEN_WHILE_START)) return parse_while_statement(p);
    if (match(p, LILITH_TOKEN_FOR_START)) return parse_for_statement(p);
    if (match(p, LILITH_TOKEN_RETURN_START)) return parse_return_statement(p);
    if (match(p, LILITH_TOKEN_YIELD_START)) return parse_yield_statement(p);
    if (match(p, LILITH_TOKEN_BREAK)) return parse_break_statement(p);
    if (match(p, LILITH_TOKEN_CONTINUE)) return parse_continue_statement(p);
    if (match(p, LILITH_TOKEN_FUNC_DEF_START)) return parse_function_statement(p);
    if (match(p, LILITH_TOKEN_CLASS_DEF_START)) return parse_class_statement(p);
    if (match(p, LILITH_TOKEN_TRY_START)) return parse_try_statement(p);
    if (match(p, LILITH_TOKEN_MATCH_START)) return parse_match_statement(p);
    if (match(p, LILITH_TOKEN_IMPORT_START)) return parse_import_statement(p);
    if (match(p, LILITH_TOKEN_PARALLEL_START))
        return parse_hpc_statement(p, LILITH_TOKEN_PARALLEL_END, "parallel");
    if (match(p, LILITH_TOKEN_GPU_START))
        return parse_hpc_statement(p, LILITH_TOKEN_GPU_END, "gpu");
    if (match(p, LILITH_TOKEN_TENSOR_START))
        return parse_hpc_statement(p, LILITH_TOKEN_TENSOR_END, "tensor");
    if (match(p, LILITH_TOKEN_STREAM_START))
        return parse_hpc_statement(p, LILITH_TOKEN_STREAM_END, "stream");
    if (match(p, LILITH_TOKEN_MEMORY_START))
        return parse_hpc_statement(p, LILITH_TOKEN_MEMORY_END, "memory");

    return parse_expression_statement(p);
}

static AstNode *parse_stmt_list(Parser *p, LilithTokenType terminator) {
    size_t line = p->current.line;
    size_t col = p->current.column;

    AstNode **stmts = NULL;
    size_t count = 0;
    size_t capacity = 0;

    while (!check(p, terminator) && !check(p, LILITH_TOKEN_EOF)) {
        if (count >= capacity) {
            capacity = capacity < 8 ? 8 : capacity * 2;
            stmts = (AstNode **)realloc(stmts, sizeof(AstNode *) * capacity);
        }
        stmts[count++] = parse_statement(p);
        while (match(p, LILITH_TOKEN_STMT_SEP)) {}
    }

    return ast_block(stmts, count, line, col);
}

static AstNode *parse_block(Parser *p) {
    size_t line = p->current.line;
    size_t col = p->current.column;
    consume(p, LILITH_TOKEN_BLOCK_START, "Expected '[[' to start block.");
    AstNode *block = parse_stmt_list(p, LILITH_TOKEN_BLOCK_END);
    block->line = line;
    block->column = col;
    consume(p, LILITH_TOKEN_BLOCK_END, "Expected ']]' to end block.");
    return block;
}

static AstNode *parse_assignment(Parser *p) {
    size_t line = p->current.line;
    size_t col = p->current.column;
    Token ident = p->current;
    advance(p); /* consume identifier */
    consume(p, LILITH_TOKEN_ASSIGN, "Expected '[=]' after identifier in assignment.");
    AstNode *value = parse_expression(p);
    char *name = token_to_string(ident);
    AstNode *target = ast_identifier(name, ident.line, ident.column);
    free(name);
    return ast_assign(target, value, line, col);
}

static AstNode *parse_expression_statement(Parser *p) {
    size_t line = p->current.line;
    size_t col = p->current.column;
    AstNode *expr = parse_expression(p);
    if (match(p, LILITH_TOKEN_ASSIGN)) {
        AstNode *value = parse_expression(p);
        return ast_assign(expr, value, line, col);
    }
    return ast_expression_stmt(expr, line, col);
}

static AstNode *parse_if_statement(Parser *p) {
    size_t line = p->previous.line;
    size_t col = p->previous.column;
    consume(p, LILITH_TOKEN_EXPR_GROUP_START, "Expected '((' after '[?' in if statement.");
    AstNode *cond = parse_expression(p);
    consume(p, LILITH_TOKEN_EXPR_GROUP_END, "Expected '))' after condition.");
    AstNode *then_branch = parse_block(p);
    AstNode *else_branch = NULL;
    if (match(p, LILITH_TOKEN_ELSE_INTRO)) {
        else_branch = parse_block(p);
    }
    consume(p, LILITH_TOKEN_IF_END, "Expected '?]' at end of if statement.");
    return ast_if(cond, then_branch, else_branch, line, col);
}

static AstNode *parse_while_statement(Parser *p) {
    size_t line = p->previous.line;
    size_t col = p->previous.column;
    consume(p, LILITH_TOKEN_EXPR_GROUP_START, "Expected '((' after '<+' in while.");
    AstNode *cond = parse_expression(p);
    consume(p, LILITH_TOKEN_EXPR_GROUP_END, "Expected '))' after condition.");
    AstNode *body = parse_block(p);
    consume(p, LILITH_TOKEN_WHILE_END, "Expected '+>' at end of while loop.");
    return ast_while(cond, body, line, col);
}

static AstNode *parse_for_statement(Parser *p) {
    size_t line = p->previous.line;
    size_t col = p->previous.column;
    consume(p, LILITH_TOKEN_EXPR_GROUP_START, "Expected '((' after '<:' in for loop.");
    Token var = consume(p, LILITH_TOKEN_IDENTIFIER, "Expected iteration variable in for loop.");
    consume(p, LILITH_TOKEN_IN_OPERATOR, "Expected '[%]' after iteration variable.");
    AstNode *iter = parse_expression(p);
    consume(p, LILITH_TOKEN_EXPR_GROUP_END, "Expected '))' after for loop iterable.");
    AstNode *body = parse_block(p);
    consume(p, LILITH_TOKEN_FOR_END, "Expected ':>' at end of for loop.");
    char *var_str = token_to_string(var);
    AstNode *node = ast_for(var_str, iter, body, line, col);
    free(var_str);
    return node;
}

static AstNode *parse_return_statement(Parser *p) {
    size_t line = p->previous.line;
    size_t col = p->previous.column;
    AstNode *value = NULL;
    if (!check(p, LILITH_TOKEN_RETURN_END)) {
        value = parse_expression(p);
    }
    consume(p, LILITH_TOKEN_RETURN_END, "Expected '-(' after return value.");
    return ast_return(value, line, col);
}

static AstNode *parse_yield_statement(Parser *p) {
    size_t line = p->previous.line;
    size_t col = p->previous.column;
    AstNode *value = NULL;
    if (!check(p, LILITH_TOKEN_YIELD_END)) {
        value = parse_expression(p);
    }
    consume(p, LILITH_TOKEN_YIELD_END, "Expected '?-(' after yield value.");
    return ast_yield(value, line, col);
}

static AstNode *parse_break_statement(Parser *p) {
    return ast_break(p->previous.line, p->previous.column);
}

static AstNode *parse_continue_statement(Parser *p) {
    return ast_continue(p->previous.line, p->previous.column);
}

static AstNode *parse_function_statement(Parser *p) {
    size_t line = p->previous.line;
    size_t col = p->previous.column;
    int is_async = 0;
    if (match(p, LILITH_TOKEN_TILDE)) {
        is_async = 1;
    }
    Token name = consume(p, LILITH_TOKEN_IDENTIFIER, "Expected function name after '(|'.");
    consume(p, LILITH_TOKEN_EXPR_GROUP_START, "Expected '((' for parameter list.");
    char **params = NULL;
    char **param_types = NULL;
    size_t count = 0;
    size_t cap = 0;
    while (!check(p, LILITH_TOKEN_EXPR_GROUP_END) && !check(p, LILITH_TOKEN_EOF)) {
        Token param = consume(p, LILITH_TOKEN_IDENTIFIER, "Expected parameter name.");
        if (count >= cap) {
            cap = cap < 4 ? 4 : cap * 2;
            params = (char **)realloc(params, sizeof(char *) * cap);
            param_types = (char **)realloc(param_types, sizeof(char *) * cap);
        }
        params[count] = token_to_string(param);
        param_types[count] = NULL;
        if (match(p, LILITH_TOKEN_TYPE_ANNOT)) {
            Token type_tok = consume(p, LILITH_TOKEN_IDENTIFIER, "Expected type name after '(:)'.");
            param_types[count] = token_to_string(type_tok);
        }
        count++;
        if (!match(p, LILITH_TOKEN_COMMA_COMMA)) break;
    }
    consume(p, LILITH_TOKEN_EXPR_GROUP_END, "Expected '))' after parameters.");
    /* Optional return type */
    char *return_type = NULL;
    if (match(p, LILITH_TOKEN_ARROW)) {
        Token type_tok = consume(p, LILITH_TOKEN_IDENTIFIER, "Expected return type after '->'.");
        return_type = token_to_string(type_tok);
    }
    AstNode *body = parse_block(p);
    consume(p, LILITH_TOKEN_FUNC_DEF_END, "Expected '|)' after function body.");
    char *name_str = token_to_string(name);
    AstNode *node = ast_function(name_str, params, param_types, count, body, return_type, is_async, line, col);
    free(name_str);
    free(return_type);
    return node;
}

static AstNode *parse_class_statement(Parser *p) {
    size_t line = p->previous.line;
    size_t col = p->previous.column;
    Token name = consume(p, LILITH_TOKEN_IDENTIFIER, "Expected class name after '{|'.");
    /* Parse inheritance list if present */
    if (match(p, LILITH_TOKEN_INHERITANCE_START)) {
        while (!check(p, LILITH_TOKEN_INHERITANCE_END) && !check(p, LILITH_TOKEN_EOF)) advance(p);
        consume(p, LILITH_TOKEN_INHERITANCE_END, "Expected ':])' after inheritance list.");
    }
    AstNode *body = parse_block(p);
    consume(p, LILITH_TOKEN_CLASS_DEF_END, "Expected '|}' after class body.");
    char *name_str = token_to_string(name);
    /* ast_class will copy the method array, so we need to extract it from body */
    AstNode **methods = body->as.block.stmts;
    size_t method_count = body->as.block.count;
    body->as.block.stmts = NULL;
    body->as.block.count = 0;
    ast_free(body);
    AstNode *node = ast_class(name_str, methods, method_count, line, col);
    free(name_str);
    return node;
}

static AstNode *parse_try_statement(Parser *p) {
    size_t line = p->previous.line;
    size_t col = p->previous.column;
    AstNode *try_body = parse_block(p);
    char *catch_var = NULL;
    AstNode *catch_body = NULL;
    AstNode *finally_body = NULL;

    if (match(p, LILITH_TOKEN_EXCEPT_START)) {
        if (check(p, LILITH_TOKEN_IDENTIFIER)) {
            Token var = p->current;
            advance(p);
            catch_var = token_to_string(var);
        }
        consume(p, LILITH_TOKEN_EXCEPT_DIVIDER, "Expected '[/]' after exception variable.");
        catch_body = parse_block(p);
        consume(p, LILITH_TOKEN_EXCEPT_END, "Expected '!]' after except block.");
    }

    if (match(p, LILITH_TOKEN_FINALLY_START)) {
        finally_body = parse_block(p);
        consume(p, LILITH_TOKEN_FINALLY_END, "Expected '~:]' after finally block.");
    }

    consume(p, LILITH_TOKEN_TRY_END, "Expected '?}' at end of try statement.");
    return ast_try(try_body, catch_var, catch_body, finally_body, line, col);
}

static AstNode *parse_match_statement(Parser *p) {
    size_t line = p->previous.line;
    size_t col = p->previous.column;
    AstNode *expr = parse_expression(p);
    consume(p, LILITH_TOKEN_MATCH_END, "Expected '>-)' after match expression.");

    AstNode **cases = NULL;
    size_t count = 0;
    size_t cap = 0;

    while (check(p, LILITH_TOKEN_LIST_LITERAL_START)) {
        advance(p); /* consume [< */
        AstNode *pattern = parse_expression(p);
        consume(p, LILITH_TOKEN_LIST_LITERAL_END, "Expected '>]' after pattern.");
        AstNode *body = parse_block(p);
        if (count >= cap) {
            cap = cap < 4 ? 4 : cap * 2;
            cases = (AstNode **)realloc(cases, sizeof(AstNode *) * cap);
        }
        cases[count++] = ast_if(pattern, body, NULL, line, col);
    }

    return ast_match(expr, cases, count, line, col);
}

static AstNode *parse_import_statement(Parser *p) {
    size_t line = p->previous.line;
    size_t col = p->previous.column;
    char **names = NULL;
    size_t count = 0;
    size_t cap = 0;
    while (!check(p, LILITH_TOKEN_IMPORT_END) && !check(p, LILITH_TOKEN_EOF)) {
        Token name = consume(p, LILITH_TOKEN_IDENTIFIER, "Expected module name.");
        if (count >= cap) {
            cap = cap < 4 ? 4 : cap * 2;
            names = (char **)realloc(names, sizeof(char *) * cap);
        }
        names[count++] = token_to_string(name);
        if (!match(p, LILITH_TOKEN_COMMA_COMMA)) break;
    }
    consume(p, LILITH_TOKEN_IMPORT_END, "Expected '}>' after imports.");
    return ast_import(names, count, line, col);
}

static AstNode *parse_hpc_statement(Parser *p, LilithTokenType end_token, const char *kind) {
    size_t line = p->previous.line;
    size_t col = p->previous.column;
    AstNode **specs = NULL;
    size_t spec_count = 0;
    size_t spec_cap = 0;
    /* Optionally parse spec expression(s) inside ((...)) */
    if (match(p, LILITH_TOKEN_EXPR_GROUP_START)) {
        while (!check(p, LILITH_TOKEN_EXPR_GROUP_END) && !check(p, LILITH_TOKEN_EOF)) {
            if (spec_count >= spec_cap) {
                spec_cap = spec_cap < 4 ? 4 : spec_cap * 2;
                specs = (AstNode **)realloc(specs, sizeof(AstNode *) * spec_cap);
            }
            specs[spec_count++] = parse_expression(p);
            if (!match(p, LILITH_TOKEN_COMMA_COMMA)) break;
        }
        consume(p, LILITH_TOKEN_EXPR_GROUP_END, "Expected '))' after HPC spec.");
    }
    AstNode *body = parse_block(p);
    consume(p, end_token, "Expected HPC end token after body.");
    return ast_hpc(kind, specs, spec_count, body, line, col);
}

/* ========================================================================= */
/* Public API                                                                */
/* ========================================================================= */

AstNode *parser_parse(Lexer *lexer) {
    Parser parser;
    memset(&parser, 0, sizeof(parser));
    parser.lexer = lexer;
    advance(&parser);

    consume(&parser, LILITH_TOKEN_PROGRAM_START, "Expected '{[' at start of program.");
    AstNode *program_body = parse_stmt_list(&parser, LILITH_TOKEN_PROGRAM_END);
    consume(&parser, LILITH_TOKEN_PROGRAM_END, "Expected ']}' at end of program.");

    if (parser.had_error) {
        ast_free(program_body);
        return NULL;
    }
    return ast_program(program_body, program_body->line, program_body->column);
}
