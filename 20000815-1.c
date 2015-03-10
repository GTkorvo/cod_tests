#include "cod.h"
#undef NDEBUG
#include "assert.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <setjmp.h>

/*
 *  Original test was:
 */
// struct table_elt
// {
//   void *exp;
//   struct table_elt *next_same_hash;
//   struct table_elt *prev_same_hash;
//   struct table_elt *next_same_value;
//   struct table_elt *prev_same_value;
//   struct table_elt *first_same_value;
//   struct table_elt *related_value;
//   int cost;
//   int mode;
//   char in_memory;
//   char in_struct;
//   char is_const;
//   char flag;
// };
// 
// struct write_data
// {
//   int sp : 1;			 
//   int var : 1;			 
//   int nonscalar : 1;		 
//   int all : 1;			 
// };
// 
// int cse_rtx_addr_varies_p(void *);
// void remove_from_table(struct table_elt *, int);
// static struct table_elt *table[32];
// 
// void
// invalidate_memory (writes)
//      struct write_data *writes;
// {
//   register int i;
//   register struct table_elt *p, *next;
//   int all = writes->all;
//   int nonscalar = writes->nonscalar;
// 
//   for (i = 0; i < 31; i++)
//     for (p = table[i]; p; p = next)
//       {
// 	next = p->next_same_hash;
// 	if (p->in_memory
// 	    && (all
// 		|| (nonscalar && p->in_struct)
// 		|| cse_rtx_addr_varies_p (p->exp)))
// 	  remove_from_table (p, i);
//       }
// }
// 
// int cse_rtx_addr_varies_p(void *x) { return 0; }
// void remove_from_table(struct table_elt *x, int y) { abort (); }
// 
// int
// main()
// {
//   struct write_data writes;
//   struct table_elt elt;
// 
//   __builtin_memset(&elt, 0, sizeof(elt));
//   elt.in_memory = 1;
//   table[0] = &elt;
// 
//   __builtin_memset(&writes, 0, sizeof(writes));
//   writes.var = 1;
//   writes.nonscalar = 1;
// 
//   invalidate_memory(&writes);
//   return 0;
// }

int exit_value = 0; /* success */
jmp_buf env;

void
my_abort()
{
    exit_value = 1; /* failure */
    longjmp(env, 1);
}

void
test_exit(int value)
{
    if (value != 0) {
	exit_value = value;
    }
    longjmp(env, 1);
}

FILE *test_output = NULL;
int verbose = 0;

int test_printf(const char *format, ...)
{
    int ret;
    va_list args;
    va_start(args, format);

    if (test_output == NULL) {
	test_output = fopen("20000815-1.c.output", "w");
    }
    if (verbose) vprintf(format, args);
    ret = vfprintf(test_output, format, args);

    va_end(args);
    return ret;
}

int
main(int argc, char**argv)
{
    while (argc > 1) {
	if (strcmp(argv[1], "-v") == 0) {
	    verbose++;
        }
	argc--; argv++;
    }
    cod_extern_entry externs[] = 
    {
	{"cse_rtx_addr_varies_p", (void*)(long)-1},
	{"remove_from_table", (void*)(long)-1},
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{"printf", (void*)printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	int cse_rtx_addr_varies_p(void *x);\n\
	void remove_from_table(struct table_elt *x, int y);\n\
	int main();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);\n\
        int printf(const char *format, ...);";
    char *func_bodies[] = {

/* body for cse_rtx_addr_varies_p */
"{ return 0; }",

/* body for remove_from_table */
"{ abort (); }",

/* body for main */
"{\n\
  struct write_data writes;\n\
  struct table_elt elt;\n\
\n\
  __builtin_memset(&elt, 0, sizeof(elt));\n\
  elt.in_memory = 1;\n\
  table[0] = &elt;\n\
\n\
  __builtin_memset(&writes, 0, sizeof(writes));\n\
  writes.var = 1;\n\
  writes.nonscalar = 1;\n\
\n\
  invalidate_memory(&writes);\n\
  return 0;\n\
}",
""};

    char *func_decls[] = {
	"int cse_rtx_addr_varies_p(void *x);",
	"void remove_from_table(struct table_elt *x, int y);",
	"int main();",
	""};

    char *global_decls[] = {
	"struct table_elt\n\
{\n\
  void *exp;\n\
  struct table_elt *next_same_hash;\n\
  struct table_elt *prev_same_hash;\n\
  struct table_elt *next_same_value;\n\
  struct table_elt *prev_same_value;\n\
  struct table_elt *first_same_value;\n\
  struct table_elt *related_value;\n\
  int cost;\n\
  int mode;\n\
  char in_memory;\n\
  char in_struct;\n\
  char is_const;\n\
  char flag;\n\
};",
	"struct write_data\n\
{\n\
  int sp : 1;			 \n\
  int var : 1;			 \n\
  int nonscalar : 1;		 \n\
  int all : 1;			 \n\
};\n\
\n\
int cse_rtx_addr_varies_p(void *);\n\
void remove_from_table(struct table_elt *, int);\n\
static struct table_elt *table[32];\n\
\n\
void\n\
invalidate_memory (writes)\n\
     struct write_data *writes;",
""};

    int i;
    cod_code gen_code[3];
    cod_parse_context context;
    for (i=0; i < 3; i++) {
        int j;
        if (verbose) {
             printf("Working on subroutine %s\n", externs[i].extern_name);
        }
        if (i==0) {
            context = new_cod_parse_context();
            cod_assoc_externs(context, externs);
            for (j=0; j < sizeof(global_decls)/sizeof(global_decls[0])-1; j++) {
                cod_parse_for_globals(global_decls[j], context);
            }
            cod_parse_for_context(extern_string, context);
        } else {
	    cod_extern_entry single_extern[2];
	    single_extern[0] = externs[i-1];
	    single_extern[1].extern_name = NULL;
	    single_extern[1].extern_value = NULL;
	    cod_assoc_externs(context, single_extern);
	    cod_parse_for_context(func_decls[i-1], context);
	}
        cod_subroutine_declaration(func_decls[i], context);
        gen_code[i] = cod_code_gen(func_bodies[i], context);
        externs[i].extern_value = (void*) gen_code[i]->func;
        if (i == 2) {
            int (*func)() = (int(*)()) externs[i].extern_value;
            if (setjmp(env) == 0) {
                func();
            }
            if (exit_value != 0) {
                printf("Test ./generated/20000815-1.c failed\n");
                exit(exit_value);
            }
        } else {
            context = cod_copy_globals(context);
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp 20000815-1.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/20000815-1.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/20000815-1.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/20000815-1.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/20000815-1.c Succeeded\n");
    return 0;
}
