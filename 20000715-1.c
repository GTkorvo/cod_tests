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
// void abort(void);
// void exit(int);
// 
// void
// test1(void)
// {
//   int x = 3, y = 2;
// 
//   if ((x < y ? x++ : y++) != 2)
//     abort ();
// 
//   if (x != 3)
//     abort ();
// 
//   if (y != 3)
//     abort ();
// }
// 
// void
// test2(void)
// {
//   int x = 3, y = 2, z;
// 
//   z = (x < y) ? x++ : y++;
//   if (z != 2)
//     abort ();
// 
//   if (x != 3)
//     abort ();
// 
//   if (y != 3)
//     abort ();
// }
// 
// void
// test3(void)
// {
//   int x = 3, y = 2;
//   int xx = 3, yy = 2;
// 
//   if ((xx < yy ? x++ : y++) != 2)
//     abort ();
// 
//   if (x != 3)
//     abort ();
// 
//   if (y != 3)
//     abort ();
// }
// 
// int x, y;
// 
// static void
// init_xy(void)
// {
//   x = 3;
//   y = 2;
// }
// 
// void
// test4(void)
// {
//   init_xy();
//   if ((x < y ? x++ : y++) != 2)
//     abort ();
// 
//   if (x != 3)
//     abort ();
// 
//   if (y != 3)
//     abort ();
// }
// 
// void
// test5(void)
// {
//   int z;
// 
//   init_xy();
//   z = (x < y) ? x++ : y++;
//   if (z != 2)
//     abort ();
// 
//   if (x != 3)
//     abort ();
// 
//   if (y != 3)
//     abort ();
// }
// 
// void
// test6(void)
// {
//   int xx = 3, yy = 2;
//   int z;
// 
//   init_xy();
//   z = (xx < y) ? x++ : y++;
//   if (z != 2)
//     abort ();
// 
//   if (x != 3)
//     abort ();
// 
//   if (y != 3)
//     abort ();
// }
// 
// int
// main(){
//   test1 ();
//   test2 ();
//   test3 ();
//   test4 ();
//   test5 ();
//   test6 ();
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
	test_output = fopen("20000715-1.c.output", "w");
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
	{"test1", (void*)(long)-1},
	{"test2", (void*)(long)-1},
	{"test3", (void*)(long)-1},
	{"init_xy", (void*)(long)-1},
	{"test4", (void*)(long)-1},
	{"test5", (void*)(long)-1},
	{"test6", (void*)(long)-1},
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{"printf", (void*)printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	void test1();\n\
	void test2();\n\
	void test3();\n\
	static void init_xy();\n\
	void test4();\n\
	void test5();\n\
	void test6();\n\
	int main();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);\n\
        int printf(const char *format, ...);";
    char *global_decls[] = {
	"int x, y;",
""};

    char *func_decls[] = {
	"void test1();",
	"void test2();",
	"void test3();",
	"static void init_xy();",
	"void test4();",
	"void test5();",
	"void test6();",
	"int main();",
	""};

    char *func_bodies[] = {

/* body for test1 */
"{\n\
  int x = 3, y = 2;\n\
\n\
  if ((x < y ? x++ : y++) != 2)\n\
    abort ();\n\
\n\
  if (x != 3)\n\
    abort ();\n\
\n\
  if (y != 3)\n\
    abort ();\n\
}",

/* body for test2 */
"{\n\
  int x = 3, y = 2, z;\n\
\n\
  z = (x < y) ? x++ : y++;\n\
  if (z != 2)\n\
    abort ();\n\
\n\
  if (x != 3)\n\
    abort ();\n\
\n\
  if (y != 3)\n\
    abort ();\n\
}",

/* body for test3 */
"{\n\
  int x = 3, y = 2;\n\
  int xx = 3, yy = 2;\n\
\n\
  if ((xx < yy ? x++ : y++) != 2)\n\
    abort ();\n\
\n\
  if (x != 3)\n\
    abort ();\n\
\n\
  if (y != 3)\n\
    abort ();\n\
}",

/* body for init_xy */
"{\n\
  x = 3;\n\
  y = 2;\n\
}",

/* body for test4 */
"{\n\
  init_xy();\n\
  if ((x < y ? x++ : y++) != 2)\n\
    abort ();\n\
\n\
  if (x != 3)\n\
    abort ();\n\
\n\
  if (y != 3)\n\
    abort ();\n\
}",

/* body for test5 */
"{\n\
  int z;\n\
\n\
  init_xy();\n\
  z = (x < y) ? x++ : y++;\n\
  if (z != 2)\n\
    abort ();\n\
\n\
  if (x != 3)\n\
    abort ();\n\
\n\
  if (y != 3)\n\
    abort ();\n\
}",

/* body for test6 */
"{\n\
  int xx = 3, yy = 2;\n\
  int z;\n\
\n\
  init_xy();\n\
  z = (xx < y) ? x++ : y++;\n\
  if (z != 2)\n\
    abort ();\n\
\n\
  if (x != 3)\n\
    abort ();\n\
\n\
  if (y != 3)\n\
    abort ();\n\
}",

/* body for main */
"{\n\
  test1 ();\n\
  test2 ();\n\
  test3 ();\n\
  test4 ();\n\
  test5 ();\n\
  test6 ();\n\
  exit (0);\n\
}",
""};

    int i;
    cod_code gen_code[8];
    cod_parse_context context;
    for (i=0; i < 8; i++) {
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
        if (i == 7) {
            int (*func)() = (int(*)()) externs[i].extern_value;
            if (setjmp(env) == 0) {
                func();
            }
            if (exit_value != 0) {
                printf("Test ./20000715-1.c failed\n");
                exit(exit_value);
            }
        } else {
            context = cod_copy_globals(context);
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp 20000715-1.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/20000715-1.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./20000715-1.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./20000715-1.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./20000715-1.c Succeeded\n");
    return 0;
}
