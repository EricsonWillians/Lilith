#define _GNU_SOURCE
#include "os.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

Value native_env_get(int argc, Value *argv) {
    if (argc < 1 || !IS_STRING(argv[0])) return NIL_VAL;
    const char *name = AS_STRING(argv[0])->chars;
    const char *val = getenv(name);
    if (!val) return NIL_VAL;
    return OBJ_VAL(obj_string_copy(val, strlen(val)));
}

Value native_env_set(int argc, Value *argv) {
    if (argc < 2 || !IS_STRING(argv[0]) || !IS_STRING(argv[1])) return BOOL_VAL(0);
    const char *name = AS_STRING(argv[0])->chars;
    const char *val = AS_STRING(argv[1])->chars;
    return BOOL_VAL(setenv(name, val, 1) == 0);
}

Value native_os_time(int argc, Value *argv) {
    (void)argc; (void)argv;
    return NUMBER_VAL((double)time(NULL));
}

Value native_os_sleep(int argc, Value *argv) {
    if (argc < 1 || !IS_NUMBER(argv[0])) return NIL_VAL;
    unsigned int sec = (unsigned int)AS_NUMBER(argv[0]);
    sleep(sec);
    return NIL_VAL;
}
