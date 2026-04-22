#include "num.h"
#include <stdlib.h>

Value native_num_from(int argc, Value *argv) {
    if (argc == 0) return NUMBER_VAL(0);
    Value v = argv[0];
    if (IS_NUMBER(v)) return v;
    if (IS_STRING(v)) {
        char *end;
        double d = strtod(AS_STRING(v)->chars, &end);
        if (*end == '\0') return NUMBER_VAL(d);
    }
    return NUMBER_VAL(0);
}
