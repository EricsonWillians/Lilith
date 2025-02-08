/**
 * @file test_lexer.c
 * @brief Unit tests for the Lilith lexer.
 *
 * This file contains several test cases that verify the behavior of the Lilith lexer.
 * It tests reserved tokens, number literals, string literals, identifier recognition,
 * and comment skipping. The tests use standard C assertions; if an assertion fails,
 * the program will abort.
 */

#include "lexer.h"
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
 * Input: "/* This is a comment */abc"
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

/**
 * @brief Main entry point for running lexer tests.
 */
int main(void) {
    printf("Running Lexer Tests...\n");

    test_program_tokens();
    test_number_literal();
    test_string_literal();
    test_comment_skipping();
    test_reserved_token();

    printf("All Lexer tests passed successfully.\n");
    return 0;
}
