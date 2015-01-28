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
	test_output = fopen("builtin-prefetch-5.c.output", "w");
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
	{"arg_ptr", (void*)(long)-1},
	{"arg_idx", (void*)(long)-1},
	{"glob_ptr", (void*)(long)-1},
	{"glob_idx", (void*)(long)-1},
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	void arg_ptr (char *p);\n\
	void arg_idx (char *p, int i);\n\
	void glob_ptr ();\n\
	void glob_idx ();\n\
	int main ();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);";
    char *func_bodies[] = {

/* body for arg_ptr */
"{\n\
  __builtin_prefetch (p, 0, 0);\n\
}",

/* body for arg_idx */
"{\n\
  __builtin_prefetch (&p[i], 0, 0);\n\
}",

/* body for glob_ptr */
"{\n\
  __builtin_prefetch (ptr, 0, 0);\n\
}",

/* body for glob_idx */
"{\n\
  __builtin_prefetch (&ptr[idx], 0, 0);\n\
}",

/* body for main */
"{\n\
  __builtin_prefetch (&s.b, 0, 0);\n\
  __builtin_prefetch (&s.c[1], 0, 0);\n\
\n\
  arg_ptr (&s.c[1]);\n\
  arg_ptr (ptr+3);\n\
  arg_idx (ptr, 3);\n\
  arg_idx (ptr+1, 2);\n\
  idx = 3;\n\
  glob_ptr ();\n\
  glob_idx ();\n\
  ptr++;\n\
  idx = 2;\n\
  glob_ptr ();\n\
  glob_idx ();\n\
  exit (0);\n\
}",
""};

    char *func_decls[] = {
	"void arg_ptr (char *p);",
	"void arg_idx (char *p, int i);",
	"void glob_ptr ();",
	"void glob_idx ();",
	"int main ();",
	""};

    char *global_decls[] = {
	"/* Test that __builtin_prefetch does no harm.\n\
\n\
   Use addresses that are unlikely to be word-aligned.  Some targets\n\
   have alignment requirements for prefetch addresses, so make sure the\n\
   compiler takes care of that.  This fails if it aborts, anything else",
	"\n\
struct S",
	"s;\n\
\n\
char arr[100];\n\
char *ptr = arr;\n\
int idx = 3;",
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
        for (j=0; j < 3; j++) {
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
                printf("Test ./generated/builtin-prefetch-5.c failed\n");
                exit(exit_value);
            }
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp builtin-prefetch-5.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/builtin-prefetch-5.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/builtin-prefetch-5.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/builtin-prefetch-5.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/builtin-prefetch-5.c Succeeded\n");
    return 0;
}
