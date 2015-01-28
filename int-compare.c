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
	test_output = fopen("int-compare.c.output", "w");
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
	{"gt", (void*)(long)-1},
	{"ge", (void*)(long)-1},
	{"lt", (void*)(long)-1},
	{"le", (void*)(long)-1},
	{"true", (void*)(long)-1},
	{"false", (void*)(long)-1},
	{"f", (void*)(long)-1},
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	int gt (int a, int b);\n\
	int ge (int a, int b);\n\
	int lt (int a, int b);\n\
	int le (int a, int b);\n\
	void true (int c);\n\
	void false (int c);\n\
	void f ();\n\
	void main ();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);";
    char *func_bodies[] = {

/* body for gt */
"{\n\
  return a > b;\n\
}",

/* body for ge */
"{\n\
  return a >= b;\n\
}",

/* body for lt */
"{\n\
  return a < b;\n\
}",

/* body for le */
"{\n\
  return a <= b;\n\
}",

/* body for true */
"{\n\
  if (!c)\n\
    abort();\n\
}",

/* body for false */
"{\n\
  if (c)\n\
    abort();\n\
}",

/* body for f */
"{\n\
  true (gt (2, 1));\n\
  false (gt (1, 2));\n\
\n\
  true (gt (INT_MAX, 0));\n\
  false (gt (0, INT_MAX));\n\
  true (gt (INT_MAX, 1));\n\
  false (gt (1, INT_MAX));\n\
\n\
  false (gt (INT_MIN, 0));\n\
  true (gt (0, INT_MIN));\n\
  false (gt (INT_MIN, 1));\n\
  true (gt (1, INT_MIN));\n\
\n\
  true (gt (INT_MAX, INT_MIN));\n\
  false (gt (INT_MIN, INT_MAX));\n\
\n\
  true (ge (2, 1));\n\
  false (ge (1, 2));\n\
\n\
  true (ge (INT_MAX, 0));\n\
  false (ge (0, INT_MAX));\n\
  true (ge (INT_MAX, 1));\n\
  false (ge (1, INT_MAX));\n\
\n\
  false (ge (INT_MIN, 0));\n\
  true (ge (0, INT_MIN));\n\
  false (ge (INT_MIN, 1));\n\
  true (ge (1, INT_MIN));\n\
\n\
  true (ge (INT_MAX, INT_MIN));\n\
  false (ge (INT_MIN, INT_MAX));\n\
\n\
  false (lt (2, 1));\n\
  true (lt (1, 2));\n\
\n\
  false (lt (INT_MAX, 0));\n\
  true (lt (0, INT_MAX));\n\
  false (lt (INT_MAX, 1));\n\
  true (lt (1, INT_MAX));\n\
\n\
  true (lt (INT_MIN, 0));\n\
  false (lt (0, INT_MIN));\n\
  true (lt (INT_MIN, 1));\n\
  false (lt (1, INT_MIN));\n\
\n\
  false (lt (INT_MAX, INT_MIN));\n\
  true (lt (INT_MIN, INT_MAX));\n\
\n\
  false (le (2, 1));\n\
  true (le (1, 2));\n\
\n\
  false (le (INT_MAX, 0));\n\
  true (le (0, INT_MAX));\n\
  false (le (INT_MAX, 1));\n\
  true (le (1, INT_MAX));\n\
\n\
  true (le (INT_MIN, 0));\n\
  false (le (0, INT_MIN));\n\
  true (le (INT_MIN, 1));\n\
  false (le (1, INT_MIN));\n\
\n\
  false (le (INT_MAX, INT_MIN));\n\
  true (le (INT_MIN, INT_MAX));\n\
}",

/* body for main */
"{\n\
  f ();\n\
  exit (0);\n\
}",
""};

    char *func_decls[] = {
	"int gt (int a, int b);",
	"int ge (int a, int b);",
	"int lt (int a, int b);",
	"int le (int a, int b);",
	"void true (int c);",
	"void false (int c);",
	"void f ();",
	"void main ();",
	""};

    char *global_decls[] = {
	"#include <limits.h>",
""};

    int i;
    cod_code gen_code[8];
    for (i=0; i < 8; i++) {
        int j;
        if (verbose) {
             printf("Working on subroutine %s\n", externs[i].extern_name);
        }
        cod_parse_context context = new_cod_parse_context();
        cod_assoc_externs(context, externs);
        for (j=0; j < 1; j++) {
            cod_parse_for_globals(global_decls[j], context);
        }
        cod_parse_for_context(extern_string, context);
        cod_subroutine_declaration(func_decls[i], context);
        gen_code[i] = cod_code_gen(func_bodies[i], context);
        externs[i].extern_value = (void*) gen_code[i]->func;
        if (i == 7) {
            int (*func)() = (int(*)()) externs[i].extern_value;
            if (setjmp(env) == 0) {
                func();
            }
            if (exit_value != 0) {
                printf("Test ./generated/int-compare.c failed\n");
                exit(exit_value);
            }
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp int-compare.c.output ./pre_patch/int-compare.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/int-compare.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/int-compare.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/int-compare.c Succeeded\n");
    return 0;
}
