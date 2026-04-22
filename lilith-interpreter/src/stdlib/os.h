#ifndef LILITH_STDOS_H
#define LILITH_STDOS_H

#include "runtime/value.h"

Value native_env_get(int argc, Value *argv);
Value native_env_set(int argc, Value *argv);
Value native_os_time(int argc, Value *argv);
Value native_os_sleep(int argc, Value *argv);

#endif
