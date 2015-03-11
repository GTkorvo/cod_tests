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
// /* Program to test gcc's usage of the gofast library.  */
// 
// /* The main guiding themes are to make it trivial to add test cases over time
//    and to make it easy for a program to parse the output to see if the right
//    libcalls are being made.  */
// 
// #include <stdio.h>
// 
// float fp_add (float a, float b) { return a + b; }
// float fp_sub (float a, float b) { return a - b; }
// float fp_mul (float a, float b) { return a * b; }
// float fp_div (float a, float b) { return a / b; }
// float fp_neg (float a) { return -a; }
// 
// double dp_add (double a, double b) { return a + b; }
// double dp_sub (double a, double b) { return a - b; }
// double dp_mul (double a, double b) { return a * b; }
// double dp_div (double a, double b) { return a / b; }
// double dp_neg (double a) { return -a; }
// 
// double fp_to_dp (float f) { return f; }
// float dp_to_fp (double d) { return d; }
// 
// int eqsf2 (float a, float b) { return a == b; }
// int nesf2 (float a, float b) { return a != b; }
// int gtsf2 (float a, float b) { return a > b; }
// int gesf2 (float a, float b) { return a >= b; }
// int ltsf2 (float a, float b) { return a < b; }
// int lesf2 (float a, float b) { return a <= b; }
// 
// int eqdf2 (double a, double b) { return a == b; }
// int nedf2 (double a, double b) { return a != b; }
// int gtdf2 (double a, double b) { return a > b; }
// int gedf2 (double a, double b) { return a >= b; }
// int ltdf2 (double a, double b) { return a < b; }
// int ledf2 (double a, double b) { return a <= b; }
// 
// float floatsisf (int i) { return i; }
// double floatsidf (int i) { return i; }
// int fixsfsi (float f) { return f; }
// int fixdfsi (double d) { return d; }
// unsigned int fixunssfsi (float f) { return f; }
// unsigned int fixunsdfsi (double d) { return d; }
// 
// int fail_count = 0;
// 
// int
// fail (char *msg)
// {
//   fail_count++;
//   fprintf (stderr, "Test failed: %s\n", msg);
// }
// 
// int
// main()
// {
//   if (fp_add (1, 1) != 2) fail ("fp_add 1+1");
//   if (fp_sub (3, 2) != 1) fail ("fp_sub 3-2");
//   if (fp_mul (2, 3) != 6) fail ("fp_mul 2*3");
//   if (fp_div (3, 2) != 1.5) fail ("fp_div 3/2");
//   if (fp_neg (1) != -1) fail ("fp_neg 1");
// 
//   if (dp_add (1, 1) != 2) fail ("dp_add 1+1");
//   if (dp_sub (3, 2) != 1) fail ("dp_sub 3-2");
//   if (dp_mul (2, 3) != 6) fail ("dp_mul 2*3");
//   if (dp_div (3, 2) != 1.5) fail ("dp_div 3/2");
//   if (dp_neg (1) != -1) fail ("dp_neg 1");
// 
//   if (fp_to_dp (1.5) != 1.5) fail ("fp_to_dp 1.5");
//   if (dp_to_fp (1.5) != 1.5) fail ("dp_to_fp 1.5");
// 
//   if (floatsisf (1) != 1) fail ("floatsisf 1");
//   if (floatsidf (1) != 1) fail ("floatsidf 1");
//   if (fixsfsi (1.42) != 1) fail ("fixsfsi 1.42");
//   if (fixunssfsi (1.42) != 1) fail ("fixunssfsi 1.42");
//   if (fixdfsi (1.42) != 1) fail ("fixdfsi 1.42");
//   if (fixunsdfsi (1.42) != 1) fail ("fixunsdfsi 1.42");
// 
//   if (eqsf2 (1, 1) == 0) fail ("eqsf2 1==1");
//   if (eqsf2 (1, 2) != 0) fail ("eqsf2 1==2");
//   if (nesf2 (1, 2) == 0) fail ("nesf2 1!=1");
//   if (nesf2 (1, 1) != 0) fail ("nesf2 1!=1");
//   if (gtsf2 (2, 1) == 0) fail ("gtsf2 2>1");
//   if (gtsf2 (1, 1) != 0) fail ("gtsf2 1>1");
//   if (gtsf2 (0, 1) != 0) fail ("gtsf2 0>1");
//   if (gesf2 (2, 1) == 0) fail ("gesf2 2>=1");
//   if (gesf2 (1, 1) == 0) fail ("gesf2 1>=1");
//   if (gesf2 (0, 1) != 0) fail ("gesf2 0>=1");
//   if (ltsf2 (1, 2) == 0) fail ("ltsf2 1<2");
//   if (ltsf2 (1, 1) != 0) fail ("ltsf2 1<1");
//   if (ltsf2 (1, 0) != 0) fail ("ltsf2 1<0");
//   if (lesf2 (1, 2) == 0) fail ("lesf2 1<=2");
//   if (lesf2 (1, 1) == 0) fail ("lesf2 1<=1");
//   if (lesf2 (1, 0) != 0) fail ("lesf2 1<=0");
// 
//   if (fail_count != 0)
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
	test_output = fopen("gofast.c.output", "w");
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
	{"fp_add", (void*)(long)-1},
	{"fp_sub", (void*)(long)-1},
	{"fp_mul", (void*)(long)-1},
	{"fp_div", (void*)(long)-1},
	{"fp_neg", (void*)(long)-1},
	{"dp_add", (void*)(long)-1},
	{"dp_sub", (void*)(long)-1},
	{"dp_mul", (void*)(long)-1},
	{"dp_div", (void*)(long)-1},
	{"dp_neg", (void*)(long)-1},
	{"fp_to_dp", (void*)(long)-1},
	{"dp_to_fp", (void*)(long)-1},
	{"eqsf2", (void*)(long)-1},
	{"nesf2", (void*)(long)-1},
	{"gtsf2", (void*)(long)-1},
	{"gesf2", (void*)(long)-1},
	{"ltsf2", (void*)(long)-1},
	{"lesf2", (void*)(long)-1},
	{"eqdf2", (void*)(long)-1},
	{"nedf2", (void*)(long)-1},
	{"gtdf2", (void*)(long)-1},
	{"gedf2", (void*)(long)-1},
	{"ltdf2", (void*)(long)-1},
	{"ledf2", (void*)(long)-1},
	{"floatsisf", (void*)(long)-1},
	{"floatsidf", (void*)(long)-1},
	{"fixsfsi", (void*)(long)-1},
	{"fixdfsi", (void*)(long)-1},
	{"fixunssfsi", (void*)(long)-1},
	{"fixunsdfsi", (void*)(long)-1},
	{"fail", (void*)(long)-1},
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{"printf", (void*)printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	float fp_add (float a, float b);\n\
	float fp_sub (float a, float b);\n\
	float fp_mul (float a, float b);\n\
	float fp_div (float a, float b);\n\
	float fp_neg (float a);\n\
	double dp_add (double a, double b);\n\
	double dp_sub (double a, double b);\n\
	double dp_mul (double a, double b);\n\
	double dp_div (double a, double b);\n\
	double dp_neg (double a);\n\
	double fp_to_dp (float f);\n\
	float dp_to_fp (double d);\n\
	int eqsf2 (float a, float b);\n\
	int nesf2 (float a, float b);\n\
	int gtsf2 (float a, float b);\n\
	int gesf2 (float a, float b);\n\
	int ltsf2 (float a, float b);\n\
	int lesf2 (float a, float b);\n\
	int eqdf2 (double a, double b);\n\
	int nedf2 (double a, double b);\n\
	int gtdf2 (double a, double b);\n\
	int gedf2 (double a, double b);\n\
	int ltdf2 (double a, double b);\n\
	int ledf2 (double a, double b);\n\
	float floatsisf (int i);\n\
	double floatsidf (int i);\n\
	int fixsfsi (float f);\n\
	int fixdfsi (double d);\n\
	unsigned int fixunssfsi (float f);\n\
	unsigned int fixunsdfsi (double d);\n\
	int fail (char *msg);\n\
	int main();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);\n\
        int printf(const char *format, ...);";
    char *global_decls[] = {
	"#include <stdio.h>",
	"int fail_count = 0;",
""};

    char *func_decls[] = {
	"float fp_add (float a, float b);",
	"float fp_sub (float a, float b);",
	"float fp_mul (float a, float b);",
	"float fp_div (float a, float b);",
	"float fp_neg (float a);",
	"double dp_add (double a, double b);",
	"double dp_sub (double a, double b);",
	"double dp_mul (double a, double b);",
	"double dp_div (double a, double b);",
	"double dp_neg (double a);",
	"double fp_to_dp (float f);",
	"float dp_to_fp (double d);",
	"int eqsf2 (float a, float b);",
	"int nesf2 (float a, float b);",
	"int gtsf2 (float a, float b);",
	"int gesf2 (float a, float b);",
	"int ltsf2 (float a, float b);",
	"int lesf2 (float a, float b);",
	"int eqdf2 (double a, double b);",
	"int nedf2 (double a, double b);",
	"int gtdf2 (double a, double b);",
	"int gedf2 (double a, double b);",
	"int ltdf2 (double a, double b);",
	"int ledf2 (double a, double b);",
	"float floatsisf (int i);",
	"double floatsidf (int i);",
	"int fixsfsi (float f);",
	"int fixdfsi (double d);",
	"unsigned int fixunssfsi (float f);",
	"unsigned int fixunsdfsi (double d);",
	"int fail (char *msg);",
	"int main();",
	""};

    char *func_bodies[] = {

/* body for fp_add */
"{ return a + b; }",

/* body for fp_sub */
"{ return a - b; }",

/* body for fp_mul */
"{ return a * b; }",

/* body for fp_div */
"{ return a / b; }",

/* body for fp_neg */
"{ return -a; }",

/* body for dp_add */
"{ return a + b; }",

/* body for dp_sub */
"{ return a - b; }",

/* body for dp_mul */
"{ return a * b; }",

/* body for dp_div */
"{ return a / b; }",

/* body for dp_neg */
"{ return -a; }",

/* body for fp_to_dp */
"{ return f; }",

/* body for dp_to_fp */
"{ return d; }",

/* body for eqsf2 */
"{ return a == b; }",

/* body for nesf2 */
"{ return a != b; }",

/* body for gtsf2 */
"{ return a > b; }",

/* body for gesf2 */
"{ return a >= b; }",

/* body for ltsf2 */
"{ return a < b; }",

/* body for lesf2 */
"{ return a <= b; }",

/* body for eqdf2 */
"{ return a == b; }",

/* body for nedf2 */
"{ return a != b; }",

/* body for gtdf2 */
"{ return a > b; }",

/* body for gedf2 */
"{ return a >= b; }",

/* body for ltdf2 */
"{ return a < b; }",

/* body for ledf2 */
"{ return a <= b; }",

/* body for floatsisf */
"{ return i; }",

/* body for floatsidf */
"{ return i; }",

/* body for fixsfsi */
"{ return f; }",

/* body for fixdfsi */
"{ return d; }",

/* body for fixunssfsi */
"{ return f; }",

/* body for fixunsdfsi */
"{ return d; }",

/* body for fail */
"{\n\
  fail_count++;\n\
  fprintf (stderr, \"Test failed: %s\\n\", msg);\n\
}",

/* body for main */
"{\n\
  if (fp_add (1, 1) != 2) fail (\"fp_add 1+1\");\n\
  if (fp_sub (3, 2) != 1) fail (\"fp_sub 3-2\");\n\
  if (fp_mul (2, 3) != 6) fail (\"fp_mul 2*3\");\n\
  if (fp_div (3, 2) != 1.5) fail (\"fp_div 3/2\");\n\
  if (fp_neg (1) != -1) fail (\"fp_neg 1\");\n\
\n\
  if (dp_add (1, 1) != 2) fail (\"dp_add 1+1\");\n\
  if (dp_sub (3, 2) != 1) fail (\"dp_sub 3-2\");\n\
  if (dp_mul (2, 3) != 6) fail (\"dp_mul 2*3\");\n\
  if (dp_div (3, 2) != 1.5) fail (\"dp_div 3/2\");\n\
  if (dp_neg (1) != -1) fail (\"dp_neg 1\");\n\
\n\
  if (fp_to_dp (1.5) != 1.5) fail (\"fp_to_dp 1.5\");\n\
  if (dp_to_fp (1.5) != 1.5) fail (\"dp_to_fp 1.5\");\n\
\n\
  if (floatsisf (1) != 1) fail (\"floatsisf 1\");\n\
  if (floatsidf (1) != 1) fail (\"floatsidf 1\");\n\
  if (fixsfsi (1.42) != 1) fail (\"fixsfsi 1.42\");\n\
  if (fixunssfsi (1.42) != 1) fail (\"fixunssfsi 1.42\");\n\
  if (fixdfsi (1.42) != 1) fail (\"fixdfsi 1.42\");\n\
  if (fixunsdfsi (1.42) != 1) fail (\"fixunsdfsi 1.42\");\n\
\n\
  if (eqsf2 (1, 1) == 0) fail (\"eqsf2 1==1\");\n\
  if (eqsf2 (1, 2) != 0) fail (\"eqsf2 1==2\");\n\
  if (nesf2 (1, 2) == 0) fail (\"nesf2 1!=1\");\n\
  if (nesf2 (1, 1) != 0) fail (\"nesf2 1!=1\");\n\
  if (gtsf2 (2, 1) == 0) fail (\"gtsf2 2>1\");\n\
  if (gtsf2 (1, 1) != 0) fail (\"gtsf2 1>1\");\n\
  if (gtsf2 (0, 1) != 0) fail (\"gtsf2 0>1\");\n\
  if (gesf2 (2, 1) == 0) fail (\"gesf2 2>=1\");\n\
  if (gesf2 (1, 1) == 0) fail (\"gesf2 1>=1\");\n\
  if (gesf2 (0, 1) != 0) fail (\"gesf2 0>=1\");\n\
  if (ltsf2 (1, 2) == 0) fail (\"ltsf2 1<2\");\n\
  if (ltsf2 (1, 1) != 0) fail (\"ltsf2 1<1\");\n\
  if (ltsf2 (1, 0) != 0) fail (\"ltsf2 1<0\");\n\
  if (lesf2 (1, 2) == 0) fail (\"lesf2 1<=2\");\n\
  if (lesf2 (1, 1) == 0) fail (\"lesf2 1<=1\");\n\
  if (lesf2 (1, 0) != 0) fail (\"lesf2 1<=0\");\n\
\n\
  if (fail_count != 0)\n\
    abort ();\n\
  exit (0);\n\
}",
""};

    int i;
    cod_code gen_code[32];
    cod_parse_context context;
    for (i=0; i < 32; i++) {
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
        if (i == 31) {
            int (*func)() = (int(*)()) externs[i].extern_value;
            if (setjmp(env) == 0) {
                func();
            }
            if (exit_value != 0) {
                printf("Test ./generated/gofast.c failed\n");
                exit(exit_value);
            }
        } else {
            context = cod_copy_globals(context);
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp gofast.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/gofast.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/gofast.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/gofast.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/gofast.c Succeeded\n");
    return 0;
}
