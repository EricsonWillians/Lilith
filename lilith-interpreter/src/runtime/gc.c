#define _GNU_SOURCE
#include "gc.h"
#include "value.h"
#include "environment.h"
#include <stdlib.h>

/* ========================================================================= */
/* Simple mark-and-sweep garbage collector                                   */
/* ========================================================================= */

static Obj *gc_objects = NULL;

void gc_track(void *obj) {
    Obj *o = (Obj *)obj;
    o->next = gc_objects;
    gc_objects = o;
}

static void mark_value(Value value);

static void mark_obj(Obj *obj) {
    if (!obj) return;
    if (obj->type & 0x80) return; /* Already marked */
    obj->type |= 0x80; /* Mark flag */

    switch (obj->type & 0x7F) {
        case OBJ_STRING: break;
        case OBJ_LIST: {
            ObjList *list = (ObjList *)obj;
            for (size_t i = 0; i < list->count; i++) {
                mark_value(list->items[i]);
            }
            break;
        }
        case OBJ_TUPLE: {
            ObjTuple *tuple = (ObjTuple *)obj;
            for (size_t i = 0; i < tuple->count; i++) {
                mark_value(tuple->items[i]);
            }
            break;
        }
        case OBJ_DICT: {
            ObjDict *dict = (ObjDict *)obj;
            for (size_t i = 0; i < dict->capacity; i++) {
                DictEntry *entry = &dict->entries[i];
                if (entry->key) {
                    mark_obj((Obj *)entry->key);
                    mark_value(entry->value);
                }
            }
            break;
        }
        case OBJ_FUNCTION: {
            ObjFunction *fn = (ObjFunction *)obj;
            if (fn->closure) {
                /* Mark closure environment's values dict */
                mark_obj((Obj *)fn->closure->values);
            }
            break;
        }
        case OBJ_CLASS: {
            ObjClass *klass = (ObjClass *)obj;
            mark_obj((Obj *)klass->methods);
            if (klass->superclass) mark_obj((Obj *)klass->superclass);
            break;
        }
        case OBJ_INSTANCE: {
            ObjInstance *inst = (ObjInstance *)obj;
            mark_obj((Obj *)inst->fields);
            mark_obj((Obj *)inst->klass);
            break;
        }
        case OBJ_NATIVE: break;
    }
}

static void mark_value(Value value) {
    if (IS_OBJ(value)) {
        mark_obj(AS_OBJ(value));
    }
}

void gc_collect(void) {
    /* Unmark all */
    for (Obj *obj = gc_objects; obj; obj = obj->next) {
        obj->type &= 0x7F;
    }

    /* TODO: Mark roots from interpreter globals, env chain, call stack */
    /* For now, this is a no-op sweep to prevent crashes until root marking is wired */

    /* Sweep unmarked objects */
    Obj **current = &gc_objects;
    while (*current) {
        Obj *obj = *current;
        if (!(obj->type & 0x80)) {
            *current = obj->next;
            free_object(obj);
        } else {
            obj->type &= 0x7F; /* Unmark for next cycle */
            current = &obj->next;
        }
    }
}
