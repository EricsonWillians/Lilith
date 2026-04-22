#define _GNU_SOURCE
#include "io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

/* ========================================================================= */
/* HTTP client (simple blocking GET over TCP)                                */
/* ========================================================================= */

static int parse_url(const char *url, char *host, size_t host_size,
                     char *path, size_t path_size, int *port) {
    *port = 80;
    if (strncmp(url, "http://", 7) == 0) {
        url += 7;
    }

    const char *slash = strchr(url, '/');
    if (slash) {
        size_t host_len = (size_t)(slash - url);
        if (host_len >= host_size) host_len = host_size - 1;
        memcpy(host, url, host_len);
        host[host_len] = '\0';

        size_t path_len = strlen(slash);
        if (path_len >= path_size) path_len = path_size - 1;
        memcpy(path, slash, path_len);
        path[path_len] = '\0';
    } else {
        size_t host_len = strlen(url);
        if (host_len >= host_size) host_len = host_size - 1;
        memcpy(host, url, host_len);
        host[host_len] = '\0';
        path[0] = '/';
        path[1] = '\0';
    }

    char *colon = strchr(host, ':');
    if (colon) {
        *colon = '\0';
        *port = atoi(colon + 1);
    }
    return 0;
}

static char *http_request(const char *host, int port, const char *path) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return NULL;

    struct hostent *server = gethostbyname(host);
    if (!server) {
        close(sock);
        return NULL;
    }

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    memcpy(&addr.sin_addr.s_addr, server->h_addr, server->h_length);

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(sock);
        return NULL;
    }

    char request[4096];
    int req_len = snprintf(request, sizeof(request),
        "GET %s HTTP/1.0\r\n"
        "Host: %s\r\n"
        "User-Agent: Lilith-Interpreter/1.0\r\n"
        "Connection: close\r\n"
        "\r\n",
        path, host);

    if (send(sock, request, req_len, 0) != req_len) {
        close(sock);
        return NULL;
    }

    size_t capacity = 4096;
    size_t count = 0;
    char *response = (char *)malloc(capacity);
    if (!response) {
        close(sock);
        return NULL;
    }

    ssize_t n;
    while ((n = recv(sock, response + count, capacity - count - 1, 0)) > 0) {
        count += (size_t)n;
        if (count + 1 >= capacity) {
            capacity *= 2;
            char *tmp = (char *)realloc(response, capacity);
            if (!tmp) {
                free(response);
                close(sock);
                return NULL;
            }
            response = tmp;
        }
    }
    response[count] = '\0';
    close(sock);
    return response;
}

static char *extract_body(const char *response) {
    const char *sep = strstr(response, "\r\n\r\n");
    if (!sep) sep = strstr(response, "\n\n");
    if (!sep) return strdup(response);
    sep += (sep[1] == '\r') ? 4 : 2;
    return strdup(sep);
}

Value native_http_get(int argc, Value *argv) {
    if (argc < 1 || !IS_STRING(argv[0])) {
        return NIL_VAL;
    }
    const char *url = AS_STRING(argv[0])->chars;

    char host[256];
    char path[1024];
    int port;
    if (parse_url(url, host, sizeof(host), path, sizeof(path), &port) != 0) {
        return NIL_VAL;
    }

    char *response = http_request(host, port, path);
    if (!response) return NIL_VAL;

    char *body = extract_body(response);
    free(response);
    if (!body) return NIL_VAL;

    Value result = OBJ_VAL(obj_string_take(body, strlen(body)));
    return result;
}

/* ========================================================================= */
/* File I/O                                                                  */
/* ========================================================================= */

Value native_file_read(int argc, Value *argv) {
    if (argc < 1 || !IS_STRING(argv[0])) return NIL_VAL;
    const char *path = AS_STRING(argv[0])->chars;

    FILE *f = fopen(path, "rb");
    if (!f) return NIL_VAL;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buf = (char *)malloc(size + 1);
    if (!buf) {
        fclose(f);
        return NIL_VAL;
    }
    size_t rd = fread(buf, 1, size, f);
    buf[rd] = '\0';
    fclose(f);

    return OBJ_VAL(obj_string_take(buf, size));
}

Value native_file_write(int argc, Value *argv) {
    if (argc < 2 || !IS_STRING(argv[0]) || !IS_STRING(argv[1])) {
        return BOOL_VAL(0);
    }
    const char *path = AS_STRING(argv[0])->chars;
    const char *content = AS_STRING(argv[1])->chars;

    FILE *f = fopen(path, "wb");
    if (!f) return BOOL_VAL(0);

    size_t written = fwrite(content, 1, strlen(content), f);
    fclose(f);
    return BOOL_VAL(written == strlen(content));
}

/* ========================================================================= */
/* Process control                                                           */
/* ========================================================================= */

Value native_exit(int argc, Value *argv) {
    int code = 0;
    if (argc >= 1 && IS_NUMBER(argv[0])) {
        code = (int)AS_NUMBER(argv[0]);
    }
    exit(code);
    return NIL_VAL;
}
