/**
 * @file lexer.c
 * @brief Implementation of the Lilith lexer.
 *
 * This lexer converts a source code string (with punctuation-only tokens)
 * into a stream of tokens according to the fully enhanced Lilith grammar.
 *
 * See lexer.h for a description of the supported tokens. Comments in the
 * source code are delimited by "/*" and "*/".
 */

#include "lexer.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

/* ========================================================================= */
/* Internal Lexer Structure                                                 */
/* ========================================================================= */

struct Lexer {
    const char *source;   /* Source code (null-terminated) */
    char *filename;       /* Filename (allocated via strdup) */
    size_t pos;           /* Current index in the source */
    size_t length;        /* Length of the source string */
    size_t line;          /* Current line number (1-indexed) */
    size_t column;        /* Current column number (1-indexed) */
};

/* ========================================================================= */
/* Helper Functions                                                         */
/* ========================================================================= */

/* Return nonzero if we've reached the end of the source */
static int isAtEnd(Lexer *lexer) {
    return lexer->pos >= lexer->length;
}

/* Return the current character without advancing */
static char currentChar(Lexer *lexer) {
    return isAtEnd(lexer) ? '\0' : lexer->source[lexer->pos];
}

/* Peek at the next character without advancing */
static char peekNext(Lexer *lexer) {
    return (lexer->pos + 1 >= lexer->length) ? '\0' : lexer->source[lexer->pos + 1];
}

/* Advance one character and update line/column counters */
static char advance(Lexer *lexer) {
    char c = currentChar(lexer);
    lexer->pos++;
    if (c == '\n') {
        lexer->line++;
        lexer->column = 1;
    } else {
        lexer->column++;
    }
    return c;
}

/* Skip whitespace characters */
static void skipWhitespace(Lexer *lexer) {
    while (!isAtEnd(lexer)) {
        char c = currentChar(lexer);
        if (c == ' ' || c == '\r' || c == '\t' || c == '\n') {
            advance(lexer);
        } else {
            break;
        }
    }
}

/* Skip a comment delimited by "/*" and "*/" */
static void skipComment(Lexer *lexer) {
    if (currentChar(lexer) == '/' && peekNext(lexer) == '*') {
        advance(lexer); /* consume '/' */
        advance(lexer); /* consume '*' */
        while (!isAtEnd(lexer)) {
            if (currentChar(lexer) == '*' && peekNext(lexer) == '/') {
                advance(lexer); /* consume '*' */
                advance(lexer); /* consume '/' */
                break;
            }
            advance(lexer);
        }
    }
}

/* Skip whitespace and comments in a loop */
static void skipWhitespaceAndComments(Lexer *lexer) {
    for (;;) {
        skipWhitespace(lexer);
        if (currentChar(lexer) == '/' && peekNext(lexer) == '*') {
            skipComment(lexer);
        } else {
            break;
        }
    }
}

/* Return 1 if the next characters match the given string; does not advance */
static int matchString(Lexer *lexer, const char *str) {
    size_t len = strlen(str);
    if (lexer->pos + len > lexer->length)
        return 0;
    return (strncmp(lexer->source + lexer->pos, str, len) == 0);
}

/* ========================================================================= */
/* Token Creation Helper                                                    */
/* ========================================================================= */

/* Create a token from the current lexing state */
static Token makeToken(Lexer *lexer, LilithTokenType type, size_t startPos,
                       size_t startLine, size_t startColumn) {
    Token token;
    token.type = type;
    token.lexeme = lexer->source + startPos;
    token.length = lexer->pos - startPos;
    token.line = startLine;
    token.column = startColumn;
    return token;
}

/* ========================================================================= */
/* Reserved Token Mapping                                                   */
/* ========================================================================= */

/* Structure to map reserved token strings to token types */
typedef struct {
    const char *str;
    LilithTokenType type;
} TokenMapping;

/* The mapping array is ordered so that longer tokens (or those with common prefixes)
   appear first. Note that some literal strings (e.g. "[<") appear only once;
   the parser is expected to disambiguate context when necessary. */
static const TokenMapping tokenMappings[] = {
    {"<%|", LILITH_TOKEN_MACRO_DEF_START},
    {"|%>", LILITH_TOKEN_MACRO_DEF_END},
    {"[:~", LILITH_TOKEN_FINALLY_START},
    {"~:]", LILITH_TOKEN_FINALLY_END},
    {"(-<", LILITH_TOKEN_MATCH_START},
    {">-)", LILITH_TOKEN_MATCH_END},
    {"[:<", LILITH_TOKEN_FOR_CLAUSE_START},
    {">:]", LILITH_TOKEN_FOR_CLAUSE_END},
    {"[?:", LILITH_TOKEN_IF_CLAUSE_START},
    {":?]", LILITH_TOKEN_IF_CLAUSE_END},
    {"[=]", LILITH_TOKEN_ASSIGN},
    {"(:<", LILITH_TOKEN_LAMBDA_DEF_START},
    {">:)", LILITH_TOKEN_LAMBDA_DEF_END},
    {")-?", LILITH_TOKEN_YIELD_START},
    {"?-(", LILITH_TOKEN_YIELD_END},
    {")-", LILITH_TOKEN_RETURN_START},
    {"-(", LILITH_TOKEN_RETURN_END},
    {"]-!", LILITH_TOKEN_BREAK},
    {"]-?", LILITH_TOKEN_CONTINUE},
    {"{?", LILITH_TOKEN_TRY_START},
    {"?}", LILITH_TOKEN_TRY_END},
    {"[!", LILITH_TOKEN_EXCEPT_START},
    {"!]", LILITH_TOKEN_EXCEPT_END},
    {"[/]", LILITH_TOKEN_EXCEPT_DIVIDER},
    {"->", LILITH_TOKEN_ARROW},
    {"(:)", LILITH_TOKEN_TYPE_ANNOT},
    {"~(", LILITH_TOKEN_AWAIT_START},
    {")~", LILITH_TOKEN_AWAIT_END},
    {"[(", LILITH_TOKEN_COND_THEN_GROUP_START},
    {")]", LILITH_TOKEN_COND_THEN_GROUP_END},
    {"++", LILITH_TOKEN_PLUSPLUS},
    {"--", LILITH_TOKEN_MINUSMINUS},
    {"**", LILITH_TOKEN_ASTERISK_ASTERISK},
    {"//", LILITH_TOKEN_SLASH_SLASH},
    {":-:", LILITH_TOKEN_UNARY},
    {",,", LILITH_TOKEN_COMMA_COMMA},
    {"{[", LILITH_TOKEN_PROGRAM_START},
    {"]}", LILITH_TOKEN_PROGRAM_END},
    {"[[", LILITH_TOKEN_BLOCK_START},
    {"]]", LILITH_TOKEN_BLOCK_END},
    {"][" , LILITH_TOKEN_STMT_SEP},
    {"[?", LILITH_TOKEN_IF_START},
    {"?]", LILITH_TOKEN_IF_END},
    {"<+", LILITH_TOKEN_WHILE_START},
    {"+>", LILITH_TOKEN_WHILE_END},
    {"(|", LILITH_TOKEN_FUNC_DEF_START},
    {"|)", LILITH_TOKEN_FUNC_DEF_END},
    {"{|", LILITH_TOKEN_CLASS_DEF_START},
    {"|}", LILITH_TOKEN_CLASS_DEF_END},
    {"((", LILITH_TOKEN_EXPR_GROUP_START},
    {"))", LILITH_TOKEN_EXPR_GROUP_END},
    {"[<", LILITH_TOKEN_LIST_LITERAL_START},  /* Also used for case start in pattern matching */
    {">]", LILITH_TOKEN_LIST_LITERAL_END},    /* Also used for case end */
    {"(<", LILITH_TOKEN_TUPLE_LITERAL_START},
    {">)", LILITH_TOKEN_TUPLE_LITERAL_END},
    {"{<", LILITH_TOKEN_DICT_LITERAL_START},
    {">}", LILITH_TOKEN_DICT_LITERAL_END},
    {"[{", LILITH_TOKEN_SET_LITERAL_START},
    {"}]", LILITH_TOKEN_SET_LITERAL_END}
};

static const size_t tokenMappingsCount = sizeof(tokenMappings) / sizeof(tokenMappings[0]);

/* ========================================================================= */
/* Identifier and Literal Helpers                                           */
/* ========================================================================= */

static int isDigit(char c) {
    return c >= '0' && c <= '9';
}

/* For Lilith, an identifier is a sequence of allowed punctuation symbols.
   Adjust the allowed set as needed. */
static int isAllowedSym(char c) {
    const char *allowed = "!@#$%^&*_=+-/?:";  
    return (strchr(allowed, c) != NULL);
}

static Token readNumber(Lexer *lexer, size_t startPos, size_t startLine, size_t startColumn) {
    while (isDigit(currentChar(lexer))) {
        advance(lexer);
    }
    return makeToken(lexer, LILITH_TOKEN_NUMBER, startPos, startLine, startColumn);
}

static Token readString(Lexer *lexer, size_t startPos, size_t startLine, size_t startColumn) {
    advance(lexer); // Consume opening quote
    while (!isAtEnd(lexer) && currentChar(lexer) != '"') {
        advance(lexer);
    }
    if (isAtEnd(lexer)) {
        return makeToken(lexer, LILITH_TOKEN_ERROR, startPos, startLine, startColumn);
    }
    advance(lexer); // Consume closing quote
    return makeToken(lexer, LILITH_TOKEN_STRING, startPos, startLine, startColumn);
}

static Token readIdentifier(Lexer *lexer, size_t startPos, size_t startLine, size_t startColumn) {
    while (!isAtEnd(lexer) && isAllowedSym(currentChar(lexer))) {
        /* Check if the upcoming characters match any reserved token */
        int reserved = 0;
        for (size_t i = 0; i < tokenMappingsCount; i++) {
            if (matchString(lexer, tokenMappings[i].str)) {
                reserved = 1;
                break;
            }
        }
        if (reserved)
            break;
        advance(lexer);
    }
    return makeToken(lexer, LILITH_TOKEN_IDENTIFIER, startPos, startLine, startColumn);
}

/* ========================================================================= */
/* Reserved Token Matching                                                  */
/* ========================================================================= */

static int tryMatchReserved(Lexer *lexer, Token *tokenOut) {
    for (size_t i = 0; i < tokenMappingsCount; i++) {
        const char *mappingStr = tokenMappings[i].str;
        size_t len = strlen(mappingStr);
        if (lexer->pos + len <= lexer->length &&
            strncmp(lexer->source + lexer->pos, mappingStr, len) == 0) {
            size_t startPos = lexer->pos;
            size_t startLine = lexer->line;
            size_t startColumn = lexer->column;
            for (size_t j = 0; j < len; j++) {
                advance(lexer);
            }
            *tokenOut = makeToken(lexer, tokenMappings[i].type, startPos, startLine, startColumn);
            return 1;
        }
    }
    return 0;
}

/* ========================================================================= */
/* Public API Functions                                                     */
/* ========================================================================= */

Lexer *lexer_create(const char *source, const char *filename) {
    if (!source) return NULL;
    Lexer *lexer = (Lexer *)malloc(sizeof(Lexer));
    if (!lexer) return NULL;
    lexer->source = source;
    lexer->length = strlen(source);
    lexer->pos = 0;
    lexer->line = 1;
    lexer->column = 1;
    lexer->filename = filename ? strdup(filename) : NULL;
    return lexer;
}

void lexer_destroy(Lexer *lexer) {
    if (!lexer) return;
    if (lexer->filename)
        free(lexer->filename);
    free(lexer);
}

Token lexer_next_token(Lexer *lexer) {
    skipWhitespaceAndComments(lexer);
    size_t startPos = lexer->pos;
    size_t startLine = lexer->line;
    size_t startColumn = lexer->column;
    
    if (isAtEnd(lexer)) {
        return makeToken(lexer, LILITH_TOKEN_EOF, lexer->pos, lexer->line, lexer->column);
    }
    
    Token token;
    if (tryMatchReserved(lexer, &token)) {
        return token;
    }
    
    char c = currentChar(lexer);
    if (isDigit(c)) {
        return readNumber(lexer, startPos, startLine, startColumn);
    }
    if (c == '"') {
        return readString(lexer, startPos, startLine, startColumn);
    }
    if (isAllowedSym(c)) {
        return readIdentifier(lexer, startPos, startLine, startColumn);
    }
    
    /* Unrecognized character: consume it and return an error token */
    advance(lexer);
    return makeToken(lexer, LILITH_TOKEN_ERROR, startPos, startLine, startColumn);
}

Token lexer_peek_token(Lexer *lexer) {
    Lexer backup = *lexer;
    Token token = lexer_next_token(lexer);
    *lexer = backup;
    return token;
}

void lexer_reset(Lexer *lexer) {
    if (!lexer) return;
    lexer->pos = 0;
    lexer->line = 1;
    lexer->column = 1;
}

/* ========================================================================= */
/* Token Type to String Conversion                                          */
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
        case LILITH_TOKEN_MACRO_DEF_START:   return "MACRO_DEF_START";
        case LILITH_TOKEN_MACRO_DEF_END:     return "MACRO_DEF_END";
        case LILITH_TOKEN_FOR_CLAUSE_START:  return "FOR_CLAUSE_START";
        case LILITH_TOKEN_FOR_CLAUSE_END:    return "FOR_CLAUSE_END";
        case LILITH_TOKEN_IF_CLAUSE_START:   return "IF_CLAUSE_START";
        case LILITH_TOKEN_IF_CLAUSE_END:     return "IF_CLAUSE_END";
        case LILITH_TOKEN_ARROW:             return "ARROW";
        case LILITH_TOKEN_TYPE_ANNOT:        return "TYPE_ANNOT";
        case LILITH_TOKEN_AWAIT_START:       return "AWAIT_START";
        case LILITH_TOKEN_AWAIT_END:         return "AWAIT_END";
        case LILITH_TOKEN_COND_THEN_GROUP_START: return "COND_THEN_GROUP_START";
        case LILITH_TOKEN_COND_THEN_GROUP_END:   return "COND_THEN_GROUP_END";
        case LILITH_TOKEN_PLUSPLUS:          return "PLUSPLUS";
        case LILITH_TOKEN_MINUSMINUS:        return "MINUSMINUS";
        case LILITH_TOKEN_ASTERISK_ASTERISK: return "ASTERISK_ASTERISK";
        case LILITH_TOKEN_SLASH_SLASH:       return "SLASH_SLASH";
        case LILITH_TOKEN_UNARY:             return "UNARY";
        case LILITH_TOKEN_COMMA_COMMA:       return "COMMA_COMMA";
        case LILITH_TOKEN_NUMBER:            return "NUMBER";
        case LILITH_TOKEN_STRING:            return "STRING";
        case LILITH_TOKEN_IDENTIFIER:        return "IDENTIFIER";
        default:                             return "UNKNOWN";
    }
}
