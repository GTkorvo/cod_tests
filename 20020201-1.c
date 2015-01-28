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
	test_output = fopen("20020201-1.c.output", "w");
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
	int main ();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);";
    char *func_bodies[] = {

/* body for main */
"{\n\
  unsigned char cy;\n\
  unsigned short sy;\n\
  unsigned int iy;\n\
  unsigned long ly;\n\
  unsigned long long Ly;\n\
  \n\
  cy = cx / 6; if (cy != 1) abort ();\n\
  cy = cx % 6; if (cy != 1) abort ();\n\
\n\
  sy = sx / 6; if (sy != 2) abort ();\n\
  sy = sx % 6; if (sy != 2) abort ();\n\
\n\
  iy = ix / 6; if (iy != 3) abort ();\n\
  iy = ix % 6; if (iy != 3) abort ();\n\
\n\
  ly = lx / 6; if (ly != 4) abort ();\n\
  ly = lx % 6; if (ly != 4) abort ();\n\
\n\
  Ly = Lx / 6; if (Ly != 5) abort ();\n\
  Ly = Lx % 6; if (Ly != 5) abort ();\n\
\n\
  exit(0);\n\
}",
""};

    char *func_decls[] = {
	"int main ();",
	""};

    char *global_decls[] = {
	"extern void abort (void);\n\
extern void exit (int);\n\
\n\
unsigned char cx = 7;\n\
unsigned short sx = 14;\n\
unsigned int ix = 21;\n\
unsigned long lx = 28;\n\
unsigned long long Lx = 35;",
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
        for (j=0; j < 1; j++) {
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
                printf("Test ./generated/20020201-1.c failed\n");
                exit(exit_value);
            }
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp 20020201-1.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/20020201-1.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/20020201-1.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/20020201-1.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/20020201-1.c Succeeded\n");
    return 0;
}
