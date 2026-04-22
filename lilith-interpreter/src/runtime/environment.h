#ifndef LILITH_ENVIRONMENT_H
#define LILITH_ENVIRONMENT_H

#include "value.h"

/* -------------------------------------------------------------------------- */
/* Environment — lexical variable scoping                                    */
/* -------------------------------------------------------------------------- */

typedef struct Environment {
    ObjDict *values;
    struct Environment *enclosing;
} Environment;

Environment *env_new(void);
Environment *env_new_enclosing(Environment *enclosing);
void env_free(Environment *env);

void env_define(Environment *env, const char *name, Value value);
int  env_get(Environment *env, const char *name, Value *out);
int  env_set(Environment *env, const char *name, Value value);
int  env_get_at(Environment *env, const char *name, Value *out, int depth);
int  env_set_at(Environment *env, const char *name, Value value, int depth);

#endif
