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
	test_output = fopen("20010106-1.c.output", "w");
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
	 int f(int i);\n\
	int main();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);";
    char *func_bodies[] = {

/* body for f */
"{\n\
  switch (i)\n\
  {\n\
    case -2:\n\
      return 33;\n\
    case -1:\n\
      return 0;\n\
    case 0:\n\
      return 7;\n\
    case 1:\n\
      return 4;\n\
    case 2:\n\
      return 3;\n\
    case 3:\n\
      return 15;\n\
    case 4:\n\
     return 9;\n\
    default:\n\
      abort ();\n\
  }\n\
}",

/* body for main */
"{\n\
  if (f(-1))\n\
    abort ();\n\
  exit (0);\n\
}",
""};

    char *func_decls[] = {
	" int f(int i);",
	"int main();",
	""};

    char *global_decls[] = {
	"/* Copyright 2001 Free Software Foundation",
""};

    int i;
    cod_code gen_code[2];
    for (i=0; i < 2; i++) {
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
        if (i == 1) {
            int (*func)() = (int(*)()) externs[i].extern_value;
            if (setjmp(env) == 0) {
                func();
            }
            if (exit_value != 0) {
                printf("Test ./generated/20010106-1.c failed\n");
                exit(exit_value);
            }
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp 20010106-1.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/20010106-1.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/20010106-1.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/20010106-1.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/20010106-1.c Succeeded\n");
    return 0;
}