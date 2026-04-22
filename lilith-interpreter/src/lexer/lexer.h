#ifndef LILITH_LEXER_H
#define LILITH_LEXER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

typedef enum {
    LILITH_TOKEN_EOF,
    LILITH_TOKEN_ERROR,

    /* ------------------------------------------------------------
       Program Structure
       ------------------------------------------------------------ */
    LILITH_TOKEN_PROGRAM_START,     /* "{[" */
    LILITH_TOKEN_PROGRAM_END,       /* "]}" */

    LILITH_TOKEN_BLOCK_START,       /* "[[" */
    LILITH_TOKEN_BLOCK_END,         /* "]]" */

    LILITH_TOKEN_STMT_SEP,          /* "][" */

    /* ------------------------------------------------------------
       Assignment
       ------------------------------------------------------------ */
    LILITH_TOKEN_ASSIGN,            /* "[=]" */

    /* ------------------------------------------------------------
       Expression Grouping
       ------------------------------------------------------------ */
    LILITH_TOKEN_EXPR_GROUP_START,  /* "((" */
    LILITH_TOKEN_EXPR_GROUP_END,    /* "))" */

    /* ------------------------------------------------------------
       Control Flow
       ------------------------------------------------------------ */
    LILITH_TOKEN_IF_START,          /* "[?" */
    LILITH_TOKEN_IF_END,            /* "?]" */
    LILITH_TOKEN_ELSE_INTRO,        /* ":|:" */

    LILITH_TOKEN_WHILE_START,       /* "<+" */
    LILITH_TOKEN_WHILE_END,         /* "+>" */

    LILITH_TOKEN_RETURN_START,      /* ")-" */
    LILITH_TOKEN_RETURN_END,        /* "-(" */
    LILITH_TOKEN_YIELD_START,       /* ")-?" */
    LILITH_TOKEN_YIELD_END,         /* "?-(" */

    LILITH_TOKEN_BREAK,             /* "]-!" */
    LILITH_TOKEN_CONTINUE,          /* "]-?" */

    /* ------------------------------------------------------------
       Exception Handling
       ------------------------------------------------------------ */
    LILITH_TOKEN_TRY_START,         /* "{?" */
    LILITH_TOKEN_TRY_END,           /* "?}" */
    LILITH_TOKEN_EXCEPT_START,      /* "[!" */
    LILITH_TOKEN_EXCEPT_END,        /* "!]" */
    LILITH_TOKEN_EXCEPT_DIVIDER,    /* "[/]" */
    LILITH_TOKEN_FINALLY_START,     /* "[:~" */
    LILITH_TOKEN_FINALLY_END,       /* "~:]" */

    /* ------------------------------------------------------------
       Pattern Matching
       ------------------------------------------------------------ */
    LILITH_TOKEN_MATCH_START,       /* "(-<" */
    LILITH_TOKEN_MATCH_END,         /* ">-)" */

    /* ------------------------------------------------------------
       Function / Class
       ------------------------------------------------------------ */
    LILITH_TOKEN_FUNC_DEF_START,    /* "(|" */
    LILITH_TOKEN_FUNC_DEF_END,      /* "|)" */
    LILITH_TOKEN_CLASS_DEF_START,   /* "{|" */
    LILITH_TOKEN_CLASS_DEF_END,     /* "|}" */
    LILITH_TOKEN_ARROW,             /* "->" */
    LILITH_TOKEN_TYPE_ANNOT,        /* "(:)" */
    LILITH_TOKEN_INHERITANCE_START, /* "([:" */
    LILITH_TOKEN_INHERITANCE_END,   /* ":])" */

    /* ------------------------------------------------------------
       Collections
       ------------------------------------------------------------ */
    LILITH_TOKEN_LIST_LITERAL_START,  /* "[<" */
    LILITH_TOKEN_LIST_LITERAL_END,    /* ">]" */
    LILITH_TOKEN_TUPLE_LITERAL_START, /* "(<" */
    LILITH_TOKEN_TUPLE_LITERAL_END,   /* ">)" */
    LILITH_TOKEN_DICT_LITERAL_START,  /* "{<" */
    LILITH_TOKEN_DICT_LITERAL_END,    /* ">}" */
    LILITH_TOKEN_SET_LITERAL_START,   /* "[{" */
    LILITH_TOKEN_SET_LITERAL_END,     /* "}]" */
    LILITH_TOKEN_DICT_MAP,            /* "[:]" */

    /* ------------------------------------------------------------
       Comprehensions
       ------------------------------------------------------------ */
    LILITH_TOKEN_FOR_CLAUSE_START,    /* "[:<" */
    LILITH_TOKEN_FOR_CLAUSE_END,      /* ">:]" */
    LILITH_TOKEN_IN_OPERATOR,         /* "[%]" */
    LILITH_TOKEN_IF_CLAUSE_START,     /* "[?:" */
    LILITH_TOKEN_IF_CLAUSE_END,       /* ":?]" */
    LILITH_TOKEN_WHERE_CLAUSE_START,  /* "[?|" */
    LILITH_TOKEN_WHERE_CLAUSE_END,    /* "|?]" */
    LILITH_TOKEN_GROUP_CLAUSE_START,  /* "[&|" */
    LILITH_TOKEN_GROUP_CLAUSE_END,    /* "|&]" */

    /* ------------------------------------------------------------
       Lambda
       ------------------------------------------------------------ */
    LILITH_TOKEN_LAMBDA_DEF_START,    /* "(:<" */
    LILITH_TOKEN_LAMBDA_DEF_END,      /* ">:)" */

    /* ------------------------------------------------------------
       Operators
       ------------------------------------------------------------ */
    LILITH_TOKEN_PLUSPLUS,            /* "++" */
    LILITH_TOKEN_MINUSMINUS,          /* "--" */
    LILITH_TOKEN_ASTERISK_ASTERISK,   /* "**" */
    LILITH_TOKEN_SLASH_SLASH,         /* "//" */
    LILITH_TOKEN_PERCENT_PERCENT,     /* "%%" */
    LILITH_TOKEN_EQEQ,                /* "==" */
    LILITH_TOKEN_BANG_EQ,             /* "!=" */
    LILITH_TOKEN_LESS_LESS,           /* "<<" */
    LILITH_TOKEN_GREATER_GREATER,     /* ">>" */
    LILITH_TOKEN_UNARY,               /* ":-:" */

    /* ------------------------------------------------------------
       Import
       ------------------------------------------------------------ */
    LILITH_TOKEN_IMPORT_START,        /* "<{" */
    LILITH_TOKEN_IMPORT_END,          /* "}>" */
    LILITH_TOKEN_COMMA_COMMA,         /* ",," */
    LILITH_TOKEN_DOT_DOT,             /* ".." */

    /* ------------------------------------------------------------
       Macro System
       ------------------------------------------------------------ */
    LILITH_TOKEN_MACRO_DEF_START,     /* "<%|" */
    LILITH_TOKEN_MACRO_DEF_END,       /* "|%"> */
    LILITH_TOKEN_QUOTE_START,         /* "`[" */
    LILITH_TOKEN_QUOTE_END,           /* "]`" */
    LILITH_TOKEN_UNQUOTE_START,       /* ",[" */
    LILITH_TOKEN_UNQUOTE_END,         /* "]," */
    LILITH_TOKEN_PATTERN_MATCH_START, /* "{#" */
    LILITH_TOKEN_PATTERN_MATCH_END,   /* "#}" */
    LILITH_TOKEN_NODE_MATCH_START,    /* "(@" */
    LILITH_TOKEN_NODE_MATCH_END,      /* "@)" */
    LILITH_TOKEN_AST_TRANSFORM_START, /* "{@" */
    LILITH_TOKEN_AST_TRANSFORM_END,   /* "@}" */
    LILITH_TOKEN_TRANSFORM_ARROW,     /* "=>>" */
    LILITH_TOKEN_SYM_EXPR_START,      /* "<%%" */
    LILITH_TOKEN_SYM_EXPR_END,        /* "%%>" */
    LILITH_TOKEN_COMPILE_EXPR_START,  /* "<@" */
    LILITH_TOKEN_COMPILE_EXPR_END,    /* "@>" */

    /* ------------------------------------------------------------
       Phase / Scope / Memory Specifiers
       ------------------------------------------------------------ */
    LILITH_TOKEN_PHASE_PARSE,         /* "!PARSE" */
    LILITH_TOKEN_PHASE_EXPAND,        /* "!EXPAND" */
    LILITH_TOKEN_PHASE_COMPILE,       /* "!COMPILE" */
    LILITH_TOKEN_SCOPE_CAPTURE,       /* "!CAPTURE" */
    LILITH_TOKEN_SCOPE_ISOLATE,       /* "!ISOLATE" */
    LILITH_TOKEN_SCOPE_INJECT,        /* "!INJECT" */
    LILITH_TOKEN_ALLOC_STATIC,        /* "!STATIC" */
    LILITH_TOKEN_ALLOC_DYNAMIC,       /* "!DYNAMIC" */
    LILITH_TOKEN_ALLOC_SHARED,        /* "!SHARED" */
    LILITH_TOKEN_ALLOC_PINNED,        /* "!PINNED" */

    /* ------------------------------------------------------------
       Parallel Processing
       ------------------------------------------------------------ */
    LILITH_TOKEN_PARALLEL_START,      /* "<|" */
    LILITH_TOKEN_PARALLEL_END,        /* "|>" */
    LILITH_TOKEN_SYNC_START,          /* "[S" */

    /* ------------------------------------------------------------
       GPU Computing
       ------------------------------------------------------------ */
    LILITH_TOKEN_GPU_START,           /* "<%" */
    LILITH_TOKEN_GPU_END,             /* "%>" */
    LILITH_TOKEN_KERNEL_START,        /* "[K" */
    LILITH_TOKEN_GRID_START,          /* "[G" */

    /* ------------------------------------------------------------
       Tensor Operations
       ------------------------------------------------------------ */
    LILITH_TOKEN_TENSOR_START,        /* "[#" */
    LILITH_TOKEN_TENSOR_END,          /* "#]" */
    LILITH_TOKEN_DEVICE_START,        /* "[D" */
    LILITH_TOKEN_DEVICE_GPU,          /* "!GPU" */
    LILITH_TOKEN_DEVICE_TPU,          /* "!TPU" */
    LILITH_TOKEN_DEVICE_CPU,          /* "!CPU" */
    LILITH_TOKEN_DEVICE_DSP,          /* "!DSP" */

    /* ------------------------------------------------------------
       Neural Network
       ------------------------------------------------------------ */
    LILITH_TOKEN_LAYER_SPEC_START,    /* "[L" — also used for layout spec */
    LILITH_TOKEN_LAYER_CONV_START,    /* "[C" */
    LILITH_TOKEN_LAYER_POOL_START,    /* "[P" */
    LILITH_TOKEN_LAYER_NORM_START,    /* "[N" */

    /* ------------------------------------------------------------
       Memory Management
       ------------------------------------------------------------ */
    LILITH_TOKEN_MEMORY_START,        /* "[^" */
    LILITH_TOKEN_MEMORY_END,          /* "^]" */

    /* ------------------------------------------------------------
       Stream Processing
       ------------------------------------------------------------ */
    LILITH_TOKEN_STREAM_START,        /* "<~" */
    LILITH_TOKEN_STREAM_END,          /* "~>" */
    LILITH_TOKEN_WINDOW_START,        /* "[W" */

    /* ------------------------------------------------------------
       Async
       ------------------------------------------------------------ */
    LILITH_TOKEN_COND_THEN_GROUP_START, /* "[(" */
    LILITH_TOKEN_COND_THEN_GROUP_END,   /* ")]" */
    LILITH_TOKEN_AWAIT_START,         /* "~(" */
    LILITH_TOKEN_AWAIT_END,           /* ")~" */
    LILITH_TOKEN_TILDE,               /* "~" */

    /* ------------------------------------------------------------
       Standalone punctuation tokens (used by parser)
       ------------------------------------------------------------ */
    LILITH_TOKEN_DOT,                 /* "." */
    LILITH_TOKEN_LBRACKET,            /* "[" */
    LILITH_TOKEN_RBRACKET,            /* "]" */

    /* ------------------------------------------------------------
       Literals
       ------------------------------------------------------------ */
    LILITH_TOKEN_NUMBER,
    LILITH_TOKEN_STRING,
    LILITH_TOKEN_IDENTIFIER

} LilithTokenType;

typedef struct {
    LilithTokenType type;
    const char *lexeme;
    size_t length;
    size_t line;
    size_t column;
} Token;

typedef struct Lexer Lexer;

Lexer *lexer_create(const char *source, const char *filename);
void lexer_destroy(Lexer *lexer);
Token lexer_next_token(Lexer *lexer);
Token lexer_peek_token(Lexer *lexer);
void lexer_reset(Lexer *lexer);
const char *lilith_token_type_to_string(LilithTokenType type);

#ifdef __cplusplus
}
#endif

#endif
