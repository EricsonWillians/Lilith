#include "meta.h"

Value native_meta_type(int argc, Value *argv) {
    if (argc == 0) return OBJ_VAL(obj_string_copy("nil", 3));
    Value v = argv[0];
    if (IS_NIL(v))       return OBJ_VAL(obj_string_copy("nil", 3));
    if (IS_BOOL(v))      return OBJ_VAL(obj_string_copy("bool", 4));
    if (IS_NUMBER(v))    return OBJ_VAL(obj_string_copy("number", 6));
    if (IS_STRING(v))    return OBJ_VAL(obj_string_copy("string", 6));
    if (IS_LIST(v))      return OBJ_VAL(obj_string_copy("list", 4));
    if (IS_TUPLE(v))     return OBJ_VAL(obj_string_copy("tuple", 5));
    if (IS_DICT(v))      return OBJ_VAL(obj_string_copy("dict", 4));
    if (IS_FUNCTION(v))  return OBJ_VAL(obj_string_copy("function", 8));
    if (IS_CLASS(v))     return OBJ_VAL(obj_string_copy("class", 5));
    if (IS_INSTANCE(v))  return OBJ_VAL(obj_string_copy("instance", 8));
    if (IS_NATIVE(v))    return OBJ_VAL(obj_string_copy("native", 6));
    return OBJ_VAL(obj_string_copy("unknown", 7));
}
