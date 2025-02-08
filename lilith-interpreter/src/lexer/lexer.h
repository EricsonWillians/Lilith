#ifndef LILITH_LEXER_H
#define LILITH_LEXER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/**
 * @file lexer.h
 * @brief Lexer interface for the Lilith programming language.
 *
 * Lilith is a punctuation-only language with symmetric paired tokens.
 * Its grammar includes:
 *   - Program delimiters: "{[" and "]}"
 *   - Blocks: "[[" and "]]"
 *   - Statement separator: "]["
 *   - Assignment operator: "[=]"
 *   - Expression grouping: "((" and "))"
 *   - IF–ELSE statements: "[?" ... "?]" with optional else introduced by ":|:"
 *   - WHILE–loops: "<+" and "+>"
 *   - Function definitions: "(|" ... "|)"
 *   - Class definitions: "{|" ... "|}"
 *   - Parameter lists enclosed in "((" and "))" with parameters separated by ",,"
 *   - Conditional expressions with inner grouping tokens "[(" and ")]"
 *   - Return and yield statements: ")-" ... "-(" and ")-?" ... "?-(" respectively
 *   - Loop control: break ("]-!") and continue ("]-?")
 *   - Exception handling: try ("{?" ... "?}"), except ("[!" ... "!]") with divider "[/]", finally ("[:~" ... "~:]")
 *   - Pattern matching: match ("(-<" ... ">-)") and case blocks ("[<" ... ">]")
 *   - Type annotations (arrow "->" and marker "(:)")
 *   - Macros: definition ("<%|" ... "|%>") and invocation (identifier + argument block)
 *   - Asynchronous constructs: await ("~(" ... ")~")
 *   - Comprehensions for collections:
 *       * List literals: "[<" ... ">]"
 *       * Tuple literals: "(<" ... ">)"
 *       * Dictionary literals: "{<" ... ">}" with key-value mapping "[:]"
 *       * Set literals: "[{" ... "}]"
 *       * Comprehension clauses: for clause "[:<" ... ">:]" (with in-operator "[%]") and if clause "[?:" ... ":?]"
 *   - Lambda expressions: defined with "(:<" ... ">:)"
 *   - Arithmetic operators: addition/subtraction ("++", "--"), multiplication/division ("**", "//")
 *   - A palindromic unary operator: ":-:"
 *   - A separator for parameters and list elements: ",,"
 *
 * In addition, comments are handled at the lexer level: a comment begins with "/*" and ends with "*/".
 */

/* ========================================================================= */
/* Token Types                                                             */
/* ========================================================================= */

/* The complete enumeration of token types supported by Lilith. */
typedef enum {
    LILITH_TOKEN_EOF,
    LILITH_TOKEN_ERROR,

    /* Program delimiters */
    LILITH_TOKEN_PROGRAM_START,     /* "{[" */
    LILITH_TOKEN_PROGRAM_END,       /* "]}" */

    /* Blocks */
    LILITH_TOKEN_BLOCK_START,       /* "[[" */
    LILITH_TOKEN_BLOCK_END,         /* "]]" */

    /* Statement separator */
    LILITH_TOKEN_STMT_SEP,          /* "][" */

    /* Assignment operator */
    LILITH_TOKEN_ASSIGN,            /* "[=]" */

    /* Expression grouping */
    LILITH_TOKEN_EXPR_GROUP_START,  /* "((" */
    LILITH_TOKEN_EXPR_GROUP_END,    /* "))" */

    /* Conditional statement */
    LILITH_TOKEN_IF_START,          /* "[?" */
    LILITH_TOKEN_IF_END,            /* "?]" */
    LILITH_TOKEN_ELSE_INTRO,        /* ":|:" */

    /* While loop */
    LILITH_TOKEN_WHILE_START,       /* "<+" */
    LILITH_TOKEN_WHILE_END,         /* "+>" */

    /* Function definition */
    LILITH_TOKEN_FUNC_DEF_START,    /* "(|" */
    LILITH_TOKEN_FUNC_DEF_END,      /* "|)" */

    /* Class definition */
    LILITH_TOKEN_CLASS_DEF_START,   /* "{|" */
    LILITH_TOKEN_CLASS_DEF_END,     /* "|}" */

    /* Parameter list grouping */
    LILITH_TOKEN_PARAM_LIST_START,  /* "((" in parameter lists */
    LILITH_TOKEN_PARAM_LIST_END,    /* "))" in parameter lists */

    /* Return and yield */
    LILITH_TOKEN_RETURN_START,      /* ")-" */
    LILITH_TOKEN_RETURN_END,        /* "-(" */
    LILITH_TOKEN_YIELD_START,       /* ")-?" */
    LILITH_TOKEN_YIELD_END,         /* "?-(" */

    /* Loop control */
    LILITH_TOKEN_BREAK,             /* "]-!" */
    LILITH_TOKEN_CONTINUE,          /* "]-?" */

    /* Exception handling */
    LILITH_TOKEN_TRY_START,         /* "{?" */
    LILITH_TOKEN_TRY_END,           /* "?}" */
    LILITH_TOKEN_EXCEPT_START,      /* "[!" */
    LILITH_TOKEN_EXCEPT_END,        /* "!]" */
    LILITH_TOKEN_EXCEPT_DIVIDER,    /* "[/]" */
    LILITH_TOKEN_FINALLY_START,     /* "[:~" */
    LILITH_TOKEN_FINALLY_END,       /* "~:]" */

    /* Pattern matching */
    LILITH_TOKEN_MATCH_START,       /* "(-<" */
    LILITH_TOKEN_MATCH_END,         /* ">-)" */
    LILITH_TOKEN_CASE_START,        /* "[<" */
    LILITH_TOKEN_CASE_END,          /* ">]" */

    /* Type annotation and return arrow */
    LILITH_TOKEN_ARROW,             /* "->" */
    LILITH_TOKEN_TYPE_ANNOT,        /* "(:)" */

    /* Macros */
    LILITH_TOKEN_MACRO_DEF_START,   /* "<%|" */
    LILITH_TOKEN_MACRO_DEF_END,     /* "|%>" */

    /* Asynchronous constructs */
    LILITH_TOKEN_AWAIT_START,       /* "~(" */
    LILITH_TOKEN_AWAIT_END,         /* ")~" */

    /* Conditional expression specific grouping */
    LILITH_TOKEN_COND_THEN_GROUP_START, /* "[(" used for then-branch */
    LILITH_TOKEN_COND_THEN_GROUP_END,   /* ")]" used for then-branch */

    /* Collection Literals */
    LILITH_TOKEN_LIST_LITERAL_START,  /* "[<" */
    LILITH_TOKEN_LIST_LITERAL_END,    /* ">]" */
    LILITH_TOKEN_TUPLE_LITERAL_START, /* "(<" */
    LILITH_TOKEN_TUPLE_LITERAL_END,   /* ">)" */
    LILITH_TOKEN_DICT_LITERAL_START,  /* "{<" */
    LILITH_TOKEN_DICT_LITERAL_END,    /* ">}" */
    LILITH_TOKEN_SET_LITERAL_START,   /* "[{" */
    LILITH_TOKEN_SET_LITERAL_END,     /* "}]" */
    LILITH_TOKEN_DICT_MAP,            /* "[:]" for dictionary key-value mapping */

    /* Comprehension Clauses */
    LILITH_TOKEN_FOR_CLAUSE_START,    /* "[:<" */
    LILITH_TOKEN_FOR_CLAUSE_END,      /* ">:]" */
    LILITH_TOKEN_IN_OPERATOR,         /* "[%]" */
    LILITH_TOKEN_IF_CLAUSE_START,     /* "[?:" */
    LILITH_TOKEN_IF_CLAUSE_END,       /* ":?]" */

    /* Lambda expressions */
    LILITH_TOKEN_LAMBDA_DEF_START,    /* "(:<" */
    LILITH_TOKEN_LAMBDA_DEF_END,      /* ">:)" */

    /* Arithmetic operators */
    LILITH_TOKEN_PLUSPLUS,            /* "++" */
    LILITH_TOKEN_MINUSMINUS,          /* "--" */
    LILITH_TOKEN_ASTERISK_ASTERISK,   /* "**" */
    LILITH_TOKEN_SLASH_SLASH,         /* "//" */

    /* Unary operator */
    LILITH_TOKEN_UNARY,               /* ":-:" */

    /* Separator for lists/parameters */
    LILITH_TOKEN_COMMA_COMMA,         /* ",," */

    /* Literals and identifiers */
    LILITH_TOKEN_NUMBER,
    LILITH_TOKEN_STRING,
    LILITH_TOKEN_IDENTIFIER

} LilithTokenType;

/**
 * @brief Represents a token produced by the lexer.
 */
typedef struct {
    LilithTokenType type;  /**< The type of the token */
    const char *lexeme;    /**< Pointer to the lexeme in the source */
    size_t length;         /**< Length of the lexeme */
    size_t line;           /**< Line number (1-indexed) where the token starts */
    size_t column;         /**< Column number (1-indexed) where the token starts */
} Token;

/**
 * @brief Opaque type representing a lexer instance.
 */
typedef struct Lexer Lexer;

/* ========================================================================= */
/* Lexer API                                                               */
/* ========================================================================= */

/**
 * @brief Creates a new lexer instance for the given source code.
 *
 * @param source Null-terminated string containing the source code.
 * @param filename Optional filename (for error reporting) or NULL.
 * @return Pointer to a new Lexer instance, or NULL on error.
 */
Lexer *lexer_create(const char *source, const char *filename);

/**
 * @brief Destroys a lexer instance, freeing its resources.
 *
 * @param lexer Pointer to the Lexer instance.
 */
void lexer_destroy(Lexer *lexer);

/**
 * @brief Retrieves the next token from the lexer.
 *
 * This function advances the lexer and returns the next token. When the end of the
 * source is reached, a token of type LILITH_TOKEN_EOF is returned.
 *
 * @param lexer Pointer to the Lexer instance.
 * @return The next token.
 */
Token lexer_next_token(Lexer *lexer);

/**
 * @brief Peeks at the next token without advancing the lexer's state.
 *
 * @param lexer Pointer to the Lexer instance.
 * @return The next token.
 */
Token lexer_peek_token(Lexer *lexer);

/**
 * @brief Resets the lexer to the beginning of the source.
 *
 * @param lexer Pointer to the Lexer instance.
 */
void lexer_reset(Lexer *lexer);

/**
 * @brief Converts a token type to its human-readable string representation.
 *
 * @param type The token type.
 * @return A constant string describing the token type.
 */
const char *lilith_token_type_to_string(LilithTokenType type);

#ifdef __cplusplus
}
#endif

#endif /* LILITH_LEXER_H */
