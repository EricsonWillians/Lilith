#include "string.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

Value native_str_len(int argc, Value *argv) {
    if (argc < 1 || !IS_STRING(argv[0])) return NUMBER_VAL(0);
    return NUMBER_VAL((double)AS_STRING(argv[0])->length);
}

Value native_str_trim(int argc, Value *argv) {
    if (argc < 1 || !IS_STRING(argv[0])) return argv[0];
    const char *s = AS_STRING(argv[0])->chars;
    size_t len = AS_STRING(argv[0])->length;
    while (len > 0 && isspace((unsigned char)s[0])) { s++; len--; }
    while (len > 0 && isspace((unsigned char)s[len - 1])) { len--; }
    return OBJ_VAL(obj_string_copy(s, len));
}

Value native_str_contains(int argc, Value *argv) {
    if (argc < 2 || !IS_STRING(argv[0]) || !IS_STRING(argv[1])) return BOOL_VAL(0);
    return BOOL_VAL(strstr(AS_STRING(argv[0])->chars, AS_STRING(argv[1])->chars) != NULL);
}

Value native_str_starts_with(int argc, Value *argv) {
    if (argc < 2 || !IS_STRING(argv[0]) || !IS_STRING(argv[1])) return BOOL_VAL(0);
    const char *s = AS_STRING(argv[0])->chars;
    const char *prefix = AS_STRING(argv[1])->chars;
    return BOOL_VAL(strncmp(s, prefix, strlen(prefix)) == 0);
}

Value native_str_ends_with(int argc, Value *argv) {
    if (argc < 2 || !IS_STRING(argv[0]) || !IS_STRING(argv[1])) return BOOL_VAL(0);
    const char *s = AS_STRING(argv[0])->chars;
    const char *suffix = AS_STRING(argv[1])->chars;
    size_t slen = strlen(s);
    size_t suflen = strlen(suffix);
    if (suflen > slen) return BOOL_VAL(0);
    return BOOL_VAL(strcmp(s + slen - suflen, suffix) == 0);
}

Value native_str_replace(int argc, Value *argv) {
    if (argc < 3 || !IS_STRING(argv[0]) || !IS_STRING(argv[1]) || !IS_STRING(argv[2]))
        return argv[0];
    const char *src = AS_STRING(argv[0])->chars;
    const char *from = AS_STRING(argv[1])->chars;
    const char *to = AS_STRING(argv[2])->chars;
    size_t from_len = strlen(from);
    size_t to_len = strlen(to);

    size_t count = 0;
    const char *tmp = src;
    while ((tmp = strstr(tmp, from)) != NULL) { count++; tmp += from_len; }

    size_t result_len = strlen(src) + count * (to_len - from_len);
    char *result = (char *)malloc(result_len + 1);
    if (!result) return argv[0];

    char *dst = result;
    while (*src) {
        const char *match = strstr(src, from);
        if (match) {
            size_t prefix = (size_t)(match - src);
            memcpy(dst, src, prefix);
            dst += prefix;
            memcpy(dst, to, to_len);
            dst += to_len;
            src = match + from_len;
        } else {
            size_t rest = strlen(src);
            memcpy(dst, src, rest);
            dst += rest;
            break;
        }
    }
    *dst = '\0';
    return OBJ_VAL(obj_string_take(result, (size_t)(dst - result)));
}

Value native_str_substring(int argc, Value *argv) {
    if (argc < 2 || !IS_STRING(argv[0]) || !IS_NUMBER(argv[1])) return argv[0];
    const char *s = AS_STRING(argv[0])->chars;
    size_t len = AS_STRING(argv[0])->length;
    size_t start = (size_t)AS_NUMBER(argv[1]);
    if (start > len) start = len;
    size_t end = len;
    if (argc >= 3 && IS_NUMBER(argv[2])) {
        end = (size_t)AS_NUMBER(argv[2]);
        if (end > len) end = len;
    }
    if (end < start) end = start;
    return OBJ_VAL(obj_string_copy(s + start, end - start));
}

Value native_str_split(int argc, Value *argv) {
    if (argc < 2 || !IS_STRING(argv[0]) || !IS_STRING(argv[1])) {
        ObjList *list = obj_list_new();
        return OBJ_VAL(list);
    }
    const char *s = AS_STRING(argv[0])->chars;
    const char *delim = AS_STRING(argv[1])->chars;
    size_t dlen = strlen(delim);
    ObjList *list = obj_list_new();

    const char *p = s;
    while (*p) {
        const char *match = strstr(p, delim);
        if (match) {
            value_array_write(list, OBJ_VAL(obj_string_copy(p, (size_t)(match - p))));
            p = match + dlen;
        } else {
            value_array_write(list, OBJ_VAL(obj_string_copy(p, strlen(p))));
            break;
        }
    }
    return OBJ_VAL(list);
}

Value native_str_join(int argc, Value *argv) {
    if (argc < 2 || !IS_LIST(argv[1])) return OBJ_VAL(obj_string_copy("", 0));
    const char *sep = argc >= 1 && IS_STRING(argv[0]) ? AS_STRING(argv[0])->chars : "";
    size_t sep_len = strlen(sep);
    ObjList *list = AS_LIST(argv[1]);

    size_t total = 0;
    for (size_t i = 0; i < list->count; i++) {
        if (IS_STRING(list->items[i])) {
            total += AS_STRING(list->items[i])->length;
        }
    }
    if (list->count > 1) total += sep_len * (list->count - 1);

    char *buf = (char *)malloc(total + 1);
    if (!buf) return OBJ_VAL(obj_string_copy("", 0));
    buf[0] = '\0';
    char *p = buf;
    for (size_t i = 0; i < list->count; i++) {
        if (i > 0) {
            memcpy(p, sep, sep_len);
            p += sep_len;
        }
        if (IS_STRING(list->items[i])) {
            size_t len = AS_STRING(list->items[i])->length;
            memcpy(p, AS_STRING(list->items[i])->chars, len);
            p += len;
        }
    }
    *p = '\0';
    return OBJ_VAL(obj_string_take(buf, total));
}
