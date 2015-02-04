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
	test_output = fopen("loop-5.c.output", "w");
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
	{"ap", (void*)(long)-1},
	{"ap", (void*)(long)-1},
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	static int ap(int i); static void testit();\n\
	static int ap(int i);\n\
	int main();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);";
    char *func_bodies[] = {

/* body for ap */
"{\n\
  int ir[4] = {0,1,2,3};\n\
  int ix,n,m;\n\
  n=1; m=3;\n\
  for (ix=1;ix<=4;ix++) {\n\
    if (n == 1) m = 4;\n\
    else        m = n-1;\n\
    ap(ir[n-1]);\n\
    n = m;\n\
  }\n\
}",

/* body for ap */
"{\n\
  if (t > 3)\n\
    abort();\n\
  a[t++] = i;\n\
  return 1;\n\
}",

/* body for main */
"{\n\
  testit();\n\
  if (a[0] != 0)\n\
    abort();\n\
  if (a[1] != 3)\n\
    abort();\n\
  if (a[2] != 2)\n\
    abort();\n\
  if (a[3] != 1)\n\
    abort();\n\
  exit(0);\n\
}",
""};

    char *func_decls[] = {
	"static int ap(int i); static void testit();",
	"static int ap(int i);",
	"int main();",
	""};

    char *global_decls[] = {
	"static int t = 0;\n\
static int a[4];",
""};

    int i;
    cod_code gen_code[3];
    cod_parse_context context;
    for (i=0; i < 3; i++) {
        int j;
        if (verbose) {
             printf("Working on subroutine %s\n", externs[i].extern_name);
        }
        if (i==0) {
            context = new_cod_parse_context();
            cod_assoc_externs(context, externs);
            for (j=0; j < 1; j++) {
                cod_parse_for_globals(global_decls[j], context);
            }
            cod_parse_for_context(extern_string, context);
        } else {
	    cod_extern_entry single_extern[2];
	    single_extern[0] = externs[i-1];
	    single_extern[1].extern_name = NULL;
	    single_extern[1].extern_value = NULL;
	    cod_assoc_externs(context, single_extern);
	    cod_parse_for_context(func_decls[i-1], context);
	}
        cod_subroutine_declaration(func_decls[i], context);
        gen_code[i] = cod_code_gen(func_bodies[i], context);
        externs[i].extern_value = (void*) gen_code[i]->func;
        if (i == 2) {
            int (*func)() = (int(*)()) externs[i].extern_value;
            if (setjmp(env) == 0) {
                func();
            }
            if (exit_value != 0) {
                printf("Test ./generated/loop-5.c failed\n");
                exit(exit_value);
            }
        } else {
            context = cod_copy_globals(context);
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp loop-5.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/loop-5.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/loop-5.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/loop-5.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/loop-5.c Succeeded\n");
    return 0;
}
