#ifndef LILITH_STDIO_H
#define LILITH_STDIO_H

#include "runtime/value.h"

/* -------------------------------------------------------------------------- */
/* Native I/O and networking functions                                       */
/* -------------------------------------------------------------------------- */

Value native_http_get(int argc, Value *argv);
Value native_file_read(int argc, Value *argv);
Value native_file_write(int argc, Value *argv);
Value native_file_open(int argc, Value *argv);
Value native_file_close(int argc, Value *argv);
Value native_exit(int argc, Value *argv);

#endif
