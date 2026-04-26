#ifndef LILITH_VALUE_H
#define LILITH_VALUE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Forward declarations */
struct AstNode;
struct Environment;

/* -------------------------------------------------------------------------- */
/* Object system                                                              */
/* -------------------------------------------------------------------------- */

typedef struct Obj Obj;

typedef enum {
    OBJ_STRING,
    OBJ_LIST,
    OBJ_TUPLE,
    OBJ_DICT,
    OBJ_FUNCTION,
    OBJ_CLASS,
    OBJ_INSTANCE,
    OBJ_NATIVE,
} ObjType;

struct Obj {
    ObjType type;
    Obj *next; /* Intrusive GC list */
};

/* -------------------------------------------------------------------------- */
/* Value system                                                               */
/* -------------------------------------------------------------------------- */

typedef enum {
    VAL_NIL,
    VAL_BOOL,
    VAL_NUMBER,
    VAL_OBJ,
} ValueType;

typedef struct {
    ValueType type;
    union {
        bool boolean;
        double number;
        Obj *obj;
    } as;
} Value;

/* -------------------------------------------------------------------------- */
/* Object subtypes                                                            */
/* -------------------------------------------------------------------------- */

typedef struct {
    Obj obj;
    char *chars;
    size_t length;
    uint32_t hash;
} ObjString;

typedef struct {
    Obj obj;
    Value *items;
    size_t count;
    size_t capacity;
} ObjList;

typedef struct {
    Obj obj;
    Value *items;
    size_t count;
} ObjTuple;

typedef struct {
    ObjString *key;
    Value value;
} DictEntry;

typedef struct {
    Obj obj;
    DictEntry *entries;
    size_t count;
    size_t capacity;
} ObjDict;

typedef struct {
    Obj obj;
    char *name;
    char **params;
    char **param_types;   /* NULL = unannotated; parallel to params */
    size_t param_count;
    struct AstNode *body;
    char *return_type;    /* NULL = unannotated */
    int is_async;
    int implicit_return;
    struct Environment *closure;
} ObjFunction;

typedef struct {
    Obj obj;
    char *name;
    ObjDict *methods;
} ObjClass;

typedef struct {
    Obj obj;
    ObjClass *klass;
    ObjDict *fields;
} ObjInstance;

typedef Value (*NativeFn)(int arg_count, Value *args);

typedef struct {
    Obj obj;
    NativeFn fn;
    char *name;
} ObjNative;

/* -------------------------------------------------------------------------- */
/* Value macros                                                               */
/* -------------------------------------------------------------------------- */

#define NIL_VAL          ((Value){VAL_NIL,   { .number = 0 }})
#define BOOL_VAL(v)      ((Value){VAL_BOOL,  { .boolean = (v) }})
#define NUMBER_VAL(v)    ((Value){VAL_NUMBER,{ .number = (v) }})
#define OBJ_VAL(o)       ((Value){VAL_OBJ,   { .obj = (Obj*)(o) }})

#define AS_BOOL(v)       ((v).as.boolean)
#define AS_NUMBER(v)     ((v).as.number)
#define AS_OBJ(v)        ((v).as.obj)

#define IS_NIL(v)        ((v).type == VAL_NIL)
#define IS_BOOL(v)       ((v).type == VAL_BOOL)
#define IS_NUMBER(v)     ((v).type == VAL_NUMBER)
#define IS_OBJ(v)        ((v).type == VAL_OBJ)

#define IS_STRING(v)     (is_obj_type(v, OBJ_STRING))
#define IS_LIST(v)       (is_obj_type(v, OBJ_LIST))
#define IS_TUPLE(v)      (is_obj_type(v, OBJ_TUPLE))
#define IS_DICT(v)       (is_obj_type(v, OBJ_DICT))
#define IS_FUNCTION(v)   (is_obj_type(v, OBJ_FUNCTION))
#define IS_CLASS(v)      (is_obj_type(v, OBJ_CLASS))
#define IS_INSTANCE(v)   (is_obj_type(v, OBJ_INSTANCE))
#define IS_NATIVE(v)     (is_obj_type(v, OBJ_NATIVE))

#define AS_STRING(v)     ((ObjString*)AS_OBJ(v))
#define AS_LIST(v)       ((ObjList*)AS_OBJ(v))
#define AS_TUPLE(v)      ((ObjTuple*)AS_OBJ(v))
#define AS_DICT(v)       ((ObjDict*)AS_OBJ(v))
#define AS_FUNCTION(v)   ((ObjFunction*)AS_OBJ(v))
#define AS_CLASS(v)      ((ObjClass*)AS_OBJ(v))
#define AS_INSTANCE(v)   ((ObjInstance*)AS_OBJ(v))
#define AS_NATIVE(v)     ((ObjNative*)AS_OBJ(v))

static inline bool is_obj_type(Value value, ObjType type) {
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

/* -------------------------------------------------------------------------- */
/* Value API                                                                  */
/* -------------------------------------------------------------------------- */

Value value_nil(void);
Value value_bool(bool b);
Value value_number(double n);
Value value_obj(Obj *obj);

ObjString *obj_string_take(char *chars, size_t length);
ObjString *obj_string_copy(const char *chars, size_t length);
ObjList *obj_list_new(void);
ObjTuple *obj_tuple_new(size_t count);
ObjDict *obj_dict_new(void);
ObjFunction *obj_function_new(const char *name);
ObjClass *obj_class_new(const char *name);
ObjInstance *obj_instance_new(ObjClass *klass);
ObjNative *obj_native_new(NativeFn fn, const char *name);

void value_array_write(ObjList *list, Value value);
void value_print(Value value);
const char *value_to_string(Value value);
const char *value_type_name(Value value);
bool values_equal(Value a, Value b);

uint32_t hash_string(const char *key, size_t length);

void dict_set(ObjDict *dict, ObjString *key, Value value);
bool dict_get(ObjDict *dict, ObjString *key, Value *value);
bool dict_delete(ObjDict *dict, ObjString *key);

void free_object(Obj *obj);

#endif
