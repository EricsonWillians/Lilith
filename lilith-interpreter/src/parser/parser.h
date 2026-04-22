#ifndef LILITH_PARSER_H
#define LILITH_PARSER_H

#include "ast.h"
#include "lexer/lexer.h"

typedef struct {
    Lexer *lexer;
    Token current;
    Token previous;
    int had_error;
    int panic_mode;
} Parser;

AstNode *parser_parse(Lexer *lexer);

#endif
