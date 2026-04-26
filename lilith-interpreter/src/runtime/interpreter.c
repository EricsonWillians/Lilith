#define _GNU_SOURCE
#include "interpreter.h"
#include "gc.h"
#include "stdlib/io.h"
#include "stdlib/math.h"
#include "stdlib/string.h"
#include "stdlib/list.h"
#include "stdlib/json.h"
#include "stdlib/os.h"
#include "stdlib/meta.h"
#include "stdlib/seq.h"
#include "stdlib/time.h"
#include "stdlib/num.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>



/* ========================================================================= */
/* Error handling                                                            */
/* ========================================================================= */

void runtime_error(Interpreter *interp, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char buf[1024];
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    if (interp->error_msg) free(interp->error_msg);
    interp->error_msg = strdup(buf);
    interp->throw_flag = 1;
}

static void runtime_error_node(Interpreter *interp, AstNode *node, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char msg[512];
    vsnprintf(msg, sizeof(msg), fmt, args);
    va_end(args);
    runtime_error(interp, "[line %zu] %s", node->line, msg);
}

/* ========================================================================= */
/* Helpers                                                                   */
/* ========================================================================= */

static int is_truthy(Value value) {
    if (IS_NIL(value)) return 0;
    if (IS_BOOL(value)) return AS_BOOL(value);
    return 1;
}

static int check_type(Value value, const char *type_name) {
    if (!type_name) return 1;
    if (strcmp(type_name, "any") == 0) return 1;
    return strcmp(value_type_name(value), type_name) == 0;
}

static Value value_str_concat(Value a, Value b) {
    const char *as = value_to_string(a);
    const char *bs = value_to_string(b);
    size_t alen = strlen(as);
    size_t blen = strlen(bs);
    char *buf = (char *)malloc(alen + blen + 1);
    memcpy(buf, as, alen);
    memcpy(buf + alen, bs, blen);
    buf[alen + blen] = '\0';
    return OBJ_VAL(obj_string_take(buf, alen + blen));
}

static Value native_input(int argc, Value *argv) {
    if (argc >= 1) {
        value_print(argv[0]);
        fflush(stdout);
    }
    char *line = NULL;
    size_t len = 0;
    if (getline(&line, &len, stdin) == -1) {
        return NIL_VAL;
    }
    size_t n = strlen(line);
    if (n > 0 && line[n - 1] == '\n') line[n - 1] = '\0';
    return OBJ_VAL(obj_string_take(line, strlen(line)));
}

static Value native_print(int argc, Value *argv) {
    for (int i = 0; i < argc; i++) {
        if (i > 0) printf(" ");
        value_print(argv[i]);
    }
    printf("\n");
    return NIL_VAL;
}

static void define_native(Interpreter *interp, const char *name, NativeFn fn) {
    env_define(interp->globals, name, OBJ_VAL(obj_native_new(fn, name)));
}

/* ========================================================================= */
/* Interpreter lifecycle                                                     */
/* ========================================================================= */

void interpreter_init(Interpreter *interp) {
    interp->globals = env_new();
    interp->env = interp->globals;
    interp->return_flag = 0;
    interp->break_flag = 0;
    interp->continue_flag = 0;
    interp->throw_flag = 0;
    interp->error_msg = NULL;
    interp->current_function = NULL;

    /* Sacred core: print & input */
    define_native(interp, "@!", native_print);
    define_native(interp, "print", native_print);
    define_native(interp, "input", native_input);

    /* Namespaced modules */
    define_native(interp, "http..get", native_http_get);
    define_native(interp, "io..read", native_file_read);
    define_native(interp, "io..write", native_file_write);
    define_native(interp, "io..open", native_file_open);
    define_native(interp, "io..close", native_file_close);
    define_native(interp, "sys..exit", native_exit);

    /* Math */
    define_native(interp, "math..abs", native_math_abs);
    define_native(interp, "math..floor", native_math_floor);
    define_native(interp, "math..ceil", native_math_ceil);
    define_native(interp, "math..sqrt", native_math_sqrt);
    define_native(interp, "math..pow", native_math_pow);
    define_native(interp, "math..sin", native_math_sin);
    define_native(interp, "math..cos", native_math_cos);
    define_native(interp, "math..tan", native_math_tan);
    define_native(interp, "math..pi", native_math_pi);
    define_native(interp, "math..e", native_math_e);
    define_native(interp, "math..rand", native_math_rand);

    /* String */
    define_native(interp, "str..from", native_str_from);
    define_native(interp, "str..trim", native_str_trim);
    define_native(interp, "str..contains", native_str_contains);
    define_native(interp, "str..starts", native_str_starts);
    define_native(interp, "str..ends", native_str_ends);
    define_native(interp, "str..replace", native_str_replace);
    define_native(interp, "str..slice", native_str_slice);
    define_native(interp, "str..split", native_str_split);
    define_native(interp, "str..join", native_str_join);

    /* List */
    define_native(interp, "list..push", native_list_push);
    define_native(interp, "list..pop", native_list_pop);
    define_native(interp, "list..find", native_list_find);
    define_native(interp, "list..sort", native_list_sort);

    /* JSON */
    define_native(interp, "json..encode", native_json_encode);
    define_native(interp, "json..decode", native_json_decode);

    /* OS & Env */
    define_native(interp, "env..get", native_env_get);
    define_native(interp, "env..set", native_env_set);
    define_native(interp, "os..time", native_os_time);
    define_native(interp, "os..sleep", native_os_sleep);

    /* Meta */
    define_native(interp, "meta..type", native_meta_type);

    /* Seq */
    define_native(interp, "seq..len", native_seq_len);

    /* Time */
    define_native(interp, "time..clock", native_time_clock);

    /* Num */
    define_native(interp, "num..from", native_num_from);

    /* Convenience aliases (shorthand for common namespaced functions) */
    define_native(interp, "len", native_seq_len);
    define_native(interp, "type", native_meta_type);
    define_native(interp, "str", native_str_from);
    define_native(interp, "num", native_num_from);
    define_native(interp, "clock", native_time_clock);
}

void interpreter_free(Interpreter *interp) {
    if (interp->error_msg) free(interp->error_msg);
    env_free(interp->globals);
}

Value interpreter_run(Interpreter *interp, AstNode *program) {
    interp->return_flag = 0;
    interp->break_flag = 0;
    interp->continue_flag = 0;
    interp->throw_flag = 0;
    if (interp->error_msg) { free(interp->error_msg); interp->error_msg = NULL; }

    if (!program || program->type != AST_PROGRAM) {
        runtime_error(interp, "Expected AST_PROGRAM node.");
        return NIL_VAL;
    }
    return eval_stmt(interp, program->as.program.body);
}

/* ========================================================================= */
/* Forward declarations for static helpers                                   */
/* ========================================================================= */

static void eval_comprehension(Interpreter *interp, AstNode *comp, ObjList *result, size_t clause_idx);
static void eval_dict_comprehension(Interpreter *interp, AstNode *comp, ObjDict *result, size_t clause_idx);
static int match_pattern(Interpreter *interp, AstNode *pattern, Value value);

/* ========================================================================= */
/* Expression Evaluation                                                     */
/* ========================================================================= */

Value eval_expr(Interpreter *interp, AstNode *node) {
    if (!node) return NIL_VAL;

    switch (node->type) {
        case AST_NUMBER:    return NUMBER_VAL(node->as.number.value);
        case AST_STRING:    return OBJ_VAL(obj_string_copy(node->as.string.value, strlen(node->as.string.value)));
        case AST_BOOL:      return BOOL_VAL(node->as.boolean.value);
        case AST_NIL:       return NIL_VAL;

        case AST_IDENTIFIER: {
            Value val;
            if (env_get(interp->env, node->as.identifier.name, &val)) return val;
            runtime_error_node(interp, node, "Undefined variable '%s'.", node->as.identifier.name);
            return NIL_VAL;
        }

        case AST_BINARY: {
            Value left = eval_expr(interp, node->as.binary.left);
            if (interp->throw_flag) return NIL_VAL;
            Value right = eval_expr(interp, node->as.binary.right);
            if (interp->throw_flag) return NIL_VAL;

            switch (node->as.binary.op) {
                case OP_ADD: {
                    if (IS_NUMBER(left) && IS_NUMBER(right))
                        return NUMBER_VAL(AS_NUMBER(left) + AS_NUMBER(right));
                    return value_str_concat(left, right);
                }
                case OP_SUB: {
                    if (!IS_NUMBER(left) || !IS_NUMBER(right))
                        { runtime_error_node(interp, node, "Operands must be numbers for '--'."); return NIL_VAL; }
                    return NUMBER_VAL(AS_NUMBER(left) - AS_NUMBER(right));
                }
                case OP_MUL: {
                    if (!IS_NUMBER(left) || !IS_NUMBER(right))
                        { runtime_error_node(interp, node, "Operands must be numbers for '**'."); return NIL_VAL; }
                    return NUMBER_VAL(AS_NUMBER(left) * AS_NUMBER(right));
                }
                case OP_DIV: {
                    if (!IS_NUMBER(left) || !IS_NUMBER(right))
                        { runtime_error_node(interp, node, "Operands must be numbers for '//'."); return NIL_VAL; }
                    if (AS_NUMBER(right) == 0)
                        { runtime_error_node(interp, node, "Division by zero."); return NIL_VAL; }
                    return NUMBER_VAL(AS_NUMBER(left) / AS_NUMBER(right));
                }
                case OP_MOD: {
                    if (!IS_NUMBER(left) || !IS_NUMBER(right))
                        { runtime_error_node(interp, node, "Operands must be numbers for '%%'."); return NIL_VAL; }
                    if (AS_NUMBER(right) == 0)
                        { runtime_error_node(interp, node, "Modulo by zero."); return NIL_VAL; }
                    return NUMBER_VAL((double)((long)AS_NUMBER(left) % (long)AS_NUMBER(right)));
                }
                case OP_EQ:  return BOOL_VAL(values_equal(left, right));
                case OP_NE:  return BOOL_VAL(!values_equal(left, right));
                case OP_LT: {
                    if (!IS_NUMBER(left) || !IS_NUMBER(right))
                        { runtime_error_node(interp, node, "Operands must be numbers for '<<'."); return NIL_VAL; }
                    return BOOL_VAL(AS_NUMBER(left) < AS_NUMBER(right));
                }
                case OP_GT: {
                    if (!IS_NUMBER(left) || !IS_NUMBER(right))
                        { runtime_error_node(interp, node, "Operands must be numbers for '>>'."); return NIL_VAL; }
                    return BOOL_VAL(AS_NUMBER(left) > AS_NUMBER(right));
                }
            }
            return NIL_VAL;
        }

        case AST_UNARY: {
            Value operand = eval_expr(interp, node->as.unary.operand);
            if (interp->throw_flag) return NIL_VAL;
            if (node->as.unary.op == OP_NEG) {
                if (!IS_NUMBER(operand))
                    { runtime_error_node(interp, node, "Operand must be a number for ':-:'"); return NIL_VAL; }
                return NUMBER_VAL(-AS_NUMBER(operand));
            }
            return NIL_VAL;
        }

        case AST_CALL: {
            /* Special-case method calls: obj.method(args) */
            if (node->as.call.callee->type == AST_MEMBER) {
                AstNode *member = node->as.call.callee;
                Value obj = eval_expr(interp, member->as.member.object);
                if (interp->throw_flag) return NIL_VAL;
                const char *method_name = member->as.member.name;

                /* Evaluate arguments */
                Value *args = NULL;
                size_t arg_count = node->as.call.arg_count;
                if (arg_count > 0) {
                    args = (Value *)malloc(sizeof(Value) * (arg_count + 1));
                    for (size_t i = 0; i < arg_count; i++) {
                        args[i + 1] = eval_expr(interp, node->as.call.args[i]);
                        if (interp->throw_flag) { free(args); return NIL_VAL; }
                    }
                } else {
                    args = (Value *)malloc(sizeof(Value));
                }
                args[0] = obj;

                /* Resolve method */
                ObjFunction *method = NULL;
                if (IS_INSTANCE(obj)) {
                    ObjInstance *inst = AS_INSTANCE(obj);
                    Value val;
                    if (dict_get(inst->fields, obj_string_copy(method_name, strlen(method_name)), &val)) {
                        if (IS_FUNCTION(val)) method = AS_FUNCTION(val);
                    }
                    if (!method && dict_get(inst->klass->methods, obj_string_copy(method_name, strlen(method_name)), &val)) {
                        if (IS_FUNCTION(val)) method = AS_FUNCTION(val);
                    }
                } else if (IS_CLASS(obj)) {
                    Value val;
                    if (dict_get(AS_CLASS(obj)->methods, obj_string_copy(method_name, strlen(method_name)), &val)) {
                        if (IS_FUNCTION(val)) method = AS_FUNCTION(val);
                    }
                } else if (IS_LIST(obj) || IS_DICT(obj) || IS_STRING(obj) || IS_TUPLE(obj)) {
                    /* Native method dispatch */
                    if (strcmp(method_name, "length") == 0) {
                        free(args);
                        return native_seq_len(1, &obj);
                    }
                }

                if (!method) {
                    runtime_error_node(interp, node, "Undefined method '%s'.", method_name);
                    free(args);
                    return NIL_VAL;
                }

                Environment *call_env = env_new_enclosing(method->closure ? method->closure : interp->globals);
                for (size_t i = 0; i < method->param_count && i < (arg_count + 1); i++) {
                    env_define(call_env, method->params[i], args[i]);
                }
                free(args);

                Environment *prev = interp->env;
                interp->env = call_env;
                interp->return_flag = 0;
                Value block_result = eval_stmt(interp, method->body);
                Value result = NIL_VAL;
                if (interp->return_flag) {
                    env_get(call_env, "__return__", &result);
                } else if (method->implicit_return) {
                    result = block_result;
                }
                interp->return_flag = 0;
                env_free(call_env);
                interp->env = prev;
                return result;
            }

            Value callee = eval_expr(interp, node->as.call.callee);
            if (interp->throw_flag) return NIL_VAL;

            /* Evaluate arguments */
            size_t arg_count = node->as.call.arg_count;
            Value *args = arg_count > 0 ? (Value *)malloc(sizeof(Value) * arg_count) : NULL;
            for (size_t i = 0; i < arg_count; i++) {
                args[i] = eval_expr(interp, node->as.call.args[i]);
                if (interp->throw_flag) { free(args); return NIL_VAL; }
            }

            if (IS_NATIVE(callee)) {
                Value result = AS_NATIVE(callee)->fn((int)arg_count, args);
                free(args);
                return result;
            }

            if (IS_FUNCTION(callee)) {
                ObjFunction *fn = AS_FUNCTION(callee);

                /* Type-check arguments before binding */
                for (size_t i = 0; i < fn->param_count && i < arg_count; i++) {
                    if (fn->param_types && fn->param_types[i] && !check_type(args[i], fn->param_types[i])) {
                        runtime_error(interp, "Expected argument %zu to be %s, got %s",
                                      i + 1, fn->param_types[i], value_type_name(args[i]));
                        free(args);
                        return NIL_VAL;
                    }
                }

                Environment *call_env = env_new_enclosing(fn->closure ? fn->closure : interp->globals);
                for (size_t i = 0; i < fn->param_count && i < arg_count; i++) {
                    env_define(call_env, fn->params[i], args[i]);
                }
                free(args);

                Environment *prev = interp->env;
                ObjFunction *prev_fn = interp->current_function;
                interp->env = call_env;
                interp->current_function = fn;
                interp->return_flag = 0;
                Value block_result = eval_stmt(interp, fn->body);
                Value result = NIL_VAL;
                if (interp->return_flag) {
                    env_get(call_env, "__return__", &result);
                } else if (fn->implicit_return) {
                    result = block_result;
                }
                interp->return_flag = 0;
                interp->current_function = prev_fn;
                env_free(call_env);
                interp->env = prev;
                return result;
            }

            if (IS_CLASS(callee)) {
                ObjClass *klass = AS_CLASS(callee);
                ObjInstance *inst = obj_instance_new(klass);
                /* Call init if present */
                Value init_val;
                if (dict_get(klass->methods, obj_string_copy("init", 4), &init_val) && IS_FUNCTION(init_val)) {
                    ObjFunction *init = AS_FUNCTION(init_val);
                    Environment *call_env = env_new_enclosing(interp->globals);
                    env_define(call_env, "self", OBJ_VAL(inst));
                    for (size_t i = 1; i < init->param_count && (i - 1) < arg_count; i++) {
                        env_define(call_env, init->params[i], args[i - 1]);
                    }
                    free(args);
                    Environment *prev = interp->env;
                    interp->env = call_env;
                    interp->return_flag = 0;
                    eval_stmt(interp, init->body);
                    interp->return_flag = 0;
                    env_free(call_env);
                    interp->env = prev;
                } else {
                    free(args);
                }
                return OBJ_VAL(inst);
            }

            runtime_error_node(interp, node, "Can only call functions and classes.");
            free(args);
            return NIL_VAL;
        }

        case AST_MEMBER: {
            Value obj = eval_expr(interp, node->as.member.object);
            if (interp->throw_flag) return NIL_VAL;
            const char *name = node->as.member.name;

            if (IS_INSTANCE(obj)) {
                ObjInstance *inst = AS_INSTANCE(obj);
                Value val;
                if (dict_get(inst->fields, obj_string_copy(name, strlen(name)), &val)) return val;
                if (dict_get(inst->klass->methods, obj_string_copy(name, strlen(name)), &val)) return val;
                runtime_error_node(interp, node, "Undefined property '%s'.", name);
                return NIL_VAL;
            }

            if (IS_DICT(obj)) {
                Value val;
                if (dict_get(AS_DICT(obj), obj_string_copy(name, strlen(name)), &val)) return val;
                return NIL_VAL;
            }

            if (IS_LIST(obj) || IS_STRING(obj) || IS_TUPLE(obj)) {
                if (strcmp(name, "length") == 0) {
                    return native_seq_len(1, &obj);
                }
            }

            runtime_error_node(interp, node, "Only instances and dicts have properties.");
            return NIL_VAL;
        }

        case AST_INDEX: {
            Value obj = eval_expr(interp, node->as.index.object);
            if (interp->throw_flag) return NIL_VAL;
            Value idx = eval_expr(interp, node->as.index.index);
            if (interp->throw_flag) return NIL_VAL;

            if (IS_LIST(obj)) {
                if (!IS_NUMBER(idx)) { runtime_error_node(interp, node, "List index must be a number."); return NIL_VAL; }
                ObjList *list = AS_LIST(obj);
                long i = (long)AS_NUMBER(idx);
                if (i < 0 || (size_t)i >= list->count) { runtime_error_node(interp, node, "List index out of bounds."); return NIL_VAL; }
                return list->items[i];
            }
            if (IS_TUPLE(obj)) {
                if (!IS_NUMBER(idx)) { runtime_error_node(interp, node, "Tuple index must be a number."); return NIL_VAL; }
                ObjTuple *tuple = AS_TUPLE(obj);
                long i = (long)AS_NUMBER(idx);
                if (i < 0 || (size_t)i >= tuple->count) { runtime_error_node(interp, node, "Tuple index out of bounds."); return NIL_VAL; }
                return tuple->items[i];
            }
            if (IS_STRING(obj)) {
                if (!IS_NUMBER(idx)) { runtime_error_node(interp, node, "String index must be a number."); return NIL_VAL; }
                ObjString *str = AS_STRING(obj);
                long i = (long)AS_NUMBER(idx);
                if (i < 0 || (size_t)i >= str->length) { runtime_error_node(interp, node, "String index out of bounds."); return NIL_VAL; }
                char *buf = (char *)malloc(2);
                buf[0] = str->chars[i];
                buf[1] = '\0';
                return OBJ_VAL(obj_string_take(buf, 1));
            }
            if (IS_DICT(obj)) {
                ObjDict *dict = AS_DICT(obj);
                if (!IS_STRING(idx)) { runtime_error_node(interp, node, "Dict key must be a string."); return NIL_VAL; }
                Value val;
                if (dict_get(dict, AS_STRING(idx), &val)) return val;
                return NIL_VAL;
            }
            runtime_error_node(interp, node, "Only lists, tuples, strings, and dicts are indexable.");
            return NIL_VAL;
        }

        case AST_CONDITIONAL: {
            Value cond = eval_expr(interp, node->as.conditional.cond);
            if (interp->throw_flag) return NIL_VAL;
            return eval_expr(interp, is_truthy(cond) ? node->as.conditional.then_branch : node->as.conditional.else_branch);
        }

        case AST_AWAIT: {
            return eval_expr(interp, node->as.await_expr.value);
        }

        case AST_LAMBDA: {
            ObjFunction *fn = obj_function_new(NULL);
            fn->param_count = node->as.lambda.param_count;
            fn->params = (char **)malloc(sizeof(char *) * fn->param_count);
            for (size_t i = 0; i < fn->param_count; i++) {
                fn->params[i] = strdup(node->as.lambda.params[i]);
            }
            fn->body = node->as.lambda.body;
            fn->implicit_return = 1;
            fn->closure = interp->env;
            return OBJ_VAL(fn);
        }

        case AST_LIST: {
            ObjList *list = obj_list_new();
            for (size_t i = 0; i < node->as.list.count; i++) {
                Value elem = eval_expr(interp, node->as.list.elements[i]);
                if (interp->throw_flag) return NIL_VAL;
                value_array_write(list, elem);
            }
            return OBJ_VAL(list);
        }

        case AST_TUPLE: {
            ObjTuple *tuple = obj_tuple_new(node->as.tuple.count);
            for (size_t i = 0; i < node->as.tuple.count; i++) {
                Value elem = eval_expr(interp, node->as.tuple.elements[i]);
                if (interp->throw_flag) return NIL_VAL;
                tuple->items[i] = elem;
            }
            return OBJ_VAL(tuple);
        }

        case AST_DICT: {
            ObjDict *dict = obj_dict_new();
            for (size_t i = 0; i < node->as.dict.count; i++) {
                AstNode *entry = node->as.dict.entries[i];
                Value key = eval_expr(interp, entry->as.dict_entry.key);
                if (interp->throw_flag) return NIL_VAL;
                Value val = eval_expr(interp, entry->as.dict_entry.value);
                if (interp->throw_flag) return NIL_VAL;
                if (!IS_STRING(key)) { runtime_error_node(interp, node, "Dict keys must be strings."); return NIL_VAL; }
                dict_set(dict, AS_STRING(key), val);
            }
            return OBJ_VAL(dict);
        }

        case AST_COMPREHENSION: {
            /* Build a result list by iterating over for-clauses and filtering with if-clauses */
            ObjList *result = obj_list_new();
            eval_comprehension(interp, node, result, 0);
            if (node->as.comprehension.container == 1) {
                /* Tuple comprehension */
                ObjTuple *tuple = obj_tuple_new(result->count);
                for (size_t i = 0; i < result->count; i++) tuple->items[i] = result->items[i];
                return OBJ_VAL(tuple);
            } else if (node->as.comprehension.container == 2) {
                /* Set comprehension - for now deduplicate via dict keys */
                ObjDict *set = obj_dict_new();
                for (size_t i = 0; i < result->count; i++) {
                    const char *s = value_to_string(result->items[i]);
                    dict_set(set, obj_string_copy(s, strlen(s)), result->items[i]);
                }
                ObjList *list = obj_list_new();
                for (size_t i = 0; i < set->count; i++) {
                    value_array_write(list, set->entries[i].value);
                }
                return OBJ_VAL(list);
            }
            return OBJ_VAL(result);
        }

        case AST_DICT_COMPREHENSION: {
            ObjDict *result = obj_dict_new();
            eval_dict_comprehension(interp, node, result, 0);
            return OBJ_VAL(result);
        }

        default:
            runtime_error_node(interp, node, "Unsupported expression node type.");
            return NIL_VAL;
    }
}

/* ========================================================================= */
/* Statement Evaluation                                                      */
/* ========================================================================= */

Value eval_stmt(Interpreter *interp, AstNode *node) {
    if (!node) return NIL_VAL;

    switch (node->type) {
        case AST_PROGRAM:
            return eval_stmt(interp, node->as.program.body);

        case AST_BLOCK: {
            Value last = NIL_VAL;
            for (size_t i = 0; i < node->as.block.count; i++) {
                last = eval_stmt(interp, node->as.block.stmts[i]);
                if (interp->return_flag || interp->break_flag || interp->continue_flag || interp->throw_flag)
                    return NIL_VAL;
            }
            return last;
        }

        case AST_EXPRESSION_STMT:
            return eval_expr(interp, node->as.expression_stmt.expr);

        case AST_ASSIGN: {
            AstNode *target = node->as.assign.target;
            Value value = eval_expr(interp, node->as.assign.value);
            if (interp->throw_flag) return NIL_VAL;

            if (target->type == AST_IDENTIFIER) {
                if (!env_set(interp->env, target->as.identifier.name, value))
                    env_define(interp->env, target->as.identifier.name, value);
                return value;
            }

            if (target->type == AST_MEMBER) {
                Value obj = eval_expr(interp, target->as.member.object);
                if (interp->throw_flag) return NIL_VAL;
                const char *name = target->as.member.name;
                if (IS_INSTANCE(obj)) {
                    dict_set(AS_INSTANCE(obj)->fields, obj_string_copy(name, strlen(name)), value);
                } else if (IS_DICT(obj)) {
                    dict_set(AS_DICT(obj), obj_string_copy(name, strlen(name)), value);
                } else {
                    runtime_error_node(interp, node, "Can only assign to instance or dict properties.");
                }
                return value;
            }

            if (target->type == AST_INDEX) {
                Value obj = eval_expr(interp, target->as.index.object);
                if (interp->throw_flag) return NIL_VAL;
                Value idx = eval_expr(interp, target->as.index.index);
                if (interp->throw_flag) return NIL_VAL;
                if (IS_LIST(obj)) {
                    if (!IS_NUMBER(idx)) { runtime_error_node(interp, node, "List index must be a number."); return NIL_VAL; }
                    ObjList *list = AS_LIST(obj);
                    long i = (long)AS_NUMBER(idx);
                    if (i < 0 || (size_t)i >= list->count) { runtime_error_node(interp, node, "List index out of bounds."); return NIL_VAL; }
                    list->items[i] = value;
                } else if (IS_DICT(obj)) {
                    if (!IS_STRING(idx)) { runtime_error_node(interp, node, "Dict key must be a string."); return NIL_VAL; }
                    dict_set(AS_DICT(obj), AS_STRING(idx), value);
                } else {
                    runtime_error_node(interp, node, "Can only index-assign to lists and dicts.");
                }
                return value;
            }

            if (target->type == AST_TUPLE) {
                /* Tuple destructuring: (<a,, b,, c>) [=] value */
                Value src = value;
                size_t count = 0;
                Value *items = NULL;
                if (IS_TUPLE(src)) { count = AS_TUPLE(src)->count; items = AS_TUPLE(src)->items; }
                else if (IS_LIST(src)) { count = AS_LIST(src)->count; items = AS_LIST(src)->items; }
                else { runtime_error_node(interp, node, "Can only destructure from list or tuple."); return NIL_VAL; }

                if (count != target->as.tuple.count) {
                    runtime_error_node(interp, node, "Destructuring mismatch: %zu variables, %zu values.", target->as.tuple.count, count);
                    return NIL_VAL;
                }
                for (size_t i = 0; i < count; i++) {
                    AstNode *var = target->as.tuple.elements[i];
                    if (var->type != AST_IDENTIFIER) {
                        runtime_error_node(interp, node, "Destructuring target must be an identifier.");
                        return NIL_VAL;
                    }
                    if (!env_set(interp->env, var->as.identifier.name, items[i]))
                        env_define(interp->env, var->as.identifier.name, items[i]);
                }
                return value;
            }

            runtime_error_node(interp, node, "Invalid assignment target.");
            return NIL_VAL;
        }

        case AST_IF: {
            Value cond = eval_expr(interp, node->as.if_stmt.cond);
            if (interp->throw_flag) return NIL_VAL;
            if (is_truthy(cond)) {
                eval_stmt(interp, node->as.if_stmt.then_branch);
            } else if (node->as.if_stmt.else_branch) {
                eval_stmt(interp, node->as.if_stmt.else_branch);
            }
            return NIL_VAL;
        }

        case AST_WHILE: {
            for (;;) {
                Value cond = eval_expr(interp, node->as.while_stmt.cond);
                if (interp->throw_flag) return NIL_VAL;
                if (!is_truthy(cond)) break;
                eval_stmt(interp, node->as.while_stmt.body);
                if (interp->return_flag || interp->throw_flag) return NIL_VAL;
                if (interp->break_flag) { interp->break_flag = 0; break; }
                if (interp->continue_flag) { interp->continue_flag = 0; }
            }
            return NIL_VAL;
        }

        case AST_FOR: {
            Value iterable = eval_expr(interp, node->as.for_stmt.iter);
            if (interp->throw_flag) return NIL_VAL;
            ObjList *list = NULL;
            ObjTuple *tuple = NULL;
            ObjString *str = NULL;
            size_t count = 0;
            Value *items = NULL;

            if (IS_LIST(iterable)) { list = AS_LIST(iterable); count = list->count; items = list->items; }
            else if (IS_TUPLE(iterable)) { tuple = AS_TUPLE(iterable); count = tuple->count; items = tuple->items; }
            else if (IS_STRING(iterable)) { str = AS_STRING(iterable); count = str->length; }
            else { runtime_error_node(interp, node, "Can only iterate over lists, tuples, and strings."); return NIL_VAL; }

            for (size_t i = 0; i < count; i++) {
                Value item = str ? OBJ_VAL(obj_string_copy(&str->chars[i], 1)) : items[i];
                env_define(interp->env, node->as.for_stmt.var, item);
                eval_stmt(interp, node->as.for_stmt.body);
                if (interp->return_flag || interp->throw_flag) return NIL_VAL;
                if (interp->break_flag) { interp->break_flag = 0; break; }
                if (interp->continue_flag) { interp->continue_flag = 0; }
            }
            return NIL_VAL;
        }

        case AST_RETURN: {
            interp->return_flag = 1;
            /* Store return value in the current environment so caller can retrieve it */
            Value val = eval_expr(interp, node->as.return_stmt.value);
            if (interp->throw_flag) return NIL_VAL;
            /* Check return type if annotated */
            if (interp->current_function && interp->current_function->return_type &&
                !check_type(val, interp->current_function->return_type)) {
                runtime_error(interp, "Expected return type %s, got %s",
                              interp->current_function->return_type, value_type_name(val));
                return NIL_VAL;
            }
            env_define(interp->env, "__return__", val);
            return val;
        }

        case AST_YIELD: {
            interp->return_flag = 1;
            Value val = eval_expr(interp, node->as.yield_stmt.value);
            if (interp->throw_flag) return NIL_VAL;
            env_define(interp->env, "__return__", val);
            return val;
        }

        case AST_BREAK:
            interp->break_flag = 1;
            return NIL_VAL;

        case AST_CONTINUE:
            interp->continue_flag = 1;
            return NIL_VAL;

        case AST_FUNCTION: {
            ObjFunction *fn = obj_function_new(node->as.function.name);
            fn->param_count = node->as.function.param_count;
            fn->params = (char **)malloc(sizeof(char *) * fn->param_count);
            fn->param_types = (char **)calloc(fn->param_count, sizeof(char *));
            for (size_t i = 0; i < fn->param_count; i++) {
                fn->params[i] = strdup(node->as.function.params[i]);
                if (node->as.function.param_types && node->as.function.param_types[i])
                    fn->param_types[i] = strdup(node->as.function.param_types[i]);
            }
            fn->body = node->as.function.body;
            fn->return_type = node->as.function.return_type ? strdup(node->as.function.return_type) : NULL;
            fn->is_async = node->as.function.is_async;
            fn->closure = interp->env;
            env_define(interp->env, node->as.function.name, OBJ_VAL(fn));
            return OBJ_VAL(fn);
        }

        case AST_CLASS: {
            ObjClass *klass = obj_class_new(node->as.class_def.name);
            for (size_t i = 0; i < node->as.class_def.method_count; i++) {
                AstNode *method = node->as.class_def.methods[i];
                if (method->type == AST_FUNCTION) {
                    ObjFunction *fn = obj_function_new(method->as.function.name);
                    fn->param_count = method->as.function.param_count;
                    fn->params = (char **)malloc(sizeof(char *) * fn->param_count);
                    fn->param_types = (char **)calloc(fn->param_count, sizeof(char *));
                    for (size_t j = 0; j < fn->param_count; j++) {
                        fn->params[j] = strdup(method->as.function.params[j]);
                        if (method->as.function.param_types && method->as.function.param_types[j])
                            fn->param_types[j] = strdup(method->as.function.param_types[j]);
                    }
                    fn->body = method->as.function.body;
                    fn->return_type = method->as.function.return_type ? strdup(method->as.function.return_type) : NULL;
                    fn->is_async = method->as.function.is_async;
                    fn->closure = interp->env;
                    dict_set(klass->methods, obj_string_copy(fn->name, strlen(fn->name)), OBJ_VAL(fn));
                }
            }
            env_define(interp->env, node->as.class_def.name, OBJ_VAL(klass));
            return OBJ_VAL(klass);
        }

        case AST_TRY: {
            eval_stmt(interp, node->as.try_stmt.try_body);
            if (interp->throw_flag) {
                char *err = interp->error_msg ? strdup(interp->error_msg) : strdup("unknown error");
                interp->throw_flag = 0;
                if (interp->error_msg) { free(interp->error_msg); interp->error_msg = NULL; }

                if (node->as.try_stmt.catch_body) {
                    Environment *catch_env = env_new_enclosing(interp->env);
                    if (node->as.try_stmt.catch_var) {
                        env_define(catch_env, node->as.try_stmt.catch_var, OBJ_VAL(obj_string_copy(err, strlen(err))));
                    }
                    Environment *prev = interp->env;
                    interp->env = catch_env;
                    eval_stmt(interp, node->as.try_stmt.catch_body);
                    interp->env = prev;
                    env_free(catch_env);
                }
                free(err);
            }
            int had_error = interp->throw_flag;
            interp->throw_flag = 0;

            if (node->as.try_stmt.finally_body) {
                eval_stmt(interp, node->as.try_stmt.finally_body);
            }

            if (interp->return_flag || interp->break_flag || interp->continue_flag)
                return NIL_VAL;

            /* If there was an error before finally and it wasn't handled in finally, rethrow */
            if (had_error) {
                interp->throw_flag = 1;
            }
            return NIL_VAL;
        }

        case AST_MATCH: {
            Value val = eval_expr(interp, node->as.match_stmt.expr);
            if (interp->throw_flag) return NIL_VAL;
            for (size_t i = 0; i < node->as.match_stmt.case_count; i++) {
                AstNode *case_node = node->as.match_stmt.cases[i]; /* AST_IF node: cond=pattern, then_branch=body */
                if (match_pattern(interp, case_node->as.if_stmt.cond, val)) {
                    eval_stmt(interp, case_node->as.if_stmt.then_branch);
                    return NIL_VAL;
                }
            }
            return NIL_VAL;
        }

        case AST_IMPORT:
            /* No-op: module system not yet implemented */
            return NIL_VAL;

        case AST_HPC:
            /* Execute HPC body in a new scope */
            return eval_stmt(interp, node->as.hpc.body);

        default:
            runtime_error_node(interp, node, "Unsupported statement node type.");
            return NIL_VAL;
    }
}

/* ========================================================================= */
/* Comprehension helper                                                      */
/* ========================================================================= */

static void eval_comprehension(Interpreter *interp, AstNode *comp, ObjList *result, size_t clause_idx) {
    if (clause_idx >= comp->as.comprehension.clause_count) {
        Value val = eval_expr(interp, comp->as.comprehension.expr);
        if (!interp->throw_flag) value_array_write(result, val);
        return;
    }

    AstNode *clause = comp->as.comprehension.clauses[clause_idx];
    if (clause->type == AST_FOR_CLAUSE) {
        Value iterable = eval_expr(interp, clause->as.for_clause.iter);
        if (interp->throw_flag) return;

        ObjList *list = NULL;
        ObjTuple *tuple = NULL;
        ObjString *str = NULL;
        size_t count = 0;
        Value *items = NULL;

        if (IS_LIST(iterable)) { list = AS_LIST(iterable); count = list->count; items = list->items; }
        else if (IS_TUPLE(iterable)) { tuple = AS_TUPLE(iterable); count = tuple->count; items = tuple->items; }
        else if (IS_STRING(iterable)) { str = AS_STRING(iterable); count = str->length; }
        else { runtime_error(interp, "Can only iterate over lists, tuples, and strings in comprehensions."); return; }

        for (size_t i = 0; i < count; i++) {
            Value item = str ? OBJ_VAL(obj_string_copy(&str->chars[i], 1)) : items[i];
            env_define(interp->env, clause->as.for_clause.var, item);
            eval_comprehension(interp, comp, result, clause_idx + 1);
            if (interp->throw_flag) return;
        }
    } else if (clause->type == AST_IF_CLAUSE) {
        Value cond = eval_expr(interp, clause->as.if_clause.cond);
        if (interp->throw_flag) return;
        if (is_truthy(cond)) {
            eval_comprehension(interp, comp, result, clause_idx + 1);
        }
    }
}

static void eval_dict_comprehension(Interpreter *interp, AstNode *comp, ObjDict *result, size_t clause_idx) {
    if (clause_idx >= comp->as.dict_comprehension.clause_count) {
        Value key = eval_expr(interp, comp->as.dict_comprehension.key);
        if (interp->throw_flag) return;
        Value val = eval_expr(interp, comp->as.dict_comprehension.value);
        if (interp->throw_flag) return;
        if (!IS_STRING(key)) { runtime_error(interp, "Dict comprehension keys must be strings."); return; }
        dict_set(result, AS_STRING(key), val);
        return;
    }

    AstNode *clause = comp->as.dict_comprehension.clauses[clause_idx];
    if (clause->type == AST_FOR_CLAUSE) {
        Value iterable = eval_expr(interp, clause->as.for_clause.iter);
        if (interp->throw_flag) return;

        ObjList *list = NULL;
        ObjTuple *tuple = NULL;
        ObjString *str = NULL;
        size_t count = 0;
        Value *items = NULL;

        if (IS_LIST(iterable)) { list = AS_LIST(iterable); count = list->count; items = list->items; }
        else if (IS_TUPLE(iterable)) { tuple = AS_TUPLE(iterable); count = tuple->count; items = tuple->items; }
        else if (IS_STRING(iterable)) { str = AS_STRING(iterable); count = str->length; }
        else { runtime_error(interp, "Can only iterate over lists, tuples, and strings in comprehensions."); return; }

        for (size_t i = 0; i < count; i++) {
            Value item = str ? OBJ_VAL(obj_string_copy(&str->chars[i], 1)) : items[i];
            env_define(interp->env, clause->as.for_clause.var, item);
            eval_dict_comprehension(interp, comp, result, clause_idx + 1);
            if (interp->throw_flag) return;
        }
    } else if (clause->type == AST_IF_CLAUSE) {
        Value cond = eval_expr(interp, clause->as.if_clause.cond);
        if (interp->throw_flag) return;
        if (is_truthy(cond)) {
            eval_dict_comprehension(interp, comp, result, clause_idx + 1);
        }
    }
}

/* ========================================================================= */
/* Pattern matching                                                          */
/* ========================================================================= */

static int match_pattern(Interpreter *interp, AstNode *pattern, Value value) {
    switch (pattern->type) {
        case AST_NUMBER:
            return IS_NUMBER(value) && AS_NUMBER(value) == pattern->as.number.value;
        case AST_STRING:
            return IS_STRING(value) && strcmp(AS_STRING(value)->chars, pattern->as.string.value) == 0;
        case AST_BOOL:
            return IS_BOOL(value) && AS_BOOL(value) == pattern->as.boolean.value;
        case AST_NIL:
            return IS_NIL(value);
        case AST_IDENTIFIER:
            /* Binding pattern: always matches, binds the value */
            env_define(interp->env, pattern->as.identifier.name, value);
            return 1;
        case AST_TUPLE: {
            if (!IS_TUPLE(value) && !IS_LIST(value)) return 0;
            size_t count = IS_TUPLE(value) ? AS_TUPLE(value)->count : AS_LIST(value)->count;
            Value *items = IS_TUPLE(value) ? AS_TUPLE(value)->items : AS_LIST(value)->items;
            if (count != pattern->as.tuple.count) return 0;
            for (size_t i = 0; i < count; i++) {
                if (!match_pattern(interp, pattern->as.tuple.elements[i], items[i])) return 0;
            }
            return 1;
        }
        default:
            return 0;
    }
}
