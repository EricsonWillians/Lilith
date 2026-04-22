#ifndef LILITH_STDSTRING_H
#define LILITH_STDSTRING_H

#include "runtime/value.h"

Value native_str_len(int argc, Value *argv);
Value native_str_trim(int argc, Value *argv);
Value native_str_contains(int argc, Value *argv);
Value native_str_starts_with(int argc, Value *argv);
Value native_str_ends_with(int argc, Value *argv);
Value native_str_replace(int argc, Value *argv);
Value native_str_substring(int argc, Value *argv);
Value native_str_split(int argc, Value *argv);
Value native_str_join(int argc, Value *argv);

#endif
