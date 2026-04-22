#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "runtime/interpreter.h"

static char *read_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "Could not open file: %s\n", path);
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = (char *)malloc(size + 1);
    if (!buf) {
        fclose(f);
        return NULL;
    }
    fread(buf, 1, size, f);
    buf[size] = '\0';
    fclose(f);
    return buf;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file.lilith>\n", argv[0]);
        return 1;
    }

    char *source = read_file(argv[1]);
    if (!source) return 1;

    Lexer *lexer = lexer_create(source, argv[1]);
    AstNode *ast = parser_parse(lexer);

    if (!ast) {
        printf("Parse failed.\n");
        lexer_destroy(lexer);
        free(source);
        return 1;
    }

    Interpreter interp;
    interpreter_init(&interp);
    Value result = interpreter_run(&interp, ast);

    if (interp.throw_flag) {
        fprintf(stderr, "Runtime error: %s\n", interp.error_msg ? interp.error_msg : "unknown");
        interpreter_free(&interp);
        ast_free(ast);
        lexer_destroy(lexer);
        free(source);
        return 1;
    }

    interpreter_free(&interp);
    ast_free(ast);
    lexer_destroy(lexer);
    free(source);
    return 0;
}
