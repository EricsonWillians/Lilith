#define _GNU_SOURCE
#include "lexer.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

/* ========================================================================= */
/* Lexer state                                                               */
/* ========================================================================= */

struct Lexer {
    const char *source;
    char *filename;
    size_t pos;
    size_t length;
    size_t line;
    size_t column;
};

/* ========================================================================= */
/* Helpers                                                                   */
/* ========================================================================= */

static int is_at_end(Lexer *l) {
    return l->pos >= l->length;
}

static char current_char(Lexer *l) {
    return is_at_end(l) ? '\0' : l->source[l->pos];
}

static char peek_char(Lexer *l, size_t offset) {
    return (l->pos + offset >= l->length) ? '\0' : l->source[l->pos + offset];
}

static char advance(Lexer *l) {
    char c = current_char(l);
    l->pos++;
    if (c == '\n') {
        l->line++;
        l->column = 1;
    } else {
        l->column++;
    }
    return c;
}

static void skip_whitespace(Lexer *l) {
    while (!is_at_end(l)) {
        char c = current_char(l);
        if (c == ' ' || c == '\r' || c == '\t' || c == '\n') {
            advance(l);
        } else {
            break;
        }
    }
}

static void skip_comment(Lexer *l) {
    if (current_char(l) == '/' && peek_char(l, 1) == '*') {
        advance(l); /* '/' */
        advance(l); /* '*' */
        while (!is_at_end(l)) {
            if (current_char(l) == '*' && peek_char(l, 1) == '/') {
                advance(l);
                advance(l);
                break;
            }
            advance(l);
        }
    }
}

static void skip_whitespace_and_comments(Lexer *l) {
    for (;;) {
        skip_whitespace(l);
        if (current_char(l) == '/' && peek_char(l, 1) == '*') {
            skip_comment(l);
        } else {
            break;
        }
    }
}

static int match_string(Lexer *l, const char *str) {
    size_t len = strlen(str);
    if (l->pos + len > l->length) return 0;
    return (strncmp(l->source + l->pos, str, len) == 0);
}

static Token make_token(Lexer *l, LilithTokenType type, size_t start_pos,
                        size_t start_line, size_t start_column) {
    Token token;
    token.type = type;
    token.lexeme = l->source + start_pos;
    token.length = l->pos - start_pos;
    token.line = start_line;
    token.column = start_column;
    return token;
}

/* ========================================================================= */
/* Token mappings                                                            */
/* ========================================================================= */

/* Ordered so that longer tokens and tokens with common prefixes come first.
   Specifically, if token A is a prefix of token B, B must appear before A.
   We group by descending length for clarity.                     */

typedef struct {
    const char *str;
    LilithTokenType type;
} TokenMapping;

static const TokenMapping tokenMappings[] = {

    /* --------------------------------------------------------
       Length 8
       -------------------------------------------------------- */
    {"!COMPILE", LILITH_TOKEN_PHASE_COMPILE},
    {"!DYNAMIC", LILITH_TOKEN_ALLOC_DYNAMIC},
    {"!ISOLATE", LILITH_TOKEN_SCOPE_ISOLATE},
    {"!CAPTURE", LILITH_TOKEN_SCOPE_CAPTURE},

    /* --------------------------------------------------------
       Length 7
       -------------------------------------------------------- */
    {"!EXPAND",  LILITH_TOKEN_PHASE_EXPAND},
    {"!STATIC",  LILITH_TOKEN_ALLOC_STATIC},
    {"!SHARED",  LILITH_TOKEN_ALLOC_SHARED},
    {"!PINNED",  LILITH_TOKEN_ALLOC_PINNED},
    {"!INJECT",  LILITH_TOKEN_SCOPE_INJECT},

    /* --------------------------------------------------------
       Length 6
       -------------------------------------------------------- */
    {"!PARSE",   LILITH_TOKEN_PHASE_PARSE},

    /* --------------------------------------------------------
       Length 5  (none currently)
       -------------------------------------------------------- */

    /* --------------------------------------------------------
       Length 4
       -------------------------------------------------------- */
    {"!GPU",     LILITH_TOKEN_DEVICE_GPU},
    {"!TPU",     LILITH_TOKEN_DEVICE_TPU},
    {"!CPU",     LILITH_TOKEN_DEVICE_CPU},
    {"!DSP",     LILITH_TOKEN_DEVICE_DSP},

    /* --------------------------------------------------------
       Length 3
       -------------------------------------------------------- */
    /* Program / block structure */
    {"{[",       LILITH_TOKEN_PROGRAM_START},
    {"]}",       LILITH_TOKEN_PROGRAM_END},
    {"[[",       LILITH_TOKEN_BLOCK_START},
    {"]]",       LILITH_TOKEN_BLOCK_END},
    {"][",       LILITH_TOKEN_STMT_SEP},

    /* Assignment */
    {"[=]",      LILITH_TOKEN_ASSIGN},

    /* Control flow */
    {")-?",      LILITH_TOKEN_YIELD_START},
    {"?-(",      LILITH_TOKEN_YIELD_END},
    {"]-!",      LILITH_TOKEN_BREAK},
    {"]-?",      LILITH_TOKEN_CONTINUE},
    {":-:",      LILITH_TOKEN_UNARY},
    {":|:",      LILITH_TOKEN_ELSE_INTRO},

    /* Exception handling */
    {"[:~",      LILITH_TOKEN_FINALLY_START},
    {"~:]",      LILITH_TOKEN_FINALLY_END},
    {"[/]",      LILITH_TOKEN_EXCEPT_DIVIDER},

    /* Pattern matching */
    {"(-<",      LILITH_TOKEN_MATCH_START},
    {">-)",      LILITH_TOKEN_MATCH_END},

    /* Function / class */
    {"(:)",      LILITH_TOKEN_TYPE_ANNOT},
    {"([:",      LILITH_TOKEN_INHERITANCE_START},
    {":])",      LILITH_TOKEN_INHERITANCE_END},

    /* Collections & comprehensions */
    {"[:<",      LILITH_TOKEN_FOR_CLAUSE_START},
    {">:]",      LILITH_TOKEN_FOR_CLAUSE_END},
    {"[%]",      LILITH_TOKEN_IN_OPERATOR},
    {"[?:",      LILITH_TOKEN_IF_CLAUSE_START},
    {":?]",      LILITH_TOKEN_IF_CLAUSE_END},
    {"[?|",      LILITH_TOKEN_WHERE_CLAUSE_START},
    {"|?]",      LILITH_TOKEN_WHERE_CLAUSE_END},
    {"[&|",      LILITH_TOKEN_GROUP_CLAUSE_START},
    {"|&]",      LILITH_TOKEN_GROUP_CLAUSE_END},
    {"[:]",      LILITH_TOKEN_DICT_MAP},

    /* Lambda */
    {"(:<",      LILITH_TOKEN_LAMBDA_DEF_START},
    {">:)",      LILITH_TOKEN_LAMBDA_DEF_END},

    /* Macro system */
    {"<%|",      LILITH_TOKEN_MACRO_DEF_START},
    {"|%>",      LILITH_TOKEN_MACRO_DEF_END},
    {"`[",       LILITH_TOKEN_QUOTE_START},
    {"]`",       LILITH_TOKEN_QUOTE_END},
    {",[",       LILITH_TOKEN_UNQUOTE_START},
    {"],",       LILITH_TOKEN_UNQUOTE_END},
    {"{#",       LILITH_TOKEN_PATTERN_MATCH_START},
    {"#}",       LILITH_TOKEN_PATTERN_MATCH_END},
    {"(@",       LILITH_TOKEN_NODE_MATCH_START},
    {"@)",       LILITH_TOKEN_NODE_MATCH_END},
    {"{@",       LILITH_TOKEN_AST_TRANSFORM_START},
    {"@}",       LILITH_TOKEN_AST_TRANSFORM_END},
    {"=>>",      LILITH_TOKEN_TRANSFORM_ARROW},
    {"[#",       LILITH_TOKEN_TENSOR_START},
    {"#]",       LILITH_TOKEN_TENSOR_END},
    {"<%%",      LILITH_TOKEN_SYM_EXPR_START},
    {"%%>",      LILITH_TOKEN_SYM_EXPR_END},
    {"<@",       LILITH_TOKEN_COMPILE_EXPR_START},
    {"@>",       LILITH_TOKEN_COMPILE_EXPR_END},

    /* Parallel / GPU / Tensor / Stream / Memory */
    {"<|",       LILITH_TOKEN_PARALLEL_START},
    {"|>",       LILITH_TOKEN_PARALLEL_END},
    {"<%",       LILITH_TOKEN_GPU_START},
    {"%>",       LILITH_TOKEN_GPU_END},
    {"[^",       LILITH_TOKEN_MEMORY_START},
    {"^]",       LILITH_TOKEN_MEMORY_END},
    {"<~",       LILITH_TOKEN_STREAM_START},
    {"~>",       LILITH_TOKEN_STREAM_END},

    /* Neural network / HPC specifiers */
    {"[L",       LILITH_TOKEN_LAYER_SPEC_START},
    {"[C",       LILITH_TOKEN_LAYER_CONV_START},
    {"[P",       LILITH_TOKEN_LAYER_POOL_START},
    {"[N",       LILITH_TOKEN_LAYER_NORM_START},
    {"[S",       LILITH_TOKEN_SYNC_START},
    {"[K",       LILITH_TOKEN_KERNEL_START},
    {"[G",       LILITH_TOKEN_GRID_START},
    {"[D",       LILITH_TOKEN_DEVICE_START},
    {"[W",       LILITH_TOKEN_WINDOW_START},

    /* --------------------------------------------------------
       Length 2
       -------------------------------------------------------- */
    /* Expression grouping */
    {"((",       LILITH_TOKEN_EXPR_GROUP_START},
    {"))",       LILITH_TOKEN_EXPR_GROUP_END},

    /* Control flow */
    {"[?",       LILITH_TOKEN_IF_START},
    {"?]",       LILITH_TOKEN_IF_END},
    {"<+",       LILITH_TOKEN_WHILE_START},
    {"+>",       LILITH_TOKEN_WHILE_END},
    {")-",       LILITH_TOKEN_RETURN_START},
    {"-(",       LILITH_TOKEN_RETURN_END},

    /* Exception handling */
    {"{?",       LILITH_TOKEN_TRY_START},
    {"?}",       LILITH_TOKEN_TRY_END},
    {"[!",       LILITH_TOKEN_EXCEPT_START},
    {"!]",       LILITH_TOKEN_EXCEPT_END},

    /* Function / class */
    {"(|",       LILITH_TOKEN_FUNC_DEF_START},
    {"|)",       LILITH_TOKEN_FUNC_DEF_END},
    {"{|",       LILITH_TOKEN_CLASS_DEF_START},
    {"|}",       LILITH_TOKEN_CLASS_DEF_END},
    {"->",       LILITH_TOKEN_ARROW},

    /* Collections (note: [< and >] are also used for case patterns) */
    {"[<",       LILITH_TOKEN_LIST_LITERAL_START},
    {">]",       LILITH_TOKEN_LIST_LITERAL_END},
    {"(<",       LILITH_TOKEN_TUPLE_LITERAL_START},
    {">)",       LILITH_TOKEN_TUPLE_LITERAL_END},
    {"{<",       LILITH_TOKEN_DICT_LITERAL_START},
    {">}",       LILITH_TOKEN_DICT_LITERAL_END},
    {"[{",       LILITH_TOKEN_SET_LITERAL_START},
    {"}]",       LILITH_TOKEN_SET_LITERAL_END},

    /* Conditional then-group */
    {"[(",       LILITH_TOKEN_COND_THEN_GROUP_START},
    {")]",       LILITH_TOKEN_COND_THEN_GROUP_END},

    /* Import */
    {"<{",       LILITH_TOKEN_IMPORT_START},
    {"}>",       LILITH_TOKEN_IMPORT_END},

    /* Operators */
    {"++",       LILITH_TOKEN_PLUSPLUS},
    {"--",       LILITH_TOKEN_MINUSMINUS},
    {"**",       LILITH_TOKEN_ASTERISK_ASTERISK},
    {"//",       LILITH_TOKEN_SLASH_SLASH},
    {"%%",       LILITH_TOKEN_PERCENT_PERCENT},
    {"==",       LILITH_TOKEN_EQEQ},
    {"!=",       LILITH_TOKEN_BANG_EQ},
    {"<<",       LILITH_TOKEN_LESS_LESS},
    {">>",       LILITH_TOKEN_GREATER_GREATER},
    {",,",       LILITH_TOKEN_COMMA_COMMA},

    /* Async */
    {"~(",       LILITH_TOKEN_AWAIT_START},
    {")~",       LILITH_TOKEN_AWAIT_END},

    /* Misc */
    {"..",       LILITH_TOKEN_DOT_DOT},

    /* --------------------------------------------------------
       Length 1 — only tokens for chars that cannot appear in identifiers
       -------------------------------------------------------- */
    {"~",        LILITH_TOKEN_TILDE},
    {".",        LILITH_TOKEN_DOT},
    {"[",        LILITH_TOKEN_LBRACKET},
    {"]",        LILITH_TOKEN_RBRACKET},
};

static const size_t tokenMappingsCount = sizeof(tokenMappings) / sizeof(tokenMappings[0]);

/* ========================================================================= */
/* Literals and identifiers                                                  */
/* ========================================================================= */

static int is_digit(char c) {
    return c >= '0' && c <= '9';
}

/* Identifier charset: grammar sym chars + alphabetic + digits.
   Digits are allowed inside identifiers but a leading digit is
   parsed as a number literal.                           */
static int is_allowed_sym(char c) {
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) return 1;
    if (c >= '0' && c <= '9') return 1;
    const char *allowed = "!@#$%^&*_=+-/?:";
    return (strchr(allowed, c) != NULL);
}

static Token read_number(Lexer *l, size_t start_pos, size_t start_line, size_t start_column) {
    while (is_digit(current_char(l))) advance(l);
    return make_token(l, LILITH_TOKEN_NUMBER, start_pos, start_line, start_column);
}

static Token read_string(Lexer *l, size_t start_pos, size_t start_line, size_t start_column) {
    advance(l); /* opening quote */
    while (!is_at_end(l) && current_char(l) != '"') {
        advance(l);
    }
    if (is_at_end(l)) {
        return make_token(l, LILITH_TOKEN_ERROR, start_pos, start_line, start_column);
    }
    advance(l); /* closing quote */
    return make_token(l, LILITH_TOKEN_STRING, start_pos, start_line, start_column);
}

/* Read an identifier: consume allowed chars until we hit a reserved
   token prefix or a non-allowed char.  The '..' sequence is allowed
   inside identifiers as a namespace separator.              */
static Token read_identifier(Lexer *l, size_t start_pos, size_t start_line, size_t start_column) {
    while (!is_at_end(l)) {
        /* Allow '..' namespace separator inside identifiers */
        if (current_char(l) == '.' && peek_char(l, 1) == '.') {
            advance(l);
            advance(l);
            continue;
        }
        if (!is_allowed_sym(current_char(l))) break;
        int reserved = 0;
        for (size_t i = 0; i < tokenMappingsCount; i++) {
            if (match_string(l, tokenMappings[i].str)) {
                reserved = 1;
                break;
            }
        }
        if (reserved) break;
        advance(l);
    }
    return make_token(l, LILITH_TOKEN_IDENTIFIER, start_pos, start_line, start_column);
}

/* ========================================================================= */
/* Reserved token matching                                                   */
/* ========================================================================= */

static int try_match_reserved(Lexer *l, Token *token_out) {
    for (size_t i = 0; i < tokenMappingsCount; i++) {
        const char *mappingStr = tokenMappings[i].str;
        size_t len = strlen(mappingStr);
        if (l->pos + len <= l->length &&
            strncmp(l->source + l->pos, mappingStr, len) == 0) {
            size_t startPos = l->pos;
            size_t startLine = l->line;
            size_t startColumn = l->column;
            for (size_t j = 0; j < len; j++) {
                advance(l);
            }
            *token_out = make_token(l, tokenMappings[i].type, startPos, startLine, startColumn);
            return 1;
        }
    }
    return 0;
}

/* ========================================================================= */
/* Public API                                                                */
/* ========================================================================= */

Lexer *lexer_create(const char *source, const char *filename) {
    if (!source) return NULL;
    Lexer *l = (Lexer *)malloc(sizeof(Lexer));
    if (!l) return NULL;
    l->source = source;
    l->length = strlen(source);
    l->pos = 0;
    l->line = 1;
    l->column = 1;
    l->filename = filename ? strdup(filename) : NULL;
    return l;
}

void lexer_destroy(Lexer *l) {
    if (!l) return;
    free(l->filename);
    free(l);
}

Token lexer_next_token(Lexer *l) {
    skip_whitespace_and_comments(l);
    size_t startPos = l->pos;
    size_t startLine = l->line;
    size_t startColumn = l->column;

    if (is_at_end(l)) {
        return make_token(l, LILITH_TOKEN_EOF, l->pos, l->line, l->column);
    }

    Token token;
    if (try_match_reserved(l, &token)) {
        return token;
    }

    char c = current_char(l);
    if (is_digit(c)) {
        return read_number(l, startPos, startLine, startColumn);
    }
    if (c == '"') {
        return read_string(l, startPos, startLine, startColumn);
    }
    if (is_allowed_sym(c)) {
        return read_identifier(l, startPos, startLine, startColumn);
    }

    advance(l);
    return make_token(l, LILITH_TOKEN_ERROR, startPos, startLine, startColumn);
}

Token lexer_peek_token(Lexer *l) {
    Lexer backup = *l;
    Token token = lexer_next_token(l);
    *l = backup;
    return token;
}

void lexer_reset(Lexer *l) {
    if (!l) return;
    l->pos = 0;
    l->line = 1;
    l->column = 1;
}

/* ========================================================================= */
/* Token type to string                                                      */
/* ========================================================================= */

const char *lilith_token_type_to_string(LilithTokenType type) {
    switch (type) {
        case LILITH_TOKEN_EOF:               return "EOF";
        case LILITH_TOKEN_ERROR:             return "ERROR";
        case LILITH_TOKEN_PROGRAM_START:     return "PROGRAM_START";
        case LILITH_TOKEN_PROGRAM_END:       return "PROGRAM_END";
        case LILITH_TOKEN_BLOCK_START:       return "BLOCK_START";
        case LILITH_TOKEN_BLOCK_END:         return "BLOCK_END";
        case LILITH_TOKEN_STMT_SEP:          return "STMT_SEP";
        case LILITH_TOKEN_ASSIGN:            return "ASSIGN";
        case LILITH_TOKEN_EXPR_GROUP_START:  return "EXPR_GROUP_START";
        case LILITH_TOKEN_EXPR_GROUP_END:    return "EXPR_GROUP_END";
        case LILITH_TOKEN_IF_START:          return "IF_START";
        case LILITH_TOKEN_IF_END:            return "IF_END";
        case LILITH_TOKEN_ELSE_INTRO:        return "ELSE_INTRO";
        case LILITH_TOKEN_WHILE_START:       return "WHILE_START";
        case LILITH_TOKEN_WHILE_END:         return "WHILE_END";
        case LILITH_TOKEN_FUNC_DEF_START:    return "FUNC_DEF_START";
        case LILITH_TOKEN_FUNC_DEF_END:      return "FUNC_DEF_END";
        case LILITH_TOKEN_CLASS_DEF_START:   return "CLASS_DEF_START";
        case LILITH_TOKEN_CLASS_DEF_END:     return "CLASS_DEF_END";
        case LILITH_TOKEN_RETURN_START:      return "RETURN_START";
        case LILITH_TOKEN_RETURN_END:        return "RETURN_END";
        case LILITH_TOKEN_YIELD_START:       return "YIELD_START";
        case LILITH_TOKEN_YIELD_END:         return "YIELD_END";
        case LILITH_TOKEN_BREAK:             return "BREAK";
        case LILITH_TOKEN_CONTINUE:          return "CONTINUE";
        case LILITH_TOKEN_TRY_START:         return "TRY_START";
        case LILITH_TOKEN_TRY_END:           return "TRY_END";
        case LILITH_TOKEN_EXCEPT_START:      return "EXCEPT_START";
        case LILITH_TOKEN_EXCEPT_END:        return "EXCEPT_END";
        case LILITH_TOKEN_EXCEPT_DIVIDER:    return "EXCEPT_DIVIDER";
        case LILITH_TOKEN_FINALLY_START:     return "FINALLY_START";
        case LILITH_TOKEN_FINALLY_END:       return "FINALLY_END";
        case LILITH_TOKEN_MATCH_START:       return "MATCH_START";
        case LILITH_TOKEN_MATCH_END:         return "MATCH_END";
        case LILITH_TOKEN_ARROW:             return "ARROW";
        case LILITH_TOKEN_TYPE_ANNOT:        return "TYPE_ANNOT";
        case LILITH_TOKEN_INHERITANCE_START: return "INHERITANCE_START";
        case LILITH_TOKEN_INHERITANCE_END:   return "INHERITANCE_END";
        case LILITH_TOKEN_MACRO_DEF_START:   return "MACRO_DEF_START";
        case LILITH_TOKEN_MACRO_DEF_END:     return "MACRO_DEF_END";
        case LILITH_TOKEN_AWAIT_START:       return "AWAIT_START";
        case LILITH_TOKEN_AWAIT_END:         return "AWAIT_END";
        case LILITH_TOKEN_TILDE:             return "TILDE";
        case LILITH_TOKEN_COND_THEN_GROUP_START: return "COND_THEN_GROUP_START";
        case LILITH_TOKEN_COND_THEN_GROUP_END:   return "COND_THEN_GROUP_END";
        case LILITH_TOKEN_LIST_LITERAL_START:    return "LIST_LITERAL_START";
        case LILITH_TOKEN_LIST_LITERAL_END:      return "LIST_LITERAL_END";
        case LILITH_TOKEN_TUPLE_LITERAL_START:   return "TUPLE_LITERAL_START";
        case LILITH_TOKEN_TUPLE_LITERAL_END:     return "TUPLE_LITERAL_END";
        case LILITH_TOKEN_DICT_LITERAL_START:    return "DICT_LITERAL_START";
        case LILITH_TOKEN_DICT_LITERAL_END:      return "DICT_LITERAL_END";
        case LILITH_TOKEN_SET_LITERAL_START:     return "SET_LITERAL_START";
        case LILITH_TOKEN_SET_LITERAL_END:       return "SET_LITERAL_END";
        case LILITH_TOKEN_DICT_MAP:              return "DICT_MAP";
        case LILITH_TOKEN_FOR_CLAUSE_START:      return "FOR_CLAUSE_START";
        case LILITH_TOKEN_FOR_CLAUSE_END:        return "FOR_CLAUSE_END";
        case LILITH_TOKEN_IN_OPERATOR:           return "IN_OPERATOR";
        case LILITH_TOKEN_IF_CLAUSE_START:       return "IF_CLAUSE_START";
        case LILITH_TOKEN_IF_CLAUSE_END:         return "IF_CLAUSE_END";
        case LILITH_TOKEN_WHERE_CLAUSE_START:    return "WHERE_CLAUSE_START";
        case LILITH_TOKEN_WHERE_CLAUSE_END:      return "WHERE_CLAUSE_END";
        case LILITH_TOKEN_GROUP_CLAUSE_START:    return "GROUP_CLAUSE_START";
        case LILITH_TOKEN_GROUP_CLAUSE_END:      return "GROUP_CLAUSE_END";
        case LILITH_TOKEN_LAMBDA_DEF_START:      return "LAMBDA_DEF_START";
        case LILITH_TOKEN_LAMBDA_DEF_END:        return "LAMBDA_DEF_END";
        case LILITH_TOKEN_PLUSPLUS:              return "PLUSPLUS";
        case LILITH_TOKEN_MINUSMINUS:            return "MINUSMINUS";
        case LILITH_TOKEN_ASTERISK_ASTERISK:     return "ASTERISK_ASTERISK";
        case LILITH_TOKEN_SLASH_SLASH:           return "SLASH_SLASH";
        case LILITH_TOKEN_PERCENT_PERCENT:       return "PERCENT_PERCENT";
        case LILITH_TOKEN_EQEQ:                  return "EQEQ";
        case LILITH_TOKEN_BANG_EQ:               return "BANG_EQ";
        case LILITH_TOKEN_LESS_LESS:             return "LESS_LESS";
        case LILITH_TOKEN_GREATER_GREATER:       return "GREATER_GREATER";
        case LILITH_TOKEN_UNARY:                 return "UNARY";
        case LILITH_TOKEN_COMMA_COMMA:           return "COMMA_COMMA";
        case LILITH_TOKEN_IMPORT_START:          return "IMPORT_START";
        case LILITH_TOKEN_IMPORT_END:            return "IMPORT_END";
        case LILITH_TOKEN_DOT_DOT:               return "DOT_DOT";
        case LILITH_TOKEN_DOT:                   return "DOT";
        case LILITH_TOKEN_LBRACKET:              return "LBRACKET";
        case LILITH_TOKEN_RBRACKET:              return "RBRACKET";
        case LILITH_TOKEN_QUOTE_START:           return "QUOTE_START";
        case LILITH_TOKEN_QUOTE_END:             return "QUOTE_END";
        case LILITH_TOKEN_UNQUOTE_START:         return "UNQUOTE_START";
        case LILITH_TOKEN_UNQUOTE_END:           return "UNQUOTE_END";
        case LILITH_TOKEN_PATTERN_MATCH_START:   return "PATTERN_MATCH_START";
        case LILITH_TOKEN_PATTERN_MATCH_END:     return "PATTERN_MATCH_END";
        case LILITH_TOKEN_NODE_MATCH_START:      return "NODE_MATCH_START";
        case LILITH_TOKEN_NODE_MATCH_END:        return "NODE_MATCH_END";
        case LILITH_TOKEN_AST_TRANSFORM_START:   return "AST_TRANSFORM_START";
        case LILITH_TOKEN_AST_TRANSFORM_END:     return "AST_TRANSFORM_END";
        case LILITH_TOKEN_TRANSFORM_ARROW:       return "TRANSFORM_ARROW";
        case LILITH_TOKEN_PHASE_PARSE:           return "PHASE_PARSE";
        case LILITH_TOKEN_PHASE_EXPAND:          return "PHASE_EXPAND";
        case LILITH_TOKEN_PHASE_COMPILE:         return "PHASE_COMPILE";
        case LILITH_TOKEN_SCOPE_CAPTURE:         return "SCOPE_CAPTURE";
        case LILITH_TOKEN_SCOPE_ISOLATE:         return "SCOPE_ISOLATE";
        case LILITH_TOKEN_SCOPE_INJECT:          return "SCOPE_INJECT";
        case LILITH_TOKEN_ALLOC_STATIC:          return "ALLOC_STATIC";
        case LILITH_TOKEN_ALLOC_DYNAMIC:         return "ALLOC_DYNAMIC";
        case LILITH_TOKEN_ALLOC_SHARED:          return "ALLOC_SHARED";
        case LILITH_TOKEN_ALLOC_PINNED:          return "ALLOC_PINNED";
        case LILITH_TOKEN_PARALLEL_START:        return "PARALLEL_START";
        case LILITH_TOKEN_PARALLEL_END:          return "PARALLEL_END";
        case LILITH_TOKEN_SYNC_START:            return "SYNC_START";
        case LILITH_TOKEN_GPU_START:             return "GPU_START";
        case LILITH_TOKEN_GPU_END:               return "GPU_END";
        case LILITH_TOKEN_KERNEL_START:          return "KERNEL_START";
        case LILITH_TOKEN_GRID_START:            return "GRID_START";
        case LILITH_TOKEN_TENSOR_START:          return "TENSOR_START";
        case LILITH_TOKEN_TENSOR_END:            return "TENSOR_END";
        case LILITH_TOKEN_DEVICE_START:          return "DEVICE_START";
        case LILITH_TOKEN_DEVICE_GPU:            return "DEVICE_GPU";
        case LILITH_TOKEN_DEVICE_TPU:            return "DEVICE_TPU";
        case LILITH_TOKEN_DEVICE_CPU:            return "DEVICE_CPU";
        case LILITH_TOKEN_DEVICE_DSP:            return "DEVICE_DSP";
        case LILITH_TOKEN_LAYER_SPEC_START:      return "LAYER_SPEC_START";
        case LILITH_TOKEN_LAYER_CONV_START:      return "LAYER_CONV_START";
        case LILITH_TOKEN_LAYER_POOL_START:      return "LAYER_POOL_START";
        case LILITH_TOKEN_LAYER_NORM_START:      return "LAYER_NORM_START";
        case LILITH_TOKEN_MEMORY_START:          return "MEMORY_START";
        case LILITH_TOKEN_MEMORY_END:            return "MEMORY_END";
        case LILITH_TOKEN_STREAM_START:          return "STREAM_START";
        case LILITH_TOKEN_STREAM_END:            return "STREAM_END";
        case LILITH_TOKEN_WINDOW_START:          return "WINDOW_START";
        case LILITH_TOKEN_SYM_EXPR_START:        return "SYM_EXPR_START";
        case LILITH_TOKEN_SYM_EXPR_END:          return "SYM_EXPR_END";
        case LILITH_TOKEN_COMPILE_EXPR_START:    return "COMPILE_EXPR_START";
        case LILITH_TOKEN_COMPILE_EXPR_END:      return "COMPILE_EXPR_END";
        case LILITH_TOKEN_NUMBER:                return "NUMBER";
        case LILITH_TOKEN_STRING:                return "STRING";
        case LILITH_TOKEN_IDENTIFIER:            return "IDENTIFIER";
        default:                                 return "UNKNOWN";
    }
}
