#include "list.h"
#include <stdlib.h>
#include <string.h>

Value native_list_push(int argc, Value *argv) {
    if (argc < 2 || !IS_LIST(argv[0])) return BOOL_VAL(0);
    value_array_write(AS_LIST(argv[0]), argv[1]);
    return BOOL_VAL(1);
}

Value native_list_pop(int argc, Value *argv) {
    (void)argc;
    if (!IS_LIST(argv[0])) return NIL_VAL;
    ObjList *list = AS_LIST(argv[0]);
    if (list->count == 0) return NIL_VAL;
    list->count--;
    return list->items[list->count];
}

Value native_list_find(int argc, Value *argv) {
    if (argc < 2 || !IS_LIST(argv[0])) return NUMBER_VAL(-1);
    ObjList *list = AS_LIST(argv[0]);
    for (size_t i = 0; i < list->count; i++) {
        if (values_equal(list->items[i], argv[1])) return NUMBER_VAL((double)i);
    }
    return NUMBER_VAL(-1);
}

static int compare_values(const void *a, const void *b) {
    Value *va = (Value *)a;
    Value *vb = (Value *)b;
    if (IS_NUMBER(*va) && IS_NUMBER(*vb)) {
        double diff = AS_NUMBER(*va) - AS_NUMBER(*vb);
        return (diff < 0) ? -1 : (diff > 0) ? 1 : 0;
    }
    if (IS_STRING(*va) && IS_STRING(*vb)) {
        return strcmp(AS_STRING(*va)->chars, AS_STRING(*vb)->chars);
    }
    return 0;
}

Value native_list_sort(int argc, Value *argv) {
    (void)argc;
    if (!IS_LIST(argv[0])) return BOOL_VAL(0);
    ObjList *list = AS_LIST(argv[0]);
    if (list->count > 1) {
        qsort(list->items, list->count, sizeof(Value), compare_values);
    }
    return OBJ_VAL(list);
}
