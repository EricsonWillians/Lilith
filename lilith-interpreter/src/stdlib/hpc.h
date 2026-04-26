#ifndef LILITH_STDHPC_H
#define LILITH_STDHPC_H

#include "runtime/value.h"

/* HPC runtime stubs — these are vaporware placeholders that print
   an informative message and return nil.  They allow the HPC example
   programs to run to completion without crashing.  */

Value native_compute_parallel(int argc, Value *argv);
Value native_compute(int argc, Value *argv);
Value native_compute_async(int argc, Value *argv);
Value native_store(int argc, Value *argv);
Value native_thread_id(int argc, Value *argv);
Value native_allocate_tensor(int argc, Value *argv);
Value native_fill_tensor(int argc, Value *argv);
Value native_process_window(int argc, Value *argv);
Value native_emit(int argc, Value *argv);
Value native_allocate_buffer(int argc, Value *argv);
Value native_zero_buffer(int argc, Value *argv);

#endif
