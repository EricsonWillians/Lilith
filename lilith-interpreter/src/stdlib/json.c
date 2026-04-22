#include "json.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

/* ========================================================================= */
/* JSON Encoder                                                              */
/* ========================================================================= */

static void json_encode_value(Value val, char **buf, size_t *len, size_t *cap);

static void ensure_cap(char **buf, size_t *len, size_t *cap, size_t need) {
    while (*len + need >= *cap) {
        *cap = *cap < 64 ? 64 : *cap * 2;
        *buf = (char *)realloc(*buf, *cap);
    }
}

static void append_str(char **buf, size_t *len, size_t *cap, const char *s) {
    size_t sl = strlen(s);
    ensure_cap(buf, len, cap, sl + 1);
    memcpy(*buf + *len, s, sl);
    *len += sl;
    (*buf)[*len] = '\0';
}

static void append_char(char **buf, size_t *len, size_t *cap, char c) {
    ensure_cap(buf, len, cap, 2);
    (*buf)[*len] = c;
    (*len)++;
    (*buf)[*len] = '\0';
}

static void json_encode_string(const char *s, char **buf, size_t *len, size_t *cap) {
    append_char(buf, len, cap, '"');
    for (const char *p = s; *p; p++) {
        switch (*p) {
            case '"': append_str(buf, len, cap, "\\\""); break;
            case '\\': append_str(buf, len, cap, "\\\\"); break;
            case '\b': append_str(buf, len, cap, "\\b"); break;
            case '\f': append_str(buf, len, cap, "\\f"); break;
            case '\n': append_str(buf, len, cap, "\\n"); break;
            case '\r': append_str(buf, len, cap, "\\r"); break;
            case '\t': append_str(buf, len, cap, "\\t"); break;
            default:
                if ((unsigned char)*p < 0x20) {
                    char esc[7];
                    snprintf(esc, sizeof(esc), "\\u%04x", (unsigned char)*p);
                    append_str(buf, len, cap, esc);
                } else {
                    append_char(buf, len, cap, *p);
                }
        }
    }
    append_char(buf, len, cap, '"');
}

static void json_encode_value(Value val, char **buf, size_t *len, size_t *cap) {
    if (IS_NIL(val)) {
        append_str(buf, len, cap, "null");
    } else if (IS_BOOL(val)) {
        append_str(buf, len, cap, AS_BOOL(val) ? "true" : "false");
    } else if (IS_NUMBER(val)) {
        char num[64];
        snprintf(num, sizeof(num), "%.14g", AS_NUMBER(val));
        append_str(buf, len, cap, num);
    } else if (IS_STRING(val)) {
        json_encode_string(AS_STRING(val)->chars, buf, len, cap);
    } else if (IS_LIST(val)) {
        ObjList *list = AS_LIST(val);
        append_char(buf, len, cap, '[');
        for (size_t i = 0; i < list->count; i++) {
            if (i > 0) append_char(buf, len, cap, ',');
            json_encode_value(list->items[i], buf, len, cap);
        }
        append_char(buf, len, cap, ']');
    } else if (IS_DICT(val)) {
        ObjDict *dict = AS_DICT(val);
        append_char(buf, len, cap, '{');
        int first = 1;
        for (size_t i = 0; i < dict->capacity; i++) {
            DictEntry *e = &dict->entries[i];
            if (e->key == NULL) continue;
            if (!first) append_char(buf, len, cap, ',');
            first = 0;
            json_encode_string(e->key->chars, buf, len, cap);
            append_char(buf, len, cap, ':');
            json_encode_value(e->value, buf, len, cap);
        }
        append_char(buf, len, cap, '}');
    } else {
        append_str(buf, len, cap, "null");
    }
}

Value native_json_encode(int argc, Value *argv) {
    if (argc < 1) return OBJ_VAL(obj_string_copy("null", 4));
    size_t cap = 256;
    size_t len = 0;
    char *buf = (char *)malloc(cap);
    if (!buf) return OBJ_VAL(obj_string_copy("null", 4));
    buf[0] = '\0';
    json_encode_value(argv[0], &buf, &len, &cap);
    return OBJ_VAL(obj_string_take(buf, len));
}

/* ========================================================================= */
/* JSON Decoder (minimal recursive descent)                                  */
/* ========================================================================= */

typedef struct {
    const char *s;
    size_t pos;
    size_t len;
} JsonParser;

static void json_skip_ws(JsonParser *p) {
    while (p->pos < p->len && isspace((unsigned char)p->s[p->pos])) p->pos++;
}

static Value json_parse_value(JsonParser *p);

static Value json_parse_string(JsonParser *p) {
    if (p->s[p->pos] != '"') return NIL_VAL;
    p->pos++;
    size_t cap = 64;
    size_t len = 0;
    char *buf = (char *)malloc(cap);
    if (!buf) return NIL_VAL;
    while (p->pos < p->len && p->s[p->pos] != '"') {
        if (p->s[p->pos] == '\\' && p->pos + 1 < p->len) {
            p->pos++;
            switch (p->s[p->pos]) {
                case '"': buf[len++] = '"'; break;
                case '\\': buf[len++] = '\\'; break;
                case '/': buf[len++] = '/'; break;
                case 'b': buf[len++] = '\b'; break;
                case 'f': buf[len++] = '\f'; break;
                case 'n': buf[len++] = '\n'; break;
                case 'r': buf[len++] = '\r'; break;
                case 't': buf[len++] = '\t'; break;
                case 'u': {
                    if (p->pos + 4 < p->len) {
                        char hex[5] = {0};
                        memcpy(hex, p->s + p->pos + 1, 4);
                        unsigned int cp = (unsigned int)strtol(hex, NULL, 16);
                        /* Minimal: just store low byte */
                        buf[len++] = (char)(cp & 0xFF);
                        p->pos += 4;
                    }
                    break;
                }
                default: buf[len++] = p->s[p->pos]; break;
            }
        } else {
            buf[len++] = p->s[p->pos];
        }
        p->pos++;
        if (len + 2 >= cap) {
            cap *= 2;
            buf = (char *)realloc(buf, cap);
        }
    }
    if (p->pos < p->len && p->s[p->pos] == '"') p->pos++;
    buf[len] = '\0';
    return OBJ_VAL(obj_string_take(buf, len));
}

static Value json_parse_number(JsonParser *p) {
    size_t start = p->pos;
    if (p->s[p->pos] == '-') p->pos++;
    while (p->pos < p->len && isdigit((unsigned char)p->s[p->pos])) p->pos++;
    if (p->pos < p->len && p->s[p->pos] == '.') {
        p->pos++;
        while (p->pos < p->len && isdigit((unsigned char)p->s[p->pos])) p->pos++;
    }
    if (p->pos < p->len && (p->s[p->pos] == 'e' || p->s[p->pos] == 'E')) {
        p->pos++;
        if (p->pos < p->len && (p->s[p->pos] == '+' || p->s[p->pos] == '-')) p->pos++;
        while (p->pos < p->len && isdigit((unsigned char)p->s[p->pos])) p->pos++;
    }
    char *sub = (char *)malloc(p->pos - start + 1);
    memcpy(sub, p->s + start, p->pos - start);
    sub[p->pos - start] = '\0';
    double d = strtod(sub, NULL);
    free(sub);
    return NUMBER_VAL(d);
}

static Value json_parse_array(JsonParser *p) {
    p->pos++;
    ObjList *list = obj_list_new();
    json_skip_ws(p);
    if (p->pos < p->len && p->s[p->pos] == ']') {
        p->pos++;
        return OBJ_VAL(list);
    }
    for (;;) {
        json_skip_ws(p);
        Value elem = json_parse_value(p);
        value_array_write(list, elem);
        json_skip_ws(p);
        if (p->pos < p->len && p->s[p->pos] == ',') {
            p->pos++;
            continue;
        }
        if (p->pos < p->len && p->s[p->pos] == ']') {
            p->pos++;
            break;
        }
        break;
    }
    return OBJ_VAL(list);
}

static Value json_parse_object(JsonParser *p) {
    p->pos++;
    ObjDict *dict = obj_dict_new();
    json_skip_ws(p);
    if (p->pos < p->len && p->s[p->pos] == '}') {
        p->pos++;
        return OBJ_VAL(dict);
    }
    for (;;) {
        json_skip_ws(p);
        if (p->s[p->pos] != '"') break;
        Value key = json_parse_string(p);
        json_skip_ws(p);
        if (p->pos >= p->len || p->s[p->pos] != ':') break;
        p->pos++;
        json_skip_ws(p);
        Value val = json_parse_value(p);
        if (IS_STRING(key)) {
            dict_set(dict, AS_STRING(key), val);
        }
        json_skip_ws(p);
        if (p->pos < p->len && p->s[p->pos] == ',') {
            p->pos++;
            continue;
        }
        if (p->pos < p->len && p->s[p->pos] == '}') {
            p->pos++;
            break;
        }
        break;
    }
    return OBJ_VAL(dict);
}

static Value json_parse_value(JsonParser *p) {
    json_skip_ws(p);
    if (p->pos >= p->len) return NIL_VAL;
    char c = p->s[p->pos];
    if (c == '"') return json_parse_string(p);
    if (c == '[') return json_parse_array(p);
    if (c == '{') return json_parse_object(p);
    if (c == 't' && p->pos + 4 <= p->len && strncmp(p->s + p->pos, "true", 4) == 0) {
        p->pos += 4;
        return BOOL_VAL(1);
    }
    if (c == 'f' && p->pos + 5 <= p->len && strncmp(p->s + p->pos, "false", 5) == 0) {
        p->pos += 5;
        return BOOL_VAL(0);
    }
    if (c == 'n' && p->pos + 4 <= p->len && strncmp(p->s + p->pos, "null", 4) == 0) {
        p->pos += 4;
        return NIL_VAL;
    }
    if (c == '-' || isdigit((unsigned char)c)) return json_parse_number(p);
    return NIL_VAL;
}

Value native_json_decode(int argc, Value *argv) {
    if (argc < 1 || !IS_STRING(argv[0])) return NIL_VAL;
    JsonParser p;
    p.s = AS_STRING(argv[0])->chars;
    p.pos = 0;
    p.len = AS_STRING(argv[0])->length;
    return json_parse_value(&p);
}
