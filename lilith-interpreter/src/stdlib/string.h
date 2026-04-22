#ifndef LILITH_STDSTRING_H
#define LILITH_STDSTRING_H

#include "runtime/value.h"

Value native_str_from(int argc, Value *argv);
Value native_str_trim(int argc, Value *argv);
Value native_str_contains(int argc, Value *argv);
Value native_str_starts(int argc, Value *argv);
Value native_str_ends(int argc, Value *argv);
Value native_str_replace(int argc, Value *argv);
Value native_str_slice(int argc, Value *argv);
Value native_str_split(int argc, Value *argv);
Value native_str_join(int argc, Value *argv);

#endif
