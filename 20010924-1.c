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
	test_output = fopen("20010924-1.c.output", "w");
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
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	void main();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);";
    char *func_bodies[] = {

/* body for main */
"{\n\
  if (a1.a1c != '4')\n\
    abort();\n\
  if (a1.a1p[0] != '6')\n\
    abort();\n\
  if (a1.a1p[1] != '2')\n\
    abort();\n\
  if (a1.a1p[2] != '\\0')\n\
    abort();\n\
\n\
  if (a2.a2c != 'v')\n\
    abort();\n\
  if (a2.a2p[0] != 'c')\n\
    abort();\n\
  if (a2.a2p[1] != 'q')\n\
    abort();\n\
\n\
  if (a3.a3c != 'o')\n\
    abort();\n\
  if (a3.a3p[0] != 'w')\n\
    abort();\n\
  if (a3.a3p[1] != 'x')\n\
    abort();\n\
\n\
  if (a4.a4c != '9')\n\
    abort();\n\
  if (a4.a4p[0] != 'e')\n\
    abort();\n\
  if (a4.a4p[1] != 'b')\n\
    abort();\n\
\n\
  return 0;\n\
}",
""};

    char *func_decls[] = {
	"void main();",
	""};

    char *global_decls[] = {
	"struct {\n\
  char a1c;\n\
  char *a1p;\n\
}",
	"a1 =",
	";",
	"struct {\n\
  char a2c;\n\
  char a2p[2];\n\
}",
	"a2 =",
	";",
	"/* The tests.  */\n\
struct",
	"a3 =",
	";",
	"struct {\n\
  char a4c;\n\
  char a4p[];\n\
}",
	"a4 =",
	";",
""};

    int i;
    cod_code gen_code[1];
    for (i=0; i < 1; i++) {
        int j;
        if (verbose) {
             printf("Working on subroutine %s\n", externs[i].extern_name);
        }
        cod_parse_context context = new_cod_parse_context();
        cod_assoc_externs(context, externs);
        for (j=0; j < 12; j++) {
            cod_parse_for_globals(global_decls[j], context);
        }
        cod_parse_for_context(extern_string, context);
        cod_subroutine_declaration(func_decls[i], context);
        gen_code[i] = cod_code_gen(func_bodies[i], context);
        externs[i].extern_value = (void*) gen_code[i]->func;
        if (i == 0) {
            int (*func)() = (int(*)()) externs[i].extern_value;
            if (setjmp(env) == 0) {
                func();
            }
            if (exit_value != 0) {
                printf("Test ./generated/20010924-1.c failed\n");
                exit(exit_value);
            }
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp 20010924-1.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/20010924-1.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/20010924-1.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/20010924-1.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/20010924-1.c Succeeded\n");
    return 0;
}
