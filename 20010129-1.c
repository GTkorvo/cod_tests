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
	test_output = fopen("20010129-1.c.output", "w");
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
	{"baz1", (void*)(long)-1},
	{"baz2", (void*)(long)-1},
	{"baz3", (void*)(long)-1},
	{"foo", (void*)(long)-1},
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	long baz1 (void *a);\n\
	int baz2 (const char *a);\n\
	int baz3 (int i);\n\
	int foo (void *a, long b, int c);\n\
	int main ();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);";
    char *func_bodies[] = {

/* body for baz1 */
"{\n\
  static long l;\n\
  return l++;\n\
}",

/* body for baz2 */
"{\n\
  return 0;\n\
}",

/* body for baz3 */
"{\n\
  if (!i)\n\
    abort ();\n\
  return 1;\n\
}",

/* body for foo */
"{\n\
  int d = 0, e, f = 0, i;\n\
  char g[256];\n\
  void **h;\n\
\n\
  g[0] = '\\n';\n\
  g[1] = 0;\n\
\n\
  while (baz1 (a) < b) {\n\
    if (g[0] != ' ' && g[0] != '\\t') {\n\
      f = 1;\n\
      e = 0;\n\
      if (!d && baz2 (g) == 0) {\n\
	if ((c & 0x10) == 0)\n\
	  continue;\n\
	e = d = 1;\n\
      }\n\
      if (!((c & 0x10) && (c & 0x4000) && e) && (c & 2))\n\
	continue;\n\
      if ((c & 0x2000) && baz2 (g) == 0)\n\
	continue;\n\
      if ((c & 0x1408) && baz2 (g) == 0)\n\
	continue;\n\
      if ((c & 0x200) && baz2 (g) == 0)\n\
	continue;\n\
      if (c & 0x80) {\n\
	for (h = bar, i = 0; h; h = (void **)*h, i++)\n\
	  if (baz3 (i))\n\
	    break;\n\
      }\n\
      f = 0;\n\
    }\n\
  }\n\
  return 0;\n\
}",

/* body for main */
"{\n\
  void *n = 0;\n\
  bar = &n;\n\
  foo (&n, 1, 0xc811);\n\
  exit (0);\n\
}",
""};

    char *func_decls[] = {
	"long baz1 (void *a);",
	"int baz2 (const char *a);",
	"int baz3 (int i);",
	"int foo (void *a, long b, int c);",
	"int main ();",
	""};

    char *global_decls[] = {
	"void **bar;",
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
                printf("Test ./generated/20010129-1.c failed\n");
                exit(exit_value);
            }
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp 20010129-1.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/20010129-1.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/20010129-1.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/20010129-1.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/20010129-1.c Succeeded\n");
    return 0;
}
