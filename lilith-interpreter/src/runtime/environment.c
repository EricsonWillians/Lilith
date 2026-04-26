#define _GNU_SOURCE
#include "environment.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Environment *env_new(void) {
    Environment *env = (Environment *)malloc(sizeof(Environment));
    if (!env) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    env->values = obj_dict_new();
    env->enclosing = NULL;
    return env;
}

Environment *env_new_enclosing(Environment *enclosing) {
    Environment *env = env_new();
    env->enclosing = enclosing;
    return env;
}

void env_free(Environment *env) {
    if (!env) return;
    /* values dict is GC-managed; do not free here */
    free(env);
}

void env_define(Environment *env, const char *name, Value value) {
    ObjString *key = obj_string_copy(name, strlen(name));
    dict_set(env->values, key, value);
}

int env_get(Environment *env, const char *name, Value *out) {
    ObjString key;
    key.obj.type = OBJ_STRING;
    key.chars = (char *)name;
    key.length = strlen(name);
    key.hash = hash_string(name, key.length);

    if (dict_get(env->values, &key, out)) return 1;
    if (env->enclosing) return env_get(env->enclosing, name, out);
    return 0;
}

int env_set(Environment *env, const char *name, Value value) {
    ObjString key;
    key.obj.type = OBJ_STRING;
    key.chars = (char *)name;
    key.length = strlen(name);
    key.hash = hash_string(name, key.length);

    /* If it exists in this scope, update it */
    Value dummy;
    if (dict_get(env->values, &key, &dummy)) {
        dict_set(env->values, &key, value);
        return 1;
    }
    /* Otherwise walk up */
    if (env->enclosing) return env_set(env->enclosing, name, value);
    return 0;
}

int env_get_at(Environment *env, const char *name, Value *out, int depth) {
    Environment *current = env;
    for (int i = 0; i < depth && current; i++) {
        current = current->enclosing;
    }
    if (!current) return 0;
    return env_get(current, name, out);
}

int env_set_at(Environment *env, const char *name, Value value, int depth) {
    Environment *current = env;
    for (int i = 0; i < depth && current; i++) {
        current = current->enclosing;
    }
    if (!current) return 0;
    ObjString key;
    key.obj.type = OBJ_STRING;
    key.chars = (char *)name;
    key.length = strlen(name);
    key.hash = hash_string(name, key.length);
    dict_set(current->values, &key, value);
    return 1;
}
