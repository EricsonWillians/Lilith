#ifndef LILITH_STDLIST_H
#define LILITH_STDLIST_H

#include "runtime/value.h"

Value native_list_push(int argc, Value *argv);
Value native_list_pop(int argc, Value *argv);
Value native_list_find(int argc, Value *argv);
Value native_list_sort(int argc, Value *argv);

#endif
