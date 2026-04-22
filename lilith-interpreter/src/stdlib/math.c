#include "math.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>

static int seeded = 0;

static void ensure_seed(void) {
    if (!seeded) {
        srand((unsigned int)time(NULL));
        seeded = 1;
    }
}

Value native_math_abs(int argc, Value *argv) {
    if (argc < 1 || !IS_NUMBER(argv[0])) return NUMBER_VAL(0);
    return NUMBER_VAL(fabs(AS_NUMBER(argv[0])));
}

Value native_math_floor(int argc, Value *argv) {
    if (argc < 1 || !IS_NUMBER(argv[0])) return NUMBER_VAL(0);
    return NUMBER_VAL(floor(AS_NUMBER(argv[0])));
}

Value native_math_ceil(int argc, Value *argv) {
    if (argc < 1 || !IS_NUMBER(argv[0])) return NUMBER_VAL(0);
    return NUMBER_VAL(ceil(AS_NUMBER(argv[0])));
}

Value native_math_sqrt(int argc, Value *argv) {
    if (argc < 1 || !IS_NUMBER(argv[0])) return NUMBER_VAL(0);
    return NUMBER_VAL(sqrt(AS_NUMBER(argv[0])));
}

Value native_math_pow(int argc, Value *argv) {
    if (argc < 2 || !IS_NUMBER(argv[0]) || !IS_NUMBER(argv[1])) return NUMBER_VAL(0);
    return NUMBER_VAL(pow(AS_NUMBER(argv[0]), AS_NUMBER(argv[1])));
}

Value native_math_sin(int argc, Value *argv) {
    if (argc < 1 || !IS_NUMBER(argv[0])) return NUMBER_VAL(0);
    return NUMBER_VAL(sin(AS_NUMBER(argv[0])));
}

Value native_math_cos(int argc, Value *argv) {
    if (argc < 1 || !IS_NUMBER(argv[0])) return NUMBER_VAL(0);
    return NUMBER_VAL(cos(AS_NUMBER(argv[0])));
}

Value native_math_tan(int argc, Value *argv) {
    if (argc < 1 || !IS_NUMBER(argv[0])) return NUMBER_VAL(0);
    return NUMBER_VAL(tan(AS_NUMBER(argv[0])));
}

Value native_math_pi(int argc, Value *argv) {
    (void)argc; (void)argv;
    return NUMBER_VAL(3.14159265358979323846);
}

Value native_math_e(int argc, Value *argv) {
    (void)argc; (void)argv;
    return NUMBER_VAL(2.71828182845904523536);
}

Value native_math_rand(int argc, Value *argv) {
    ensure_seed();
    if (argc >= 2 && IS_NUMBER(argv[0]) && IS_NUMBER(argv[1])) {
        int min = (int)AS_NUMBER(argv[0]);
        int max = (int)AS_NUMBER(argv[1]);
        if (max <= min) return NUMBER_VAL(min);
        return NUMBER_VAL((double)(min + rand() % (max - min)));
    }
    return NUMBER_VAL((double)rand() / (double)RAND_MAX);
}
