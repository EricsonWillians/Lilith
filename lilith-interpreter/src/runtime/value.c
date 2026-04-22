#define _GNU_SOURCE
#include "value.h"
#include "gc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ========================================================================= */
/* Simple Value Constructors                                                */
/* ========================================================================= */

Value value_nil(void) { return NIL_VAL; }
Value value_bool(bool b) { return BOOL_VAL(b); }
Value value_number(double n) { return NUMBER_VAL(n); }
Value value_obj(Obj *obj) { return OBJ_VAL(obj); }

/* ========================================================================= */
/* Hashing                                                                  */
/* ========================================================================= */

uint32_t hash_string(const char *key, size_t length) {
    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < length; i++) {
        hash ^= (uint8_t)key[i];
        hash *= 16777619;
    }
    return hash;
}

/* ========================================================================= */
/* Object Constructors                                                      */
/* ========================================================================= */

static void *allocate_object(size_t size, ObjType type) {
    Obj *obj = (Obj *)malloc(size);
    if (!obj) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    obj->type = type;
    obj->next = NULL;
    gc_track(obj);
    return obj;
}

#define ALLOCATE_OBJ(type, obj_type) (type *)allocate_object(sizeof(type), obj_type)

ObjString *obj_string_take(char *chars, size_t length) {
    uint32_t hash = hash_string(chars, length);
    ObjString *str = ALLOCATE_OBJ(ObjString, OBJ_STRING);
    str->chars = chars;
    str->length = length;
    str->hash = hash;
    return str;
}

ObjString *obj_string_copy(const char *chars, size_t length) {
    char *heap_chars = (char *)malloc(length + 1);
    if (!heap_chars) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    memcpy(heap_chars, chars, length);
    heap_chars[length] = '\0';
    return obj_string_take(heap_chars, length);
}

ObjList *obj_list_new(void) {
    ObjList *list = ALLOCATE_OBJ(ObjList, OBJ_LIST);
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
    return list;
}

ObjTuple *obj_tuple_new(size_t count) {
    ObjTuple *tuple = ALLOCATE_OBJ(ObjTuple, OBJ_TUPLE);
    tuple->items = (Value *)malloc(sizeof(Value) * count);
    if (!tuple->items) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    tuple->count = count;
    return tuple;
}

ObjDict *obj_dict_new(void) {
    ObjDict *dict = ALLOCATE_OBJ(ObjDict, OBJ_DICT);
    dict->entries = NULL;
    dict->count = 0;
    dict->capacity = 0;
    return dict;
}

ObjFunction *obj_function_new(const char *name) {
    ObjFunction *fn = ALLOCATE_OBJ(ObjFunction, OBJ_FUNCTION);
    fn->name = name ? strdup(name) : NULL;
    fn->params = NULL;
    fn->param_count = 0;
    fn->body = NULL;
    fn->closure = NULL;
    return fn;
}

ObjClass *obj_class_new(const char *name) {
    ObjClass *klass = ALLOCATE_OBJ(ObjClass, OBJ_CLASS);
    klass->name = name ? strdup(name) : NULL;
    klass->methods = obj_dict_new();
    return klass;
}

ObjInstance *obj_instance_new(ObjClass *klass) {
    ObjInstance *inst = ALLOCATE_OBJ(ObjInstance, OBJ_INSTANCE);
    inst->klass = klass;
    inst->fields = obj_dict_new();
    return inst;
}

ObjNative *obj_native_new(NativeFn fn, const char *name) {
    ObjNative *native = ALLOCATE_OBJ(ObjNative, OBJ_NATIVE);
    native->fn = fn;
    native->name = name ? strdup(name) : NULL;
    return native;
}

/* ========================================================================= */
/* List Helpers                                                             */
/* ========================================================================= */

void value_array_write(ObjList *list, Value value) {
    if (list->capacity < list->count + 1) {
        size_t old = list->capacity;
        list->capacity = old < 8 ? 8 : old * 2;
        list->items = (Value *)realloc(list->items, sizeof(Value) * list->capacity);
        if (!list->items) {
            fprintf(stderr, "Out of memory\n");
            exit(1);
        }
    }
    list->items[list->count++] = value;
}

/* ========================================================================= */
/* Dict Helpers                                                             */
/* ========================================================================= */

static DictEntry *dict_find_entry(DictEntry *entries, size_t capacity, ObjString *key) {
    uint32_t index = key->hash & (capacity - 1);
    for (;;) {
        DictEntry *entry = &entries[index];
        if (entry->key == NULL || (entry->key == key)) {
            return entry;
        }
        if (entry->key != NULL && entry->key->hash == key->hash &&
            entry->key->length == key->length &&
            memcmp(entry->key->chars, key->chars, key->length) == 0) {
            return entry;
        }
        index = (index + 1) & (capacity - 1);
    }
}

static void dict_adjust_capacity(ObjDict *dict, size_t capacity) {
    DictEntry *entries = (DictEntry *)calloc(capacity, sizeof(DictEntry));
    if (!entries) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    for (size_t i = 0; i < dict->capacity; i++) {
        DictEntry *entry = &dict->entries[i];
        if (entry->key == NULL) continue;
        DictEntry *dest = dict_find_entry(entries, capacity, entry->key);
        dest->key = entry->key;
        dest->value = entry->value;
    }
    free(dict->entries);
    dict->entries = entries;
    dict->capacity = capacity;
}

void dict_set(ObjDict *dict, ObjString *key, Value value) {
    if (dict->count + 1 > dict->capacity * 0.75) {
        size_t capacity = dict->capacity < 8 ? 8 : dict->capacity * 2;
        dict_adjust_capacity(dict, capacity);
    }
    DictEntry *entry = dict_find_entry(dict->entries, dict->capacity, key);
    bool is_new = entry->key == NULL;
    entry->key = key;
    entry->value = value;
    if (is_new) dict->count++;
}

bool dict_get(ObjDict *dict, ObjString *key, Value *value) {
    if (dict->count == 0) return false;
    DictEntry *entry = dict_find_entry(dict->entries, dict->capacity, key);
    if (entry->key == NULL) return false;
    *value = entry->value;
    return true;
}

bool dict_delete(ObjDict *dict, ObjString *key) {
    if (dict->count == 0) return false;
    DictEntry *entry = dict_find_entry(dict->entries, dict->capacity, key);
    if (entry->key == NULL) return false;
    entry->key = NULL;
    entry->value = NIL_VAL;
    return true;
}

/* ========================================================================= */
/* Value Representation                                                     */
/* ========================================================================= */

void value_print(Value value) {
    switch (value.type) {
        case VAL_NIL:    printf("nil"); break;
        case VAL_BOOL:   printf(AS_BOOL(value) ? "true" : "false"); break;
        case VAL_NUMBER: printf("%.14g", AS_NUMBER(value)); break;
        case VAL_OBJ: {
            switch (AS_OBJ(value)->type) {
                case OBJ_STRING:  printf("%s", AS_STRING(value)->chars); break;
                case OBJ_LIST: {
                    printf("[< ");
                    ObjList *list = AS_LIST(value);
                    for (size_t i = 0; i < list->count; i++) {
                        value_print(list->items[i]);
                        if (i + 1 < list->count) printf(",, ");
                    }
                    printf(" >]");
                    break;
                }
                case OBJ_TUPLE: {
                    printf("(< ");
                    ObjTuple *tuple = AS_TUPLE(value);
                    for (size_t i = 0; i < tuple->count; i++) {
                        value_print(tuple->items[i]);
                        if (i + 1 < tuple->count) printf(",, ");
                    }
                    printf(" >)");
                    break;
                }
                case OBJ_DICT: {
                    printf("{< ");
                    ObjDict *dict = AS_DICT(value);
                    size_t printed = 0;
                    for (size_t i = 0; i < dict->capacity; i++) {
                        DictEntry *entry = &dict->entries[i];
                        if (entry->key == NULL) continue;
                        value_print(OBJ_VAL(entry->key));
                        printf(" [:] ");
                        value_print(entry->value);
                        if (++printed < dict->count) printf(",, ");
                    }
                    printf(" >}");
                    break;
                }
                case OBJ_FUNCTION: {
                    ObjFunction *fn = AS_FUNCTION(value);
                    printf("<fn %s>", fn->name ? fn->name : "<lambda>");
                    break;
                }
                case OBJ_CLASS:     printf("<class %s>", AS_CLASS(value)->name); break;
                case OBJ_INSTANCE:  printf("<instance %s>", AS_INSTANCE(value)->klass->name); break;
                case OBJ_NATIVE:    printf("<native fn %s>", AS_NATIVE(value)->name); break;
            }
            break;
        }
    }
}

const char *value_to_string(Value value) {
    static char buffer[256];
    switch (value.type) {
        case VAL_NIL:    return "nil";
        case VAL_BOOL:   return AS_BOOL(value) ? "true" : "false";
        case VAL_NUMBER:
            snprintf(buffer, sizeof(buffer), "%.14g", AS_NUMBER(value));
            return buffer;
        case VAL_OBJ:
            if (IS_STRING(value)) return AS_STRING(value)->chars;
            return "<object>";
    }
    return "<unknown>";
}

/* ========================================================================= */
/* Equality                                                                 */
/* ========================================================================= */

bool values_equal(Value a, Value b) {
    if (a.type != b.type) return false;
    switch (a.type) {
        case VAL_NIL:    return true;
        case VAL_BOOL:   return AS_BOOL(a) == AS_BOOL(b);
        case VAL_NUMBER: return AS_NUMBER(a) == AS_NUMBER(b);
        case VAL_OBJ: {
            if (AS_OBJ(a)->type != AS_OBJ(b)->type) return false;
            if (IS_STRING(a)) {
                ObjString *as = AS_STRING(a);
                ObjString *bs = AS_STRING(b);
                return as->length == bs->length && memcmp(as->chars, bs->chars, as->length) == 0;
            }
            return AS_OBJ(a) == AS_OBJ(b);
        }
    }
    return false;
}

/* ========================================================================= */
/* Object Cleanup                                                           */
/* ========================================================================= */

void free_object(Obj *obj) {
    if (!obj) return;
    switch (obj->type) {
        case OBJ_STRING: {
            ObjString *s = (ObjString *)obj;
            free(s->chars);
            free(s);
            break;
        }
        case OBJ_LIST: {
            ObjList *l = (ObjList *)obj;
            free(l->items);
            free(l);
            break;
        }
        case OBJ_TUPLE: {
            ObjTuple *t = (ObjTuple *)obj;
            free(t->items);
            free(t);
            break;
        }
        case OBJ_DICT: {
            ObjDict *d = (ObjDict *)obj;
            free(d->entries);
            free(d);
            break;
        }
        case OBJ_FUNCTION: {
            ObjFunction *f = (ObjFunction *)obj;
            free(f->name);
            free(f->params);
            free(f);
            break;
        }
        case OBJ_CLASS: {
            ObjClass *c = (ObjClass *)obj;
            free(c->name);
            /* methods dict is tracked separately by GC */
            free(c);
            break;
        }
        case OBJ_INSTANCE: {
            ObjInstance *i = (ObjInstance *)obj;
            /* fields dict is tracked separately by GC */
            free(i);
            break;
        }
        case OBJ_NATIVE: {
            ObjNative *n = (ObjNative *)obj;
            free(n->name);
            free(n);
            break;
        }
    }
}
