#include "time.h"
#include <time.h>

Value native_time_clock(int argc, Value *argv) {
    (void)argc; (void)argv;
    return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}
