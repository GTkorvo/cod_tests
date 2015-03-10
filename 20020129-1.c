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
// /* This testcase failed at -O2 on IA-64, because scheduling did not take
//    into account conditional execution when using cselib for alias
//    analysis.  */
// 
// struct D { int d1; struct D *d2; };
// struct C { struct D c1; long c2, c3, c4, c5, c6; };
// struct A { struct A *a1; struct C *a2; };
// struct B { struct C b1; struct A *b2; };
// 
// extern void abort (void);
// extern void exit (int);
// 
// void
// foo (struct B *x, struct B *y)
// {
//   if (x->b2 == 0)
//     {
//       struct A *a;
// 
//       x->b2 = a = y->b2;
//       y->b2 = 0;
//       for (; a; a = a->a1)
// 	a->a2 = &x->b1;
//     }
// 
//   if (y->b2 != 0)
//     abort ();
// 
//   if (x->b1.c3 == -1)
//     {
//       x->b1.c3 = y->b1.c3;
//       x->b1.c4 = y->b1.c4;
//       y->b1.c3 = -1;
//       y->b1.c4 = 0;
//     }
// 
//   if (y->b1.c3 != -1)
//     abort ();
// }
// 
// struct B x, y;
// 
// int main ()
// {
//   y.b1.c1.d1 = 6;
//   y.b1.c3 = 145;
//   y.b1.c4 = 2448;
//   x.b1.c3 = -1;
//   foo (&x, &y);
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
	test_output = fopen("20020129-1.c.output", "w");
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
	{"foo", (void*)(long)-1},
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{"printf", (void*)printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	void foo (struct B *x, struct B *y);\n\
	int main ();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);\n\
        int printf(const char *format, ...);";
    char *func_bodies[] = {

/* body for foo */
"{\n\
  if (x->b2 == 0)\n\
    {\n\
      struct A *a;\n\
\n\
      x->b2 = a = y->b2;\n\
      y->b2 = 0;\n\
      for (; a; a = a->a1)\n\
	a->a2 = &x->b1;\n\
    }\n\
\n\
  if (y->b2 != 0)\n\
    abort ();\n\
\n\
  if (x->b1.c3 == -1)\n\
    {\n\
      x->b1.c3 = y->b1.c3;\n\
      x->b1.c4 = y->b1.c4;\n\
      y->b1.c3 = -1;\n\
      y->b1.c4 = 0;\n\
    }\n\
\n\
  if (y->b1.c3 != -1)\n\
    abort ();\n\
}",

/* body for main */
"{\n\
  y.b1.c1.d1 = 6;\n\
  y.b1.c3 = 145;\n\
  y.b1.c4 = 2448;\n\
  x.b1.c3 = -1;\n\
  foo (&x, &y);\n\
  exit (0);\n\
}",
""};

    char *func_decls[] = {
	"void foo (struct B *x, struct B *y);",
	"int main ();",
	""};

    char *global_decls[] = {
	"struct D { int d1; struct D *d2; };",
	"struct C { struct D c1; long c2, c3, c4, c5, c6; };",
	"struct A { struct A *a1; struct C *a2; };",
	"struct B { struct C b1; struct A *b2; };\n\
\n\
\n\
",
	"struct B x, y;",
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
                printf("Test ./generated/20020129-1.c failed\n");
                exit(exit_value);
            }
        } else {
            context = cod_copy_globals(context);
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp 20020129-1.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/20020129-1.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/20020129-1.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/20020129-1.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/20020129-1.c Succeeded\n");
    return 0;
}
