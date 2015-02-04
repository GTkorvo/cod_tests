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
	test_output = fopen("struct-ret-1.c.output", "w");
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
	{"f", (void*)(long)-1},
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	X f (B a, char b, double c, B d);\n\
	void main ();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);";
    char *func_bodies[] = {

/* body for f */
"{\n\
  static X xr = {\"return val\", 'R'};\n\
  X r;\n\
  r = xr;\n\
  r.c1 = b;\n\
  sprintf (out, \"X f(B,char,double,B):({%g,{%d,%d,%d}},'%c',%g,{%g,{%d,%d,%d}})\",\n\
	   a.d, a.i[0], a.i[1], a.i[2], b, c, d.d, d.i[0], d.i[1], d.i[2]);\n\
  return r;\n\
}",

/* body for main */
"{\n\
  X Xr;\n\
  char tmp[100];\n\
\n\
  Xr = f (B1, c2, d3, B2);\n\
  strcpy (tmp, out);\n\
  Xr.c[0] = Xr.c1 = '\\0';\n\
  Xr = (*fp) (B1, c2, d3, B2);\n\
  if (strcmp (tmp, out))\n\
    abort ();\n\
\n\
  exit (0);\n\
}",
""};

    char *func_decls[] = {
	"X f (B a, char b, double c, B d);",
	"void main ();",
	""};

    char *global_decls[] = {
	"#include <stdio.h>\n\
#include <string.h>\n\
\n\
char out[100];",
	"typedef struct { double d; int i[3]; } B;",
	"typedef struct { char c[33],c1; } X;\n\
\n\
char c1 = 'a';\n\
char c2 = 127;\n\
char c3 = (char)128;\n\
char c4 = (char)255;\n\
char c5 = -1;\n\
\n\
double d1 = 0.1;\n\
double d2 = 0.2;\n\
double d3 = 0.3;\n\
double d4 = 0.4;\n\
double d5 = 0.5;\n\
double d6 = 0.6;\n\
double d7 = 0.7;\n\
double d8 = 0.8;\n\
double d9 = 0.9;",
	"B B1 =",
	";\n\
B B2 =",
	";\n\
X X1 =",
	";\n\
X X2 =",
	";\n\
X X3 =",
	";",
	"X (*fp) (B, char, double, B) = &f;",
""};

    int i;
    cod_code gen_code[2];
    cod_parse_context context;
    for (i=0; i < 2; i++) {
        int j;
        if (verbose) {
             printf("Working on subroutine %s\n", externs[i].extern_name);
        }
        if (i==0) {
            context = new_cod_parse_context();
            cod_assoc_externs(context, externs);
            for (j=0; j < 10; j++) {
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
        if (i == 1) {
            int (*func)() = (int(*)()) externs[i].extern_value;
            if (setjmp(env) == 0) {
                func();
            }
            if (exit_value != 0) {
                printf("Test ./generated/struct-ret-1.c failed\n");
                exit(exit_value);
            }
        } else {
            context = cod_copy_globals(context);
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp struct-ret-1.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/struct-ret-1.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/struct-ret-1.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/struct-ret-1.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/struct-ret-1.c Succeeded\n");
    return 0;
}
