#include "cod.h"
#undef NDEBUG
#include "assert.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <setjmp.h>

/*
 *  Original test was:
 */
// f (double x[2], double y[2])
// {
//   if (x == y)
//     abort ();
// }
// 
// main ()
// {
//   struct { int f[3]; double x[1][2]; } tp[4][2];
//   int i, j, ki, kj, mi, mj;
//   float bdm[4][2][4][2];
// 
//   for (i = 0; i < 4; i++)
//     for (j = i; j < 4; j++)
//       for (ki = 0; ki < 2; ki++)
// 	for (kj = 0; kj < 2; kj++)
// 	  if ((j == i) && (ki == kj))
// 	    bdm[i][ki][j][kj] = 1000.0;
// 	  else
// 	    {
// 	      for (mi = 0; mi < 1; mi++)
// 		for (mj = 0; mj < 1; mj++)
// 		  f (tp[i][ki].x[mi], tp[j][kj].x[mj]);
// 	      bdm[i][ki][j][kj] = 1000.0;
// 	    }
//   exit (0);
// }

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
	test_output = fopen("930628-1.c.output", "w");
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
	{"printf", (void*)printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	f (double x[2], double y[2]);\n\
	void main ();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);\n\
        int printf(const char *format, ...);";
    char *global_decls[] = {
""};

    char *func_decls[] = {
	"f (double x[2], double y[2]);",
	"void main ();",
	""};

    char *func_bodies[] = {

/* body for f */
"{\n\
  if (x == y)\n\
    abort ();\n\
}",

/* body for main */
"{\n\
  struct { int f[3]; double x[1][2]; } tp[4][2];\n\
  int i, j, ki, kj, mi, mj;\n\
  float bdm[4][2][4][2];\n\
\n\
  for (i = 0; i < 4; i++)\n\
    for (j = i; j < 4; j++)\n\
      for (ki = 0; ki < 2; ki++)\n\
	for (kj = 0; kj < 2; kj++)\n\
	  if ((j == i) && (ki == kj))\n\
	    bdm[i][ki][j][kj] = 1000.0;\n\
	  else\n\
	    {\n\
	      for (mi = 0; mi < 1; mi++)\n\
		for (mj = 0; mj < 1; mj++)\n\
		  f (tp[i][ki].x[mi], tp[j][kj].x[mj]);\n\
	      bdm[i][ki][j][kj] = 1000.0;\n\
	    }\n\
  exit (0);\n\
}",
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
            for (j=0; j < sizeof(global_decls)/sizeof(global_decls[0])-1; j++) {
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
                printf("Test ./generated/930628-1.c failed\n");
                exit(exit_value);
            }
        } else {
            context = cod_copy_globals(context);
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp 930628-1.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/930628-1.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/930628-1.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/930628-1.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/930628-1.c Succeeded\n");
    return 0;
}
