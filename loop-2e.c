#include "cod.h"
#undef NDEBUG
#include "assert.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <setjmp.h>

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
	test_output = fopen("loop-2e.c.output", "w");
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
	{"f", (void*)(long)-1},
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	void f (int *p, int **q);\n\
	int main ();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);";
    char *func_bodies[] = {

/* body for f */
"{\n\
  int i;\n\
  for (i = 0; i < 40; i++)\n\
    {\n\
      *q++ = &p[i];\n\
    }\n\
}",

/* body for main */
"{\n\
  void *p;\n\
  int *q[40];\n\
  __SIZE_TYPE__ start;\n\
\n\
  /* Find the signed middle of the address space.  */\n\
  if (sizeof(start) == sizeof(int))\n\
    start = (__SIZE_TYPE__) __INT_MAX__;\n\
  else if (sizeof(start) == sizeof(long))\n\
    start = (__SIZE_TYPE__) __LONG_MAX__;\n\
  else if (sizeof(start) == sizeof(long long))\n\
    start = (__SIZE_TYPE__) __LONG_LONG_MAX__;\n\
  else\n\
    return 0;\n\
\n\
  /* Arbitrarily align the pointer.  */\n\
  start &= -32;\n\
\n\
  /* Pretend that's good enough to start address arithmetic.  */\n\
  p = (void *)start;\n\
\n\
  /* Verify that GIV replacement computes the correct results.  */\n\
  q[39] = 0;\n\
  f (p, q);\n\
  if (q[39] != (int *)p + 39)\n\
    abort ();\n\
\n\
  return 0;\n\
}",
""};

    char *func_decls[] = {
	"void f (int *p, int **q);",
	"int main ();",
	""};

    char *global_decls[] = {
""};

    int i;
    cod_code gen_code[2];
    cod_parse_context context;
    for (i=0; i < 2; i++) {
        int j;
        if (verbose) {
             printf("Working on subroutine %s\n", externs[i].extern_name);
        }
        if (i==0) {
            context = new_cod_parse_context();
            cod_assoc_externs(context, externs);
            for (j=0; j < 0; j++) {
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
        if (i == 1) {
            int (*func)() = (int(*)()) externs[i].extern_value;
            if (setjmp(env) == 0) {
                func();
            }
            if (exit_value != 0) {
                printf("Test ./generated/loop-2e.c failed\n");
                exit(exit_value);
            }
        } else {
            context = cod_copy_globals(context);
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp loop-2e.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/loop-2e.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/loop-2e.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/loop-2e.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/loop-2e.c Succeeded\n");
    return 0;
}
