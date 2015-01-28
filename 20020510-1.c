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
	test_output = fopen("20020510-1.c.output", "w");
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
	{"testc", (void*)(long)-1},
	{"tests", (void*)(long)-1},
	{"testi", (void*)(long)-1},
	{"testl", (void*)(long)-1},
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	void testc (unsigned char c, int ok);\n\
	void tests (unsigned short s, int ok);\n\
	void testi (unsigned int i, int ok);\n\
	void testl (unsigned long l, int ok);\n\
	int main ();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);";
    char *func_bodies[] = {

/* body for testc */
"{\n\
  if ((c>=1) && (c<=SCHAR_MAX))\n\
    {\n\
      if (!ok) abort ();\n\
    }\n\
  else\n\
    if (ok) abort ();\n\
}",

/* body for tests */
"{\n\
  if ((s>=1) && (s<=SHRT_MAX))\n\
    {\n\
      if (!ok) abort ();\n\
    }\n\
  else\n\
    if (ok) abort ();\n\
}",

/* body for testi */
"{\n\
  if ((i>=1) && (i<=INT_MAX))\n\
    {\n\
      if (!ok) abort ();\n\
    }\n\
  else\n\
    if (ok) abort ();\n\
}",

/* body for testl */
"{\n\
  if ((l>=1) && (l<=LONG_MAX))\n\
    {\n\
      if (!ok) abort ();\n\
    }\n\
  else\n\
    if (ok) abort ();\n\
}",

/* body for main */
"{\n\
  testc (0, 0);\n\
  testc (1, 1);\n\
  testc (SCHAR_MAX, 1);\n\
  testc (SCHAR_MAX+1, 0);\n\
  testc (UCHAR_MAX, 0);\n\
\n\
  tests (0, 0);\n\
  tests (1, 1);\n\
  tests (SHRT_MAX, 1);\n\
  tests (SHRT_MAX+1, 0);\n\
  tests (USHRT_MAX, 0);\n\
\n\
  testi (0, 0);\n\
  testi (1, 1);\n\
  testi (INT_MAX, 1);\n\
  testi (INT_MAX+1U, 0);\n\
  testi (UINT_MAX, 0);\n\
\n\
  testl (0, 0);\n\
  testl (1, 1);\n\
  testl (LONG_MAX, 1);\n\
  testl (LONG_MAX+1UL, 0);\n\
  testl (ULONG_MAX, 0);\n\
\n\
  return 0;\n\
}",
""};

    char *func_decls[] = {
	"void testc (unsigned char c, int ok);",
	"void tests (unsigned short s, int ok);",
	"void testi (unsigned int i, int ok);",
	"void testl (unsigned long l, int ok);",
	"int main ();",
	""};

    char *global_decls[] = {
	"/* Copyright (C) 2002  Free Software Foundation.\n\
\n\
   Test that optimizing ((c>=1) && (c<=127)) into (signed char)c < 0\n\
   doesn't cause any problems for the compiler and behaves correctly.\n\
\n\
\n\
\n\
#include <limits.h>\n\
\n\
extern void abort (void);",
""};

    int i;
    cod_code gen_code[5];
    for (i=0; i < 5; i++) {
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
        if (i == 4) {
            int (*func)() = (int(*)()) externs[i].extern_value;
            if (setjmp(env) == 0) {
                func();
            }
            if (exit_value != 0) {
                printf("Test ./generated/20020510-1.c failed\n");
                exit(exit_value);
            }
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp 20020510-1.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/20020510-1.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/20020510-1.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/20020510-1.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/20020510-1.c Succeeded\n");
    return 0;
}
