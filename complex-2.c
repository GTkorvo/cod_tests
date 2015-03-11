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
// __complex__ double
// f (__complex__ double x, __complex__ double y)
// {
//   x += y;
//   return x;
// }
// 
// __complex__ double ag = 1.0 + 1.0i;
// __complex__ double bg = -2.0 + 2.0i;
// 
// main ()
// {
//   __complex__ double a, b, c;
// 
//   a = ag;
//   b = -2.0 + 2.0i;
//   c = f (a, b);
// 
//   if (a != 1.0 + 1.0i)
//     abort ();
//   if (b != -2.0 + 2.0i)
//     abort ();
//   if (c != -1.0 + 3.0i)
//     abort ();
// 
//   exit (0);
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
	test_output = fopen("complex-2.c.output", "w");
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
	{"printf", (void*)printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	__complex__ double f (__complex__ double x, __complex__ double y);\n\
	void main ();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);\n\
        int printf(const char *format, ...);";
    char *global_decls[] = {
	"__complex__ double ag = 1.0 + 1.0i;\n\
__complex__ double bg = -2.0 + 2.0i;",
""};

    char *func_decls[] = {
	"__complex__ double f (__complex__ double x, __complex__ double y);",
	"void main ();",
	""};

    char *func_bodies[] = {

/* body for f */
"{\n\
  x += y;\n\
  return x;\n\
}",

/* body for main */
"{\n\
  __complex__ double a, b, c;\n\
\n\
  a = ag;\n\
  b = -2.0 + 2.0i;\n\
  c = f (a, b);\n\
\n\
  if (a != 1.0 + 1.0i)\n\
    abort ();\n\
  if (b != -2.0 + 2.0i)\n\
    abort ();\n\
  if (c != -1.0 + 3.0i)\n\
    abort ();\n\
\n\
  exit (0);\n\
}",
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
        if (i == 1) {
            int (*func)() = (int(*)()) externs[i].extern_value;
            if (setjmp(env) == 0) {
                func();
            }
            if (exit_value != 0) {
                printf("Test ./generated/complex-2.c failed\n");
                exit(exit_value);
            }
        } else {
            context = cod_copy_globals(context);
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp complex-2.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/complex-2.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/complex-2.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/complex-2.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/complex-2.c Succeeded\n");
    return 0;
}
