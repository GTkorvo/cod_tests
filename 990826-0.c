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
	test_output = fopen("990826-0.c.output", "w");
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
	int main();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);";
    char *func_bodies[] = {

/* body for main */
"{\n\
  if (floor (0.1) != 0.)\n\
    abort ();\n\
  return 0;\n\
}",
""};

    char *func_decls[] = {
	"int main();",
	""};

    char *global_decls[] = {
	"/*\n\
From: niles@fan745.gsfc.nasa.gov\n\
To: fortran@gnu.org\n\
Subject: Re: Scary problems in g77 for RedHat 6.0. (glibc-2.1) \n\
Date: Sun, 06 Jun 1999 23:37:23 -0400\n\
X-UIDL: 9c1e40c572e3b306464f703461764cd5\n\
\n\
\n\
#include <stdio.h>\n\
#include <math.h>",
	"/*\n\
It will result in 36028797018963968.000000 on Alpha RedHat Linux 6.0\n\
using glibc-2.1 at least on my 21064.  This may result in g77 bug\n\
reports concerning the INT() function, just so you know.",
	"	Thanks,	\n\
	Rick Niles.",
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
        for (j=0; j < 3; j++) {
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
                printf("Test ./generated/990826-0.c failed\n");
                exit(exit_value);
            }
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp 990826-0.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/990826-0.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/990826-0.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/990826-0.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/990826-0.c Succeeded\n");
    return 0;
}
