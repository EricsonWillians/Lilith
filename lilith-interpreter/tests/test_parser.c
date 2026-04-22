#include "parser/parser.h"
#include "lexer/lexer.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_parse_hello_world(void) {
    const char *source = "{[@!((\"Hello, World!\"))]}";
    Lexer *lexer = lexer_create(source, "test_parse_hello_world.lilith");
    AstNode *ast = parser_parse(lexer);
    assert(ast != NULL);
    assert(ast->type == AST_PROGRAM);
    ast_free(ast);
    lexer_destroy(lexer);
    printf("test_parse_hello_world passed.\n");
}

static void test_parse_assignment(void) {
    const char *source = "{[x [=] 42]}";
    Lexer *lexer = lexer_create(source, "test_parse_assignment.lilith");
    AstNode *ast = parser_parse(lexer);
    assert(ast != NULL);
    assert(ast->type == AST_PROGRAM);
    ast_free(ast);
    lexer_destroy(lexer);
    printf("test_parse_assignment passed.\n");
}

static void test_parse_if_statement(void) {
    const char *source = "{[ [?((x == 1)) [[x [=] 2]] ?] ]}";
    Lexer *lexer = lexer_create(source, "test_parse_if_statement.lilith");
    AstNode *ast = parser_parse(lexer);
    assert(ast != NULL);
    assert(ast->type == AST_PROGRAM);
    ast_free(ast);
    lexer_destroy(lexer);
    printf("test_parse_if_statement passed.\n");
}

static void test_parse_while_loop(void) {
    const char *source = "{[ <+((x << 10))[[x [=] x ++ 1]]+> ]}";
    Lexer *lexer = lexer_create(source, "test_parse_while_loop.lilith");
    AstNode *ast = parser_parse(lexer);
    assert(ast != NULL);
    assert(ast->type == AST_PROGRAM);
    ast_free(ast);
    lexer_destroy(lexer);
    printf("test_parse_while_loop passed.\n");
}

static void test_parse_function(void) {
    const char *source = "{[ (| add ((a,, b))[[)- a ++ b -(]]|) ]}";
    Lexer *lexer = lexer_create(source, "test_parse_function.lilith");
    AstNode *ast = parser_parse(lexer);
    assert(ast != NULL);
    assert(ast->type == AST_PROGRAM);
    ast_free(ast);
    lexer_destroy(lexer);
    printf("test_parse_function passed.\n");
}

static void test_parse_list_literal(void) {
    const char *source = "{[ x [=] [<1,, 2,, 3>] ]}";
    Lexer *lexer = lexer_create(source, "test_parse_list_literal.lilith");
    AstNode *ast = parser_parse(lexer);
    assert(ast != NULL);
    assert(ast->type == AST_PROGRAM);
    ast_free(ast);
    lexer_destroy(lexer);
    printf("test_parse_list_literal passed.\n");
}

static void test_parse_dict_literal(void) {
    const char *source = "{[ x [=] {<\"name\" [:] \"Alice\">} ]}";
    Lexer *lexer = lexer_create(source, "test_parse_dict_literal.lilith");
    AstNode *ast = parser_parse(lexer);
    assert(ast != NULL);
    assert(ast->type == AST_PROGRAM);
    ast_free(ast);
    lexer_destroy(lexer);
    printf("test_parse_dict_literal passed.\n");
}

static void test_parse_try_except(void) {
    const char *source = "{[ {?[[x [=] 1]][! err [/][[y [=] 2]]!]?} ]}";
    Lexer *lexer = lexer_create(source, "test_parse_try_except.lilith");
    AstNode *ast = parser_parse(lexer);
    assert(ast != NULL);
    assert(ast->type == AST_PROGRAM);
    ast_free(ast);
    lexer_destroy(lexer);
    printf("test_parse_try_except passed.\n");
}

static void test_parse_match(void) {
    const char *source = "{[ (-< x >-)[<0>][[ ]][<1>][[ ]] ]}";
    Lexer *lexer = lexer_create(source, "test_parse_match.lilith");
    AstNode *ast = parser_parse(lexer);
    assert(ast != NULL);
    assert(ast->type == AST_PROGRAM);
    ast_free(ast);
    lexer_destroy(lexer);
    printf("test_parse_match passed.\n");
}

static void test_parse_async_function(void);
static void test_parse_await(void);
static void test_parse_yield(void);
static void test_parse_parallel(void);
static void test_parse_gpu(void);
static void test_parse_tensor(void);
static void test_parse_stream(void);
static void test_parse_memory(void);

int main(void) {
    printf("Running Parser Tests...\n");
    test_parse_hello_world();
    test_parse_assignment();
    test_parse_if_statement();
    test_parse_while_loop();
    test_parse_function();
    test_parse_list_literal();
    test_parse_dict_literal();
    test_parse_try_except();
    test_parse_match();
    test_parse_async_function();
    test_parse_await();
    test_parse_yield();
    test_parse_parallel();
    test_parse_gpu();
    test_parse_tensor();
    test_parse_stream();
    test_parse_memory();
    printf("All Parser tests passed successfully.\n");
    return 0;
}

static void test_parse_async_function(void) {
    const char *source = "{[ (| ~ fetch ((url))[[)- response -(]]|) ]}";
    Lexer *lexer = lexer_create(source, "test_parse_async_function.lilith");
    AstNode *ast = parser_parse(lexer);
    assert(ast != NULL);
    assert(ast->type == AST_PROGRAM);
    ast_free(ast);
    lexer_destroy(lexer);
    printf("test_parse_async_function passed.\n");
}

static void test_parse_await(void) {
    const char *source = "{[ x [=] ~(fetch((url)))~ ]}";
    Lexer *lexer = lexer_create(source, "test_parse_await.lilith");
    AstNode *ast = parser_parse(lexer);
    assert(ast != NULL);
    assert(ast->type == AST_PROGRAM);
    ast_free(ast);
    lexer_destroy(lexer);
    printf("test_parse_await passed.\n");
}

static void test_parse_yield(void) {
    const char *source = "{[ )-? 42 ?-( ]}";
    Lexer *lexer = lexer_create(source, "test_parse_yield.lilith");
    AstNode *ast = parser_parse(lexer);
    assert(ast != NULL);
    assert(ast->type == AST_PROGRAM);
    ast_free(ast);
    lexer_destroy(lexer);
    printf("test_parse_yield passed.\n");
}

static void test_parse_parallel(void) {
    const char *source = "{[ <| [[x [=] 1]] |> ]}";
    Lexer *lexer = lexer_create(source, "test_parse_parallel.lilith");
    AstNode *ast = parser_parse(lexer);
    assert(ast != NULL);
    assert(ast->type == AST_PROGRAM);
    ast_free(ast);
    lexer_destroy(lexer);
    printf("test_parse_parallel passed.\n");
}

static void test_parse_gpu(void) {
    const char *source = "{[ <% [[x [=] 1]] %> ]}";
    Lexer *lexer = lexer_create(source, "test_parse_gpu.lilith");
    AstNode *ast = parser_parse(lexer);
    assert(ast != NULL);
    assert(ast->type == AST_PROGRAM);
    ast_free(ast);
    lexer_destroy(lexer);
    printf("test_parse_gpu passed.\n");
}

static void test_parse_tensor(void) {
    const char *source = "{[ [# [[x [=] 1]] #] ]}";
    Lexer *lexer = lexer_create(source, "test_parse_tensor.lilith");
    AstNode *ast = parser_parse(lexer);
    assert(ast != NULL);
    assert(ast->type == AST_PROGRAM);
    ast_free(ast);
    lexer_destroy(lexer);
    printf("test_parse_tensor passed.\n");
}

static void test_parse_stream(void) {
    const char *source = "{[ <~ [[x [=] 1]] ~> ]}";
    Lexer *lexer = lexer_create(source, "test_parse_stream.lilith");
    AstNode *ast = parser_parse(lexer);
    assert(ast != NULL);
    assert(ast->type == AST_PROGRAM);
    ast_free(ast);
    lexer_destroy(lexer);
    printf("test_parse_stream passed.\n");
}

static void test_parse_memory(void) {
    const char *source = "{[ [^ [[x [=] 1]] ^] ]}";
    Lexer *lexer = lexer_create(source, "test_parse_memory.lilith");
    AstNode *ast = parser_parse(lexer);
    assert(ast != NULL);
    assert(ast->type == AST_PROGRAM);
    ast_free(ast);
    lexer_destroy(lexer);
    printf("test_parse_memory passed.\n");
}
