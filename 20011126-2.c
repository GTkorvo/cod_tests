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
	test_output = fopen("20011126-2.c.output", "w");
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
	{"test", (void*)(long)-1},
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	 static const char *
test (const char *in, char *out);\n\
	int main ();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);";
    char *func_bodies[] = {

/* body for test */
"{\n\
  while (1)\n\
    {\n\
      if (*in == 'a')\n\
	{\n\
	  const char *p = in + 1;\n\
	  while (*p == 'x')\n\
	    ++p;\n\
	  if (*p == 'b')\n\
	    return p;\n\
	  while (in < p)\n\
	    *out++ = *in++;\n\
	}\n\
    }\n\
}",

/* body for main */
"{\n\
  char out[4];\n\
  test (\"aab\", out);\n\
  return 0;\n\
}",
""};

    char *func_decls[] = {
	" static const char *
test (const char *in, char *out);",
	"int main ();",
	""};

    char *global_decls[] = {
	"/* Problem originally visible on ia64.\n\
\n\
   There is a partial redundancy of \"in + 1\" that makes GCSE want to\n\
   transform the final while loop to \n\
\n\
     p = in + 1;\n\
     tmp = p;\n\
     ...\n\
     goto start;\n\
   top:\n\
     tmp = tmp + 1;\n\
   start:\n\
     in = tmp;\n\
     if (in < p) goto top;\n\
\n\
   We miscalculate the number of loop iterations as (p - tmp) = 0\n\
   instead of (p - in) = 1, which results in overflow in the doloop",
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
                printf("Test ./generated/20011126-2.c failed\n");
                exit(exit_value);
            }
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp 20011126-2.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/20011126-2.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/20011126-2.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/20011126-2.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/20011126-2.c Succeeded\n");
    return 0;
}