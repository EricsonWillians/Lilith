#include "seq.h"

Value native_seq_len(int argc, Value *argv) {
    if (argc == 0) return NUMBER_VAL(0);
    Value v = argv[0];
    if (IS_STRING(v)) return NUMBER_VAL((double)AS_STRING(v)->length);
    if (IS_LIST(v))   return NUMBER_VAL((double)AS_LIST(v)->count);
    if (IS_TUPLE(v))  return NUMBER_VAL((double)AS_TUPLE(v)->count);
    if (IS_DICT(v))   return NUMBER_VAL((double)AS_DICT(v)->count);
    return NUMBER_VAL(0);
}
