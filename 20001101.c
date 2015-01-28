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
	test_output = fopen("20001101.c.output", "w");
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
	{"abort", (void*)(long)-1},
	{"dummy", (void*)(long)-1},
	{"bogus", (void*)(long)-1},
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	extern void abort();\n\
	rtx dummy ( int *a, rtx *b);\n\
	void bogus (rtx delay_list, rtx thread, rtx insn);\n\
	int main();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);";
    char *func_bodies[] = {

/* body for abort */
"{\n\
  unsigned int unchanging : 1;\n\
}",

/* body for dummy */
"{\n\
  *a = 1;\n\
  *b = (rtx)7;\n\
  return (rtx)1;\n\
}",

/* body for bogus */
"{\n\
  rtx new_thread;\n\
  int must_annul;\n\
\n\
  delay_list = dummy ( &must_annul, &new_thread);\n\
  if (delay_list == 0 &&  new_thread )\n\
    {\n\
      thread = new_thread;\n\
    }\n\
  if (delay_list && must_annul)\n\
    insn->unchanging = 1;\n\
  if (new_thread != thread )\n\
    abort();\n\
}",

/* body for main */
"{\n\
  struc baz;\n\
  bogus (&baz, (rtx)7, 0);\n\
  exit(0);\n\
  return 0;\n\
}",
""};

    char *func_decls[] = {
	"extern void abort();",
	"rtx dummy ( int *a, rtx *b);",
	"void bogus (rtx delay_list, rtx thread, rtx insn);",
	"int main();",
	""};

    char *global_decls[] = {
	"struc, *rtx;",
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
        for (j=0; j < 1; j++) {
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
                printf("Test ./generated/20001101.c failed\n");
                exit(exit_value);
            }
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp 20001101.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/20001101.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/20001101.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/20001101.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/20001101.c Succeeded\n");
    return 0;
}