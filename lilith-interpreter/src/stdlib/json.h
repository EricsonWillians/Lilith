#ifndef LILITH_STDJSON_H
#define LILITH_STDJSON_H

#include "runtime/value.h"

Value native_json_encode(int argc, Value *argv);
Value native_json_decode(int argc, Value *argv);

#endif
