#ifndef LILITH_STDMATH_H
#define LILITH_STDMATH_H

#include "runtime/value.h"

Value native_math_abs(int argc, Value *argv);
Value native_math_floor(int argc, Value *argv);
Value native_math_ceil(int argc, Value *argv);
Value native_math_sqrt(int argc, Value *argv);
Value native_math_pow(int argc, Value *argv);
Value native_math_sin(int argc, Value *argv);
Value native_math_cos(int argc, Value *argv);
Value native_math_tan(int argc, Value *argv);
Value native_math_pi(int argc, Value *argv);
Value native_math_e(int argc, Value *argv);
Value native_math_random(int argc, Value *argv);

#endif
