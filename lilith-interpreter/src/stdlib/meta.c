#include "meta.h"
#include "runtime/value.h"
#include <string.h>

Value native_meta_type(int argc, Value *argv) {
    if (argc == 0) return OBJ_VAL(obj_string_copy("nil", 3));
    const char *name = value_type_name(argv[0]);
    return OBJ_VAL(obj_string_copy(name, strlen(name)));
}
