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
// long
// f (long x)
// {
//   return x / (-0x7fffffffL - 1L);
// }
// 
// long
// r (long x)
// {
//   return x % (-0x7fffffffL - 1L);
// }
// 
// /* Since we have a negative divisor, this equation must hold for the
//    results of / and %; no specific results are guaranteed.  */
// long
// std_eqn (long num, long denom, long quot, long rem)
// {
//   /* For completeness, a check for "ABS (rem) < ABS (denom)" belongs here,
//      but causes trouble on 32-bit machines and isn't worthwhile.  */
//   return quot * (-0x7fffffffL - 1L) + rem == num;
// }
// 
// long nums[] =
// {
//   -1L, 0x7fffffffL, -0x7fffffffL - 1L
// };
// 
// main ()
// {
//   int i;
// 
//   for (i = 0;
//        i < sizeof (nums) / sizeof (nums[0]);
//        i++)
//     if (std_eqn (nums[i], -0x7fffffffL - 1L, f (nums[i]), r (nums[i])) == 0)
//       abort ();
// 
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
	test_output = fopen("divconst-2.c.output", "w");
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
	{"r", (void*)(long)-1},
	{"std_eqn", (void*)(long)-1},
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{"printf", (void*)printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	long f (long x);\n\
	long r (long x);\n\
	long std_eqn (long num, long denom, long quot, long rem);\n\
	void main ();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);\n\
        int printf(const char *format, ...);";
    char *func_bodies[] = {

/* body for f */
"{\n\
  return x / (-0x7fffffffL - 1L);\n\
}",

/* body for r */
"{\n\
  return x % (-0x7fffffffL - 1L);\n\
}",

/* body for std_eqn */
"{\n\
  /* For completeness, a check for \"ABS (rem) < ABS (denom)\" belongs here,\n\
     but causes trouble on 32-bit machines and isn't worthwhile.  */\n\
  return quot * (-0x7fffffffL - 1L) + rem == num;\n\
}",

/* body for main */
"{\n\
  int i;\n\
\n\
  for (i = 0;\n\
       i < sizeof (nums) / sizeof (nums[0]);\n\
       i++)\n\
    if (std_eqn (nums[i], -0x7fffffffL - 1L, f (nums[i]), r (nums[i])) == 0)\n\
      abort ();\n\
\n\
  exit (0);\n\
}",
""};

    char *func_decls[] = {
	"long f (long x);",
	"long r (long x);",
	"long std_eqn (long num, long denom, long quot, long rem);",
	"void main ();",
	""};

    char *global_decls[] = {
	"long nums[] ={\n\
  -1L, 0x7fffffffL, -0x7fffffffL - 1L\n\
};",
""};

    int i;
    cod_code gen_code[4];
    cod_parse_context context;
    for (i=0; i < 4; i++) {
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
        if (i == 3) {
            int (*func)() = (int(*)()) externs[i].extern_value;
            if (setjmp(env) == 0) {
                func();
            }
            if (exit_value != 0) {
                printf("Test ./generated/divconst-2.c failed\n");
                exit(exit_value);
            }
        } else {
            context = cod_copy_globals(context);
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp divconst-2.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/divconst-2.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/divconst-2.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/divconst-2.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/divconst-2.c Succeeded\n");
    return 0;
}
