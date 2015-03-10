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
// long f1(long a){return a&0xff000000L;}
// long f2 (long a){return a&~0xff000000L;}
// long f3(long a){return a&0x000000ffL;}
// long f4(long a){return a&~0x000000ffL;}
// long f5(long a){return a&0x0000ffffL;}
// long f6(long a){return a&~0x0000ffffL;}
// 
// main ()
// {
//   long a = 0x89ABCDEF;
// 
//   if (f1(a)!=0x89000000L||
//       f2(a)!=0x00ABCDEFL||
//       f3(a)!=0x000000EFL||
//       f4(a)!=0x89ABCD00L||
//       f5(a)!=0x0000CDEFL||
//       f6(a)!=0x89AB0000L)
//     abort();
//   exit(0);
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
	test_output = fopen("900409-1.c.output", "w");
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
	{"f1", (void*)(long)-1},
	{"f2", (void*)(long)-1},
	{"f3", (void*)(long)-1},
	{"f4", (void*)(long)-1},
	{"f5", (void*)(long)-1},
	{"f6", (void*)(long)-1},
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{"printf", (void*)printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	long f1(long a);\n\
	long f2 (long a);\n\
	long f3(long a);\n\
	long f4(long a);\n\
	long f5(long a);\n\
	long f6(long a);\n\
	void main ();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);\n\
        int printf(const char *format, ...);";
    char *func_bodies[] = {

/* body for f1 */
"{return a&0xff000000L;}",

/* body for f2 */
"{return a&~0xff000000L;}",

/* body for f3 */
"{return a&0x000000ffL;}",

/* body for f4 */
"{return a&~0x000000ffL;}",

/* body for f5 */
"{return a&0x0000ffffL;}",

/* body for f6 */
"{return a&~0x0000ffffL;}",

/* body for main */
"{\n\
  long a = 0x89ABCDEF;\n\
\n\
  if (f1(a)!=0x89000000L||\n\
      f2(a)!=0x00ABCDEFL||\n\
      f3(a)!=0x000000EFL||\n\
      f4(a)!=0x89ABCD00L||\n\
      f5(a)!=0x0000CDEFL||\n\
      f6(a)!=0x89AB0000L)\n\
    abort();\n\
  exit(0);\n\
}",
""};

    char *func_decls[] = {
	"long f1(long a);",
	"long f2 (long a);",
	"long f3(long a);",
	"long f4(long a);",
	"long f5(long a);",
	"long f6(long a);",
	"void main ();",
	""};

    char *global_decls[] = {
""};

    int i;
    cod_code gen_code[7];
    cod_parse_context context;
    for (i=0; i < 7; i++) {
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
        if (i == 6) {
            int (*func)() = (int(*)()) externs[i].extern_value;
            if (setjmp(env) == 0) {
                func();
            }
            if (exit_value != 0) {
                printf("Test ./generated/900409-1.c failed\n");
                exit(exit_value);
            }
        } else {
            context = cod_copy_globals(context);
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp 900409-1.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/900409-1.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/900409-1.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/900409-1.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/900409-1.c Succeeded\n");
    return 0;
}
