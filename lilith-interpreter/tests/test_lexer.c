/**
 * @file test_lexer.c
 * @brief Unit tests for the Lilith lexer.
 *
 * This file contains several test cases that verify the behavior of the Lilith lexer.
 * It tests reserved tokens, number literals, string literals, identifier recognition,
 * and comment skipping. The tests use standard C assertions; if an assertion fails,
 * the program will abort.
 */

#include "lexer/lexer.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/**
 * @brief Tests that the lexer correctly identifies program delimiters and an identifier.
 *
 * Input: "{[abc]}"
 * Expected tokens:
 *   1. PROGRAM_START token ("{[")
 *   2. IDENTIFIER token ("abc")
 *   3. PROGRAM_END token ("]}")
 *   4. EOF token
 */
static void test_program_tokens(void) {
    const char *source = "{[abc]}";
    Lexer *lexer = lexer_create(source, "test_program_tokens.lilith");
    assert(lexer != NULL);

    Token token = lexer_next_token(lexer);
    assert(token.type == LILITH_TOKEN_PROGRAM_START);

    token = lexer_next_token(lexer);
    assert(token.type == LILITH_TOKEN_IDENTIFIER);
    assert(token.length == 3);
    assert(strncmp(token.lexeme, "abc", token.length) == 0);

    token = lexer_next_token(lexer);
    assert(token.type == LILITH_TOKEN_PROGRAM_END);

    token = lexer_next_token(lexer);
    assert(token.type == LILITH_TOKEN_EOF);

    lexer_destroy(lexer);
    printf("test_program_tokens passed.\n");
}

/**
 * @brief Tests that the lexer correctly reads a number literal.
 *
 * Input: "12345"
 * Expected: A NUMBER token with lexeme "12345".
 */
static void test_number_literal(void) {
    const char *source = "12345";
    Lexer *lexer = lexer_create(source, "test_number_literal.lilith");
    assert(lexer != NULL);

    Token token = lexer_next_token(lexer);
    assert(token.type == LILITH_TOKEN_NUMBER);
    assert(token.length == 5);
    assert(strncmp(token.lexeme, "12345", token.length) == 0);

    token = lexer_next_token(lexer);
    assert(token.type == LILITH_TOKEN_EOF);

    lexer_destroy(lexer);
    printf("test_number_literal passed.\n");
}

/**
 * @brief Tests that the lexer correctly reads a float literal.
 */
static void test_float_literal(void) {
    const char *source = "3.14";
    Lexer *lexer = lexer_create(source, "test_float_literal.lilith");
    assert(lexer != NULL);

    Token token = lexer_next_token(lexer);
    assert(token.type == LILITH_TOKEN_NUMBER);
    assert(token.length == 4);
    assert(strncmp(token.lexeme, "3.14", token.length) == 0);

    token = lexer_next_token(lexer);
    assert(token.type == LILITH_TOKEN_EOF);

    lexer_destroy(lexer);
    printf("test_float_literal passed.\n");
}

/**
 * @brief Tests that the lexer correctly reads a negative literal.
 */
static void test_negative_literal(void) {
    const char *source = "-42";
    Lexer *lexer = lexer_create(source, "test_negative_literal.lilith");
    assert(lexer != NULL);

    Token token = lexer_next_token(lexer);
    assert(token.type == LILITH_TOKEN_NUMBER);
    assert(token.length == 3);
    assert(strncmp(token.lexeme, "-42", token.length) == 0);

    token = lexer_next_token(lexer);
    assert(token.type == LILITH_TOKEN_EOF);

    lexer_destroy(lexer);
    printf("test_negative_literal passed.\n");
}

/**
 * @brief Tests that the lexer correctly reads a negative float literal.
 */
static void test_negative_float_literal(void) {
    const char *source = "-3.14";
    Lexer *lexer = lexer_create(source, "test_negative_float_literal.lilith");
    assert(lexer != NULL);

    Token token = lexer_next_token(lexer);
    assert(token.type == LILITH_TOKEN_NUMBER);
    assert(token.length == 5);
    assert(strncmp(token.lexeme, "-3.14", token.length) == 0);

    token = lexer_next_token(lexer);
    assert(token.type == LILITH_TOKEN_EOF);

    lexer_destroy(lexer);
    printf("test_negative_float_literal passed.\n");
}

/**
 * @brief Tests that the lexer correctly reads a string literal.
 *
 * Input: "\"hello\""
 * Expected: A STRING token with lexeme "\"hello\"" (including the quotes).
 */
static void test_string_literal(void) {
    const char *source = "\"hello\"";
    Lexer *lexer = lexer_create(source, "test_string_literal.lilith");
    assert(lexer != NULL);

    Token token = lexer_next_token(lexer);
    assert(token.type == LILITH_TOKEN_STRING);
    // Our readString function consumes both the opening and closing quotes.
    assert(token.length == 7);
    assert(strncmp(token.lexeme, "\"hello\"", token.length) == 0);

    token = lexer_next_token(lexer);
    assert(token.type == LILITH_TOKEN_EOF);

    lexer_destroy(lexer);
    printf("test_string_literal passed.\n");
}

/**
 * @brief Tests that the lexer properly skips comments.
 *
 * Input: "/ * This is a comment * /abc" (without spaces)
 * Expected: The comment is skipped and an IDENTIFIER token "abc" is produced.
 */
static void test_comment_skipping(void) {
    const char *source = "/* This is a comment */abc";
    Lexer *lexer = lexer_create(source, "test_comment_skipping.lilith");
    assert(lexer != NULL);

    Token token = lexer_next_token(lexer);
    assert(token.type == LILITH_TOKEN_IDENTIFIER);
    assert(token.length == 3);
    assert(strncmp(token.lexeme, "abc", token.length) == 0);

    token = lexer_next_token(lexer);
    assert(token.type == LILITH_TOKEN_EOF);

    lexer_destroy(lexer);
    printf("test_comment_skipping passed.\n");
}

/**
 * @brief Tests that a reserved token is recognized.
 *
 * Input: "[=]"
 * Expected: An ASSIGN token.
 */
static void test_reserved_token(void) {
    const char *source = "[=]";
    Lexer *lexer = lexer_create(source, "test_reserved_token.lilith");
    assert(lexer != NULL);

    Token token = lexer_next_token(lexer);
    assert(token.type == LILITH_TOKEN_ASSIGN);

    token = lexer_next_token(lexer);
    assert(token.type == LILITH_TOKEN_EOF);

    lexer_destroy(lexer);
    printf("test_reserved_token passed.\n");
}

/* ========================================================================= */
/* Forward declarations for comprehensive tests                              */
/* ========================================================================= */

static void test_hello_world(void);
static void test_function_definition(void);
static void test_class_with_inheritance(void);
static void test_if_statement(void);
static void test_while_loop(void);
static void test_list_and_dict_literals(void);
static void test_match_statement(void);
static void test_longest_match(void);
static void test_alphabetic_identifiers(void);
static void test_gpu_tensor_tokens(void);
static void test_macro_tokens(void);
static void test_stream_tokens(void);

/**
 * @brief Main entry point for running lexer tests.
 */
int main(void) {
    printf("Running Lexer Tests...\n");

    test_program_tokens();
    test_number_literal();
    test_float_literal();
    test_negative_literal();
    test_negative_float_literal();
    test_string_literal();
    test_comment_skipping();
    test_reserved_token();

    test_hello_world();
    test_function_definition();
    test_class_with_inheritance();
    test_if_statement();
    test_while_loop();
    test_list_and_dict_literals();
    test_match_statement();
    test_longest_match();
    test_alphabetic_identifiers();
    test_gpu_tensor_tokens();
    test_macro_tokens();
    test_stream_tokens();

    printf("All Lexer tests passed successfully.\n");
    return 0;
}

/* ========================================================================= */
/* Comprehensive tests against real Lilith code from examples                */
/* ========================================================================= */

static void test_hello_world(void) {
    const char *source = "{[\n    @!((\"Hello, World!\"))\n]}";
    Lexer *l = lexer_create(source, "test_hello_world.lilith");
    Token t = lexer_next_token(l);
    assert(t.type == LILITH_TOKEN_PROGRAM_START);
    t = lexer_next_token(l);
    assert(t.type == LILITH_TOKEN_IDENTIFIER);
    assert(strncmp(t.lexeme, "@!", t.length) == 0);
    t = lexer_next_token(l);
    assert(t.type == LILITH_TOKEN_EXPR_GROUP_START);
    t = lexer_next_token(l);
    assert(t.type == LILITH_TOKEN_STRING);
    t = lexer_next_token(l);
    assert(t.type == LILITH_TOKEN_EXPR_GROUP_END);
    t = lexer_next_token(l);
    assert(t.type == LILITH_TOKEN_PROGRAM_END);
    t = lexer_next_token(l);
    assert(t.type == LILITH_TOKEN_EOF);
    lexer_destroy(l);
    printf("test_hello_world passed.\n");
}

static void test_function_definition(void) {
    const char *source = "(| add ((a,, b))\n    [[\n        )- a ++ b -(\n    ]]\n|)";
    Lexer *l = lexer_create(source, "test_function_definition.lilith");
    Token t;
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_FUNC_DEF_START);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_IDENTIFIER);
    assert(strncmp(t.lexeme, "add", t.length) == 0);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_EXPR_GROUP_START);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_IDENTIFIER); /* a */
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_COMMA_COMMA);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_IDENTIFIER); /* b */
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_EXPR_GROUP_END);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_BLOCK_START);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_RETURN_START);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_IDENTIFIER); /* a */
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_PLUSPLUS);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_IDENTIFIER); /* b */
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_RETURN_END);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_BLOCK_END);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_FUNC_DEF_END);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_EOF);
    lexer_destroy(l);
    printf("test_function_definition passed.\n");
}

static void test_class_with_inheritance(void) {
    const char *source = "{| Dog ([:Animal:]) [[ ]] |}";
    Lexer *l = lexer_create(source, "test_class_with_inheritance.lilith");
    Token t;
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_CLASS_DEF_START);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_IDENTIFIER); /* Dog */
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_INHERITANCE_START);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_IDENTIFIER); /* Animal */
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_INHERITANCE_END);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_BLOCK_START);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_BLOCK_END);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_CLASS_DEF_END);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_EOF);
    lexer_destroy(l);
    printf("test_class_with_inheritance passed.\n");
}

static void test_if_statement(void) {
    const char *source = "[?((x ++ y == 10)) [[ ]] :|: [[ ]] ?]";
    Lexer *l = lexer_create(source, "test_if_statement.lilith");
    Token t;
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_IF_START);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_EXPR_GROUP_START);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_IDENTIFIER); /* x */
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_PLUSPLUS);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_IDENTIFIER); /* y */
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_EQEQ);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_NUMBER); /* 10 */
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_EXPR_GROUP_END);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_BLOCK_START);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_BLOCK_END);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_ELSE_INTRO);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_BLOCK_START);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_BLOCK_END);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_IF_END);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_EOF);
    lexer_destroy(l);
    printf("test_if_statement passed.\n");
}

static void test_while_loop(void) {
    const char *source = "<+((x << 10))[[x [=] x ++ 1]]+>";
    Lexer *l = lexer_create(source, "test_while_loop.lilith");
    Token t;
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_WHILE_START);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_EXPR_GROUP_START);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_IDENTIFIER); /* x */
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_LESS_LESS);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_NUMBER); /* 10 */
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_EXPR_GROUP_END);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_BLOCK_START);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_IDENTIFIER); /* x */
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_ASSIGN);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_IDENTIFIER); /* x */
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_PLUSPLUS);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_NUMBER); /* 1 */
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_BLOCK_END);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_WHILE_END);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_EOF);
    lexer_destroy(l);
    printf("test_while_loop passed.\n");
}

static void test_list_and_dict_literals(void) {
    const char *source = "[<1,, 2,, 3>] {<\"name\" [:] \"Alice\">}";
    Lexer *l = lexer_create(source, "test_list_and_dict_literals.lilith");
    Token t;
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_LIST_LITERAL_START);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_NUMBER); /* 1 */
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_COMMA_COMMA);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_NUMBER); /* 2 */
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_COMMA_COMMA);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_NUMBER); /* 3 */
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_LIST_LITERAL_END);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_DICT_LITERAL_START);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_STRING); /* "name" */
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_DICT_MAP);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_STRING); /* "Alice" */
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_DICT_LITERAL_END);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_EOF);
    lexer_destroy(l);
    printf("test_list_and_dict_literals passed.\n");
}

static void test_match_statement(void) {
    const char *source = "(-< value >-)[<0>][[ ]][<1>][[ ]]";
    Lexer *l = lexer_create(source, "test_match_statement.lilith");
    Token t;
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_MATCH_START);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_IDENTIFIER); /* value */
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_MATCH_END);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_LIST_LITERAL_START); /* [< */
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_NUMBER); /* 0 */
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_LIST_LITERAL_END); /* >] */
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_BLOCK_START);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_BLOCK_END);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_LIST_LITERAL_START); /* [< */
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_NUMBER); /* 1 */
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_LIST_LITERAL_END); /* >] */
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_BLOCK_START);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_BLOCK_END);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_EOF);
    lexer_destroy(l);
    printf("test_match_statement passed.\n");
}

static void test_longest_match(void) {
    /* Verify that `++` is parsed as PLUSPLUS, not PLUS + PLUS */
    const char *source = "++";
    Lexer *l = lexer_create(source, "test_longest_match.lilith");
    Token t = lexer_next_token(l);
    assert(t.type == LILITH_TOKEN_PLUSPLUS);
    assert(t.length == 2);
    t = lexer_next_token(l);
    assert(t.type == LILITH_TOKEN_EOF);
    lexer_destroy(l);

    /* Verify that `==` is parsed as EQEQ, not EQUALS + EQUALS */
    l = lexer_create("==", "test_longest_match2.lilith");
    t = lexer_next_token(l);
    assert(t.type == LILITH_TOKEN_EQEQ);
    assert(t.length == 2);
    t = lexer_next_token(l);
    assert(t.type == LILITH_TOKEN_EOF);
    lexer_destroy(l);

    /* Verify that `[=]` is parsed as ASSIGN, not LBRACKET + EQUALS + RBRACKET */
    l = lexer_create("[=]", "test_longest_match3.lilith");
    t = lexer_next_token(l);
    assert(t.type == LILITH_TOKEN_ASSIGN);
    assert(t.length == 3);
    t = lexer_next_token(l);
    assert(t.type == LILITH_TOKEN_EOF);
    lexer_destroy(l);

    /* Verify that `!=` is BANG_EQ, not BANG + EQUALS */
    l = lexer_create("!=", "test_longest_match4.lilith");
    t = lexer_next_token(l);
    assert(t.type == LILITH_TOKEN_BANG_EQ);
    assert(t.length == 2);
    t = lexer_next_token(l);
    assert(t.type == LILITH_TOKEN_EOF);
    lexer_destroy(l);

    /* Verify that `!GPU` is DEVICE_GPU, not BANG + identifier */
    l = lexer_create("!GPU", "test_longest_match5.lilith");
    t = lexer_next_token(l);
    assert(t.type == LILITH_TOKEN_DEVICE_GPU);
    assert(t.length == 4);
    t = lexer_next_token(l);
    assert(t.type == LILITH_TOKEN_EOF);
    lexer_destroy(l);

    printf("test_longest_match passed.\n");
}

static void test_alphabetic_identifiers(void) {
    const char *source = "add calculate Person blockIdx x1 _tmp";
    Lexer *l = lexer_create(source, "test_alphabetic_identifiers.lilith");
    Token t;
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_IDENTIFIER);
    assert(strncmp(t.lexeme, "add", t.length) == 0);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_IDENTIFIER);
    assert(strncmp(t.lexeme, "calculate", t.length) == 0);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_IDENTIFIER);
    assert(strncmp(t.lexeme, "Person", t.length) == 0);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_IDENTIFIER);
    assert(strncmp(t.lexeme, "blockIdx", t.length) == 0);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_IDENTIFIER);
    assert(strncmp(t.lexeme, "x1", t.length) == 0);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_IDENTIFIER);
    assert(strncmp(t.lexeme, "_tmp", t.length) == 0);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_EOF);
    lexer_destroy(l);
    printf("test_alphabetic_identifiers passed.\n");
}

static void test_gpu_tensor_tokens(void) {
    const char *source = "<%% [[ ]] %%> [# [[ ]] #] <% [[ ]] %> [^ [[ ]] ^]";
    Lexer *l = lexer_create(source, "test_gpu_tensor_tokens.lilith");
    Token t;
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_SYM_EXPR_START);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_BLOCK_START);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_BLOCK_END);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_SYM_EXPR_END);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_TENSOR_START);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_BLOCK_START);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_BLOCK_END);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_TENSOR_END);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_GPU_START);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_BLOCK_START);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_BLOCK_END);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_GPU_END);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_MEMORY_START);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_BLOCK_START);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_BLOCK_END);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_MEMORY_END);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_EOF);
    lexer_destroy(l);
    printf("test_gpu_tensor_tokens passed.\n");
}

static void test_macro_tokens(void) {
    const char *source = "<%| LOG |%> `[ ]` ,[ ], {# #} (@ @) {@ @} =>> [# #] <@ @>";
    Lexer *l = lexer_create(source, "test_macro_tokens.lilith");
    Token t;
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_MACRO_DEF_START);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_IDENTIFIER); /* LOG */
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_MACRO_DEF_END);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_QUOTE_START);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_QUOTE_END);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_UNQUOTE_START);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_UNQUOTE_END);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_PATTERN_MATCH_START);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_PATTERN_MATCH_END);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_NODE_MATCH_START);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_NODE_MATCH_END);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_AST_TRANSFORM_START);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_AST_TRANSFORM_END);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_TRANSFORM_ARROW);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_TENSOR_START);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_TENSOR_END);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_COMPILE_EXPR_START);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_COMPILE_EXPR_END);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_EOF);
    lexer_destroy(l);
    printf("test_macro_tokens passed.\n");
}

static void test_stream_tokens(void) {
    const char *source = "<~ [[ ]] ~> <| [[ ]] |> <+(( ))[[ ]]+>";
    Lexer *l = lexer_create(source, "test_stream_tokens.lilith");
    Token t;
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_STREAM_START);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_BLOCK_START);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_BLOCK_END);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_STREAM_END);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_PARALLEL_START);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_BLOCK_START);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_BLOCK_END);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_PARALLEL_END);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_WHILE_START);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_EXPR_GROUP_START);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_EXPR_GROUP_END);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_BLOCK_START);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_BLOCK_END);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_WHILE_END);
    t = lexer_next_token(l); assert(t.type == LILITH_TOKEN_EOF);
    lexer_destroy(l);
    printf("test_stream_tokens passed.\n");
}
