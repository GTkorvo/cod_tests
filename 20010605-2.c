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
	test_output = fopen("20010605-2.c.output", "w");
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
	{"foo", (void*)(long)-1},
	{"foo", (void*)(long)-1},
	{"bar", (void*)(long)-1},
	{"baz", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	void foo (), bar (), baz (); int main ();\n\
	void foo (__complex__ double x);\n\
	void bar (__complex__ float x);\n\
	void baz (__complex__ long double x);\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);";
    char *func_bodies[] = {

/* body for foo */
"{\n\
  __complex__ double x;\n\
  __complex__ float y;\n\
  __complex__ long double z;\n\
  __real__ x = 1.0;\n\
  __imag__ x = 2.0;\n\
  foo (x);\n\
  __real__ y = 3.0f;\n\
  __imag__ y = 4.0f;\n\
  bar (y);\n\
  __real__ z = 5.0L;\n\
  __imag__ z = 6.0L;\n\
  baz (z);\n\
  exit (0);\n\
}",

/* body for foo */
"{\n\
  if (__real__ x != 1.0 || __imag__ x != 2.0)\n\
    abort ();\n\
}",

/* body for bar */
"{\n\
  if (__real__ x != 3.0f || __imag__ x != 4.0f)\n\
    abort ();\n\
}",

/* body for baz */
"{\n\
  if (__real__ x != 5.0L || __imag__ x != 6.0L)\n\
    abort ();\n\
}",
""};

    char *func_decls[] = {
	"void foo (), bar (), baz (); int main ();",
	"void foo (__complex__ double x);",
	"void bar (__complex__ float x);",
	"void baz (__complex__ long double x);",
	""};

    char *global_decls[] = {
""};

    int i;
    cod_code gen_code[4];
    for (i=0; i < 4; i++) {
        int j;
        if (verbose) {
             printf("Working on subroutine %s\n", externs[i].extern_name);
        }
        cod_parse_context context = new_cod_parse_context();
        cod_assoc_externs(context, externs);
        for (j=0; j < 0; j++) {
            cod_parse_for_globals(global_decls[j], context);
        }
        cod_parse_for_context(extern_string, context);
        cod_subroutine_declaration(func_decls[i], context);
        gen_code[i] = cod_code_gen(func_bodies[i], context);
        externs[i].extern_value = (void*) gen_code[i]->func;
        if (i == 3) {
            int (*func)() = (int(*)()) externs[i].extern_value;
            if (setjmp(env) == 0) {
                func();
            }
            if (exit_value != 0) {
                printf("Test ./generated/20010605-2.c failed\n");
                exit(exit_value);
            }
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp 20010605-2.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/20010605-2.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/20010605-2.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/20010605-2.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/20010605-2.c Succeeded\n");
    return 0;
}
