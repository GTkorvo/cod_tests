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
	test_output = fopen("20020529-1.c.output", "w");
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
	{"matching", (void*)(long)-1},
	{"foo", (void*)(long)-1},
	{"main", (void*)(long)-1},
	{"f1", (void*)(long)-1},
	{"f2", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	/* PR target/6838 from cato@df.lth.se.    cris-elf got an ICE with -O2: the insn matching       ();\n\
	int foo (struct xx *p, int b, int c, int d);\n\
	int main ();\n\
	int f1 (struct xx *p);\n\
	void f2 ();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);";
    char *func_bodies[] = {

/* body for matching */
"{*mov_sidehi_mem}",

/* body for foo */
"{\n\
  int a;\n\
\n\
  for (;;)\n\
    {\n\
      a = f1(p);\n\
      if (a)\n\
	return (0);\n\
      if (b)\n\
	continue;\n\
      p->c = d;\n\
      if (p->a)\n\
	f2 ();\n\
      if (c)\n\
	f2 ();\n\
      d = p->c;\n\
      switch (a)\n\
	{\n\
	case 1:\n\
	  if (p->b)\n\
	    f2 ();\n\
	  if (c)\n\
	    f2 ();\n\
	default:\n\
	  break;\n\
	}\n\
    }\n\
  return d;\n\
}",

/* body for main */
"{\n\
  struct xx s = {0, &s, 23};\n\
  if (foo (&s, 0, 0, 0) != 0 || s.a != 0 || s.b != &s || s.c != 0)\n\
    abort ();\n\
  exit (0);\n\
}",

/* body for f1 */
"{\n\
  static int beenhere = 0;\n\
  if (beenhere++ > 1)\n\
    abort ();\n\
  return beenhere > 1;\n\
}",

/* body for f2 */
"{\n\
  abort ();\n\
}",
""};

    char *func_decls[] = {
	"/* PR target/6838 from cato@df.lth.se.    cris-elf got an ICE with -O2: the insn matching       ();",
	"int foo (struct xx *p, int b, int c, int d);",
	"int main ();",
	"int f1 (struct xx *p);",
	"void f2 ();",
	""};

    char *global_decls[] = {
	"(nil)\n\
	  (nil))\n\
   forced a splitter through the output pattern \"#\", but there was no\n\
   matching splitter.  */",
	"struct xx\n\
 {\n\
   int a;\n\
   struct xx *b;\n\
   short c;\n\
 };\n\
\n\
int f1 (struct xx *);\n\
void f2 (void);",
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
        for (j=0; j < 2; j++) {
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
                printf("Test ./generated/20020529-1.c failed\n");
                exit(exit_value);
            }
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp 20020529-1.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/20020529-1.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/20020529-1.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/20020529-1.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/20020529-1.c Succeeded\n");
    return 0;
}
