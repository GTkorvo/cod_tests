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
	test_output = fopen("20000917-1.c.output", "w");
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
	{"one", (void*)(long)-1},
	{"zero", (void*)(long)-1},
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	int3 one ();\n\
	int3 zero ();\n\
	int main ();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);";
    char *func_bodies[] = {

/* body for one */
"{\n\
  return (int3) { 1, 1, 1 };\n\
}",

/* body for zero */
"{\n\
  return (int3) { 0, 0, 0 };\n\
}",

/* body for main */
"{\n\
  int3 a;\n\
\n\
  /* gcc allocates a temporary for the inner expression statement\n\
     to store the return value of `one'.\n\
\n\
     gcc frees the temporaries for the inner expression statement.\n\
\n\
     gcc realloates the same temporary slot to store the return\n\
     value of `zero'.\n\
\n\
     gcc expands the call to zero ahead of the expansion of the\n\
     statement expressions.  The temporary gets the value of `zero'.\n\
\n\
     gcc expands statement expressions and the stale temporary is\n\
     clobbered with the value of `one'.  The bad value is copied from\n\
     the temporary into *&a.  */\n\
\n\
  *({ ({ one (); &a; }); }) = zero ();\n\
  if (a.a && a.b && a.c)\n\
    abort ();\n\
  exit (0);\n\
}",
""};

    char *func_decls[] = {
	"int3 one ();",
	"int3 zero ();",
	"int main ();",
	""};

    char *global_decls[] = {
	"/* This bug exists in gcc-2.95, egcs-1.1.2, gcc-2.7.2 and probably",
	"\n\
typedef struct int3 { int a, b, c; } int3;",
""};

    int i;
    cod_code gen_code[3];
    for (i=0; i < 3; i++) {
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
        if (i == 2) {
            int (*func)() = (int(*)()) externs[i].extern_value;
            if (setjmp(env) == 0) {
                func();
            }
            if (exit_value != 0) {
                printf("Test ./generated/20000917-1.c failed\n");
                exit(exit_value);
            }
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp 20000917-1.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/20000917-1.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/20000917-1.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/20000917-1.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/20000917-1.c Succeeded\n");
    return 0;
}