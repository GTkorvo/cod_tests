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
	test_output = fopen("compare-1.c.output", "w");
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
	{"ieq", (void*)(long)-1},
	{"ine", (void*)(long)-1},
	{"ilt", (void*)(long)-1},
	{"ile", (void*)(long)-1},
	{"igt", (void*)(long)-1},
	{"ige", (void*)(long)-1},
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	int ieq (int x, int y, int ok);\n\
	int ine (int x, int y, int ok);\n\
	int ilt (int x, int y, int ok);\n\
	int ile (int x, int y, int ok);\n\
	int igt (int x, int y, int ok);\n\
	int ige (int x, int y, int ok);\n\
	int main ();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);";
    char *func_bodies[] = {

/* body for ieq */
"{\n\
  if ((x<=y) && (x>=y))\n\
    {\n\
      if (!ok) abort ();\n\
    }\n\
  else\n\
    if (ok) abort ();\n\
\n\
  if ((x<=y) && (x==y))\n\
    {\n\
      if (!ok) abort ();\n\
    }\n\
  else\n\
    if (ok) abort ();\n\
\n\
  if ((x<=y) && (y<=x))\n\
    {\n\
      if (!ok) abort ();\n\
    }\n\
  else\n\
    if (ok) abort ();\n\
\n\
  if ((y==x) && (x<=y))\n\
    {\n\
      if (!ok) abort ();\n\
    }\n\
  else\n\
    if (ok) abort ();\n\
}",

/* body for ine */
"{\n\
  if ((x<y) || (x>y))\n\
    {\n\
      if (!ok) abort ();\n\
    }\n\
  else\n\
    if (ok) abort ();\n\
}",

/* body for ilt */
"{\n\
  if ((x<y) && (x!=y))\n\
    {\n\
      if (!ok) abort ();\n\
    }\n\
  else\n\
    if (ok) abort ();\n\
}",

/* body for ile */
"{\n\
  if ((x<y) || (x==y))\n\
    {\n\
      if (!ok) abort ();\n\
    }\n\
  else\n\
    if (ok) abort ();\n\
}",

/* body for igt */
"{\n\
  if ((x>y) && (x!=y))\n\
    {\n\
      if (!ok) abort ();\n\
    }\n\
  else\n\
    if (ok) abort ();\n\
}",

/* body for ige */
"{\n\
  if ((x>y) || (x==y))\n\
    {\n\
      if (!ok) abort ();\n\
    }\n\
  else\n\
    if (ok) abort ();\n\
}",

/* body for main */
"{\n\
  ieq (1, 4, 0);\n\
  ieq (3, 3, 1);\n\
  ieq (5, 2, 0);\n\
\n\
  ine (1, 4, 1);\n\
  ine (3, 3, 0);\n\
  ine (5, 2, 1);\n\
\n\
  ilt (1, 4, 1);\n\
  ilt (3, 3, 0);\n\
  ilt (5, 2, 0);\n\
\n\
  ile (1, 4, 1);\n\
  ile (3, 3, 1);\n\
  ile (5, 2, 0);\n\
\n\
  igt (1, 4, 0);\n\
  igt (3, 3, 0);\n\
  igt (5, 2, 1);\n\
\n\
  ige (1, 4, 0);\n\
  ige (3, 3, 1);\n\
  ige (5, 2, 1);\n\
\n\
  return 0;\n\
}",
""};

    char *func_decls[] = {
	"int ieq (int x, int y, int ok);",
	"int ine (int x, int y, int ok);",
	"int ilt (int x, int y, int ok);",
	"int ile (int x, int y, int ok);",
	"int igt (int x, int y, int ok);",
	"int ige (int x, int y, int ok);",
	"int main ();",
	""};

    char *global_decls[] = {
	"/* Copyright (C) 2002 Free Software Foundation.\n\
\n\
   Test for correctness of composite comparisons.\n\
\n\
\n\
\n\
extern void abort (void);",
""};

    int i;
    cod_code gen_code[7];
    for (i=0; i < 7; i++) {
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
        if (i == 6) {
            int (*func)() = (int(*)()) externs[i].extern_value;
            if (setjmp(env) == 0) {
                func();
            }
            if (exit_value != 0) {
                printf("Test ./generated/compare-1.c failed\n");
                exit(exit_value);
            }
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp compare-1.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/compare-1.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/compare-1.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/compare-1.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/compare-1.c Succeeded\n");
    return 0;
}
