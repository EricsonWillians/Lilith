#include "hpc.h"
#include <stdio.h>

/* All HPC natives are stubs.  They print a short message so the user
   knows the call was reached, then return a placeholder value.  This
   lets the example programs demonstrate HPC syntax without requiring
   a real parallel runtime, GPU backend, tensor library, etc.  */

static void hpc_stub_print(const char *name) {
    printf("[HPC stub: %s]\n", name);
}

Value native_compute_parallel(int argc, Value *argv) {
    hpc_stub_print("compute_parallel");
    (void)argc; (void)argv;
    return NIL_VAL;
}

Value native_compute(int argc, Value *argv) {
    hpc_stub_print("compute");
    (void)argc; (void)argv;
    return NIL_VAL;
}

Value native_compute_async(int argc, Value *argv) {
    hpc_stub_print("compute_async");
    (void)argc; (void)argv;
    return NIL_VAL;
}

Value native_store(int argc, Value *argv) {
    hpc_stub_print("store");
    (void)argc; (void)argv;
    return NIL_VAL;
}

Value native_thread_id(int argc, Value *argv) {
    hpc_stub_print("thread_id");
    (void)argc; (void)argv;
    return NUMBER_VAL(0);
}

Value native_allocate_tensor(int argc, Value *argv) {
    hpc_stub_print("allocate_tensor");
    (void)argc; (void)argv;
    return OBJ_VAL(obj_string_copy("<tensor>", 8));
}

Value native_fill_tensor(int argc, Value *argv) {
    hpc_stub_print("fill_tensor");
    (void)argc; (void)argv;
    return NIL_VAL;
}

Value native_process_window(int argc, Value *argv) {
    hpc_stub_print("process_window");
    (void)argc; (void)argv;
    return NUMBER_VAL(42);
}

Value native_emit(int argc, Value *argv) {
    hpc_stub_print("emit");
    (void)argc; (void)argv;
    return NIL_VAL;
}

Value native_allocate_buffer(int argc, Value *argv) {
    hpc_stub_print("allocate_buffer");
    (void)argc; (void)argv;
    return OBJ_VAL(obj_string_copy("<buffer>", 8));
}

Value native_zero_buffer(int argc, Value *argv) {
    hpc_stub_print("zero_buffer");
    (void)argc; (void)argv;
    return NIL_VAL;
}
