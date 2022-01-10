#include <libgccjit.h>
#include <string.h>
#include "ds.h"
#include "dsc.h"
#include "generated/dsc_parser.h"
#include "generated/dsc_lexer.h"

static gcc_jit_context *ctx;

typedef struct {
  char  *name;
  gcc_jit_function *code;
  gcc_jit_result *result; // remove
  uint32_t narg;
  uint32_t nblock;
  uint32_t nlabel;
} Fun;

#define FUN_SIZE 4096
static Fun fun_data[FUN_SIZE];
static uint32_t fun_count;

#define BLOCK_SIZE 128
gcc_jit_block* block_data[BLOCK_SIZE];
static uint32_t block_count;

#define LABEL_SIZE 64
uint32_t label_data[LABEL_SIZE];
//static uint32_t label_count;

#define VALUE_SIZE 2048
gcc_jit_rvalue* value_data[VALUE_SIZE];

#define PARAM_SIZE 16
gcc_jit_param *param_data[PARAM_SIZE];

static enum gcc_jit_binary_op get_binary(const ds_opcode op) {
  if(op == dsop_add || op == dsop_addf ||
     op == dsop_add_imm || op == dsop_add_immf) return GCC_JIT_BINARY_OP_PLUS;
  if(op == dsop_sub || op == dsop_subf ||
     op == dsop_sub_imm || op == dsop_sub_immf) return GCC_JIT_BINARY_OP_MINUS;
  if(op == dsop_mul || op == dsop_mulf ||
     op == dsop_mul_imm || op == dsop_mul_immf) return GCC_JIT_BINARY_OP_MULT;
  if(op == dsop_div || op == dsop_div ||
     op == dsop_div_imm || op == dsop_div_immf) return GCC_JIT_BINARY_OP_DIVIDE;
  if(op == dsop_mod || op == dsop_mod_imm) return GCC_JIT_BINARY_OP_MODULO;
  if(op == dsop_band) return GCC_JIT_BINARY_OP_BITWISE_AND;
  if(op == dsop_bor) return GCC_JIT_BINARY_OP_BITWISE_OR;
  if(op == dsop_bxor) return GCC_JIT_BINARY_OP_BITWISE_XOR;
  if(op == dsop_land) return GCC_JIT_BINARY_OP_LOGICAL_AND;
  if(op == dsop_lor) return GCC_JIT_BINARY_OP_LOGICAL_OR;
  if(op == dsop_blshift) return GCC_JIT_BINARY_OP_LSHIFT;
  if(op == dsop_brshift) return GCC_JIT_BINARY_OP_RSHIFT;
//  exit(4);
}

static inline enum gcc_jit_comparison get_comp(const ds_opcode op) {
  return GCC_JIT_COMPARISON_EQ + op - dsop_eq;
}
static void actual_emit_binary(const DsStmt *stmt, gcc_jit_rvalue *rhs) {
  if(stmt->op < dsop_eq || stmt->op > dsop_le) {
/*
gcc_jit_block_add_comment (block_data[block_count],
         NULL, //gcc_jit_location *loc,
         "here is a binary op");
*/
    value_data[stmt->dest] = gcc_jit_context_new_binary_op (ctx,
             NULL, //gcc_jit_location *loc,
             get_binary(stmt->op),
             gcc_jit_rvalue_get_type(rhs),
             value_data[stmt->num0], rhs);
  } else {
    value_data[stmt->dest] = gcc_jit_context_new_comparison (ctx,
        NULL, //gcc_jit_location *loc,
        get_comp(stmt->op),
        value_data[stmt->num0], rhs);
  }
}

static void dsjit_emit_binary(const DsStmt *stmt) {
  actual_emit_binary(stmt, value_data[stmt->num1]);
}

static void dsjit_emit_ibinary(const DsStmt *stmt) {
  gcc_jit_rvalue *tmp = gcc_jit_context_new_rvalue_from_long (ctx,
              gcc_jit_context_get_type(ctx, GCC_JIT_TYPE_LONG), stmt->num1);
  actual_emit_binary(stmt, tmp);
}

static void dsjit_emit_fbinary(const DsStmt *stmt) {
  gcc_jit_rvalue *tmp = gcc_jit_context_new_rvalue_from_double (ctx, //???
              gcc_jit_context_get_type(ctx, GCC_JIT_TYPE_FLOAT), stmt->fnum1);
  actual_emit_binary(stmt, tmp);
}

static enum gcc_jit_unary_op  get_unary(const ds_opcode op) {
//  if(op == dsop_inc) exit(3);
//  if(op == dsop_dec) exit(4);
//  if(op == dsop_mov) exit(5); // ??
  if(op == dsop_neg) return GCC_JIT_UNARY_OP_MINUS;
  if(op == dsop_not) return GCC_JIT_UNARY_OP_LOGICAL_NEGATE;
  if(op == dsop_cmp) return GCC_JIT_UNARY_OP_BITWISE_NEGATE;
  //  if(op == dsop_add)
  return GCC_JIT_UNARY_OP_ABS;
}

static void dsjit_emit_unary(const DsStmt *stmt) {
  gcc_jit_rvalue *const val = value_data[stmt->num0];
  value_data[stmt->dest] = gcc_jit_context_new_unary_op (ctx,
            NULL, //gcc_jit_location *loc,
            get_unary(stmt->op),
            gcc_jit_rvalue_get_type(val),
            val);
}

static void dsjit_emit_imm(const DsStmt *stmt) {
  value_data[stmt->dest] = gcc_jit_context_new_rvalue_from_long (ctx,
              gcc_jit_context_get_type(ctx, GCC_JIT_TYPE_LONG), stmt->num0);
}

static void dsjit_emit_immf(const DsStmt *stmt) {
  value_data[stmt->dest] = gcc_jit_context_new_rvalue_from_double (ctx,
              gcc_jit_context_get_type(ctx, GCC_JIT_TYPE_FLOAT), stmt->fnum0);
}

static void dsjit_emit_jump(const DsStmt *stmt) {
  if(stmt->op != dsop_jump) {
gcc_jit_block_add_comment (block_data[block_count],
         NULL, //gcc_jit_location *loc,
         "here is a cond jump");
    gcc_jit_rvalue *boolval = gcc_jit_context_new_comparison (ctx,
        NULL, //gcc_jit_location *loc,
        get_comp(stmt->op - dsop_eq_jump + dsop_eq), //......?????????????
        value_data[stmt->num0], value_data[stmt->num1]);
// get booval
        gcc_jit_block_end_with_conditional (block_data[block_count],
            NULL, // gcc_jit_location *loc,
            boolval,
            block_data[label_data[stmt->dest]],
            block_data[block_count + 1]);
//            block_data[block_count + 1],
//            block_data[label_data[stmt->dest]]);
  } else {
    gcc_jit_block_end_with_jump (block_data[block_count],
          NULL, //gcc_jit_location *loc,
           block_data[block_count+1]);

  }

  block_count++;
}

static void dsjit_emit_call(const DsStmt *stmt) {
  for(uint32_t i =0; i < fun_count; i++) {
    if(stmt->name == fun_data[i].name) {
      // we need ret register?
value_data[stmt->dest] =
      gcc_jit_context_new_call(ctx,
        NULL /*gcc_jit_location *loc*/,
        fun_data[i].code,
//        fun_data[i].narg, value_data + stmt->dest); // use num2?
        fun_data[i].narg, value_data + stmt->num1); // use num2?
      return;
   }
  }
  exit(33);
}

static void dsjit_emit_return(const DsStmt *stmt) {
  const uint32_t count = block_count;
  if(!stmt->num0) {
    gcc_jit_block_end_with_return (block_data[count],
             NULL, //gcc_jit_location *loc,
             value_data[stmt->dest]);
  } else
    gcc_jit_block_end_with_void_return (block_data[count], NULL);
  block_count++;
}

static inline void finish(void) {
  if(fun_count) {
  /* Compile the code.  */
  gcc_jit_result *result = gcc_jit_context_compile (ctx);
  if (!result)
    {
      fprintf (stderr, "NULL result");
      exit (1);
    }

    fun_data[fun_count-1].result = result; // remove
  }
}

static void dsjit_emit_function(const DsStmt *stmt) {
  fun_data[fun_count].name = stmt->name;
//  finish(); // finish old func
  block_count = 0;
}

static enum gcc_jit_types get_type(const char c) {
  if(c == 'v') return GCC_JIT_TYPE_VOID;
  if(c == 'i') return GCC_JIT_TYPE_LONG;
  //if(c == 'f')
  return GCC_JIT_TYPE_FLOAT;
}

static void dsjit_emit_argument(const DsStmt *stmt) {
  const enum gcc_jit_types t = get_type(*stmt->name);
  Fun *fun = &fun_data[fun_count];
  if(stmt->num1) {
    const uint32_t narg = fun->narg;
    gcc_jit_param *const param = param_data[narg] = gcc_jit_context_new_param (ctx, NULL, gcc_jit_context_get_type(ctx, t),  "arg");
    value_data[narg] = gcc_jit_param_as_rvalue(param);
    fun->narg++;
  } else {
block_count = 0;

    gcc_jit_function *const code = gcc_jit_context_new_function (ctx, NULL, GCC_JIT_FUNCTION_EXPORTED,
        gcc_jit_context_get_type(ctx, get_type(*stmt->name)),
        fun->name, fun->narg, param_data, 0);
    fun->code = code;
    fun_data[fun_count].code = code;
    for(uint32_t i = 0; i < fun->nblock + 1; i++) {
      char c[16];
      sprintf(c, "block%u", i);
      block_data[i] = gcc_jit_function_new_block (code, c);
    }
gcc_jit_block_add_comment (block_data[block_count],
         NULL, //gcc_jit_location *loc,
         "here is a new func");
    fun_count++;
  }
}

static void dsjit_emit_label(const DsStmt *stmt __attribute__((unused))) {
gcc_jit_block_add_comment (block_data[block_count],
         NULL, //gcc_jit_location *loc,
         "here is a label");
//if(block_data[block_count+1] != block_data[label_data[stmt->dest]])
//exit(7);
if(!block_data[block_count+1])
      block_data[block_count+1] = gcc_jit_function_new_block (fun_data[fun_count-1].code, "lkjlk");
  gcc_jit_block_end_with_jump (
  block_data[block_count],
          NULL, //gcc_jit_location *loc,
          block_data[block_count+1]);
  block_count++;
}

typedef void (*dsc_t)(const DsStmt*);

static void set_funcs_and_labels(DsScanner *ds) {
  uint32_t fun_count = 0;
//  uint32_t nlabel    = 0;
  for(size_t i = 0; i < ds->n; i++) {
    const DsStmt stmt = ds->stmts[i];
    if(stmt.type == dsc_function) {
//      if(fun_count)
//        fun_data[fun_count - 1].nlabel = nlabel;
      fun_data[fun_count].name = stmt.name;
//      nlabel = 0;
//      fun_data[fun_count].nblock = 1;
      fun_count++;
    } else if(stmt.type == dsc_label) {
//      label_data[stmt.dest] = fun_data[fun_count - 1].nblock++;
//      label_data[stmt.dest] = ++fun_data[fun_count - 1].nblock;
      label_data[stmt.dest] = ++fun_data[fun_count - 1].nblock;
//      label_data[stmt.dest] = ++fun_data[fun_count - 1].nlabel;
//      fun_data[fun_count - 1].nblock++;
      // set label at index here
    } else if(stmt.type == dsc_jump /*&& stmt.op != dsop_jump*/) {
      fun_data[fun_count - 1].nblock++;
    }
  }
}

static const dsc_t functions[dsop_max] = {
  dsjit_emit_binary,
  dsjit_emit_ibinary,
  dsjit_emit_fbinary,
  dsjit_emit_unary,
  dsjit_emit_imm,
  dsjit_emit_immf,
  dsjit_emit_jump,
  dsjit_emit_call,
  dsjit_emit_return,
  dsjit_emit_function,
  dsjit_emit_argument,
  dsjit_emit_label
};
int main(int argc, char **argv) {

ctx = gcc_jit_context_acquire();
/*
  gcc_jit_context_set_bool_option (
    ctx,
    GCC_JIT_BOOL_OPTION_DUMP_INITIAL_GIMPLE,
    1);
*/
gcc_jit_context_set_int_option (ctx,
        GCC_JIT_INT_OPTION_OPTIMIZATION_LEVEL,
        3);
  DsScanner ds = {};
  dslex_init(&ds.scanner);
  dsset_extra(&ds, ds.scanner);
  for (int i = 1; i < argc; i++) {
    FILE *file= strcmp(argv[i], "-") ?
      fopen(argv[i], "r"):
      stdin;
    if(!file) continue;
    dsset_in(file, ds.scanner);
    ds.stmts = stmt_start();
    bool ret = dsparse(&ds);
    fclose(file);
    if(ret) continue;
    set_funcs_and_labels(&ds);
    for(size_t i = 0; i < ds.n; i++) {
      const DsStmt stmt = ds.stmts[i];
      functions[stmt.type](&stmt);
    }
    stmt_release(ds.n);


//    gcc_jit_block_end_with_void_return (block_data[block_count++], NULL); //???
//    gcc_jit_block_end_with_void_return (block_data[0], NULL); //???
    finish();
    ds.n = 0;
  }
  dslex_destroy(ds.scanner);
  if(fun_count) {

  long(*_main)()  = gcc_jit_result_get_code (fun_data[fun_count-1].result, fun_data[fun_count-1].name);
  if (!_main)
    {
      fprintf (stderr, "NULL greet");
      exit (1);
    }

  /* Now call the generated function: */
  printf("result :%lu\n", _main ());
  }
  return EXIT_SUCCESS;
}
