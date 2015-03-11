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
// struct S
// {
//   int *sp, fc, *sc, a[2];
// };
// 
// f (struct S *x)
// {
//   int *t = x->sc;
//   int t1 = t[0];
//   int t2 = t[1];
//   int t3 = t[2];
//   int a0 = x->a[0];
//   int a1 = x->a[1];
//   asm("": :"r" (t2), "r" (t3));
//   t[2] = t1;
//   t[0] = a1;
//   x->a[1] = a0;
//   x->a[0] = t3;
//   x->fc = t2;
//   x->sp = t;
// }
// 
// main ()
// {
//   struct S s;
//   static int sc[3] = {2, 3, 4};
//   s.sc = sc;
//   s.a[0] = 10;
//   s.a[1] = 11;
//   f (&s);
//   if (s.sp[2] != 2)
//     abort ();
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
	test_output = fopen("960312-1.c.output", "w");
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
	f (struct S *x);\n\
	void main ();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);\n\
        int printf(const char *format, ...);";
    char *global_decls[] = {
	"struct S\n\
{\n\
  int *sp, fc, *sc, a[2];\n\
};",
""};

    char *func_decls[] = {
	"f (struct S *x);",
	"void main ();",
	""};

    char *func_bodies[] = {

/* body for f */
"{\n\
  int *t = x->sc;\n\
  int t1 = t[0];\n\
  int t2 = t[1];\n\
  int t3 = t[2];\n\
  int a0 = x->a[0];\n\
  int a1 = x->a[1];\n\
  asm(\"\": :\"r\" (t2), \"r\" (t3));\n\
  t[2] = t1;\n\
  t[0] = a1;\n\
  x->a[1] = a0;\n\
  x->a[0] = t3;\n\
  x->fc = t2;\n\
  x->sp = t;\n\
}",

/* body for main */
"{\n\
  struct S s;\n\
  static int sc[3] = {2, 3, 4};\n\
  s.sc = sc;\n\
  s.a[0] = 10;\n\
  s.a[1] = 11;\n\
  f (&s);\n\
  if (s.sp[2] != 2)\n\
    abort ();\n\
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
                printf("Test ./generated/960312-1.c failed\n");
                exit(exit_value);
            }
        } else {
            context = cod_copy_globals(context);
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp 960312-1.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/960312-1.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/960312-1.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/960312-1.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/960312-1.c Succeeded\n");
    return 0;
}
