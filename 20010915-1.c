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
	test_output = fopen("20010915-1.c.output", "w");
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
	{"main", (void*)(long)-1},
	{"x", (void*)(long)-1},
	{"m", (void*)(long)-1},
	{"s", (void*)(long)-1},
	{"r", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	int main (int argc, char **argv);\n\
	int x (int argc, char **argv);\n\
	char *m (char *x);\n\
	char *s (char *v, char **pp);\n\
	int r (const char *f);\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);";
    char *func_bodies[] = {

/* body for main */
"{\n\
  char *args[] = {\"a\", \"b\", \"c\", \"d\", \"e\"};\n\
  if (x (5, args) != 0 || check != 2 || o != 5)\n\
    abort ();\n\
  exit (0);\n\
}",

/* body for x */
"{\n\
  int opt = 0;\n\
  char *g = 0;\n\
  char *p = 0;\n\
\n\
  if (argc > o && argc > 2 && argv[o])\n\
    {\n\
      g = s (argv[o], &p);\n\
      if (g)\n\
	{\n\
	  *g++ = '\\0';\n\
	  h = s (g, &p);\n\
	  if (g == p)\n\
	    h = m (g);\n\
	}\n\
      u = s (argv[o], &p);\n\
      if (argv[o] == p)\n\
	u = m (argv[o]);\n\
    }\n\
  else\n\
    abort ();\n\
\n\
  while (++o < argc)\n\
    if (r (argv[o]) == 0)\n\
      return 1;\n\
\n\
  return 0;\n\
}",

/* body for m */
"{ abort (); }",

/* body for s */
"{\n\
  if (strcmp (v, \"a\") != 0 || check++ > 1)\n\
    abort ();\n\
  *pp = v+1;\n\
  return 0;\n\
}",

/* body for r */
"{\n\
  static char c[2] = \"b\";\n\
  static int cnt = 0;\n\
\n\
  if (*f != *c || f[1] != c[1] || cnt > 3)\n\
    abort ();\n\
  c[0]++;\n\
  cnt++;\n\
  return 1;\n\
}",
""};

    char *func_decls[] = {
	"int main (int argc, char **argv);",
	"int x (int argc, char **argv);",
	"char *m (char *x);",
	"char *s (char *v, char **pp);",
	"int r (const char *f);",
	""};

    char *global_decls[] = {
	"/* Bug in reorg.c, deleting the \"++\" in the last loop in main.\n\
\n\
\n\
extern void f (void);\n\
extern int x (int, char **);\n\
extern int r (const char *);\n\
extern char *s (char *, char **);\n\
extern char *m (char *);\n\
char *u;\n\
char *h;\n\
int check = 0;\n\
int o = 0;",
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
                printf("Test ./generated/20010915-1.c failed\n");
                exit(exit_value);
            }
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp 20010915-1.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/20010915-1.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/20010915-1.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/20010915-1.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/20010915-1.c Succeeded\n");
    return 0;
}