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
// /* On H8/300 port, NOTICE_UPDATE_CC had a bug that causes the final
//    pass to remove test insns that should be kept.  */
// 
// unsigned short
// test1 (unsigned short w)
// {
//   if ((w & 0xff00) == 0)
//     {
//       if (w == 0)
// 	w = 2;
//     }
//   return w;
// }
// 
// unsigned long
// test2 (unsigned long w)
// {
//   if ((w & 0xffff0000) == 0)
//     {
//       if (w == 0)
// 	w = 2;
//     }
//   return w;
// }
// 
// int
// test3 (unsigned short a)
// {
//   if (a & 1)
//     return 1;
//   else if (a)
//     return 1;
//   else
//     return 0;
// }
// 
// int
// main ()
// {
//   if (test1 (1) != 1)
//     abort ();
// 
//   if (test2 (1) != 1)
//     abort ();
// 
//   if (test3 (2) != 1)
//     abort ();
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
	test_output = fopen("20030120-1.c.output", "w");
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
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{"printf", (void*)printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	unsigned short test1 (unsigned short w);\n\
	unsigned long test2 (unsigned long w);\n\
	int test3 (unsigned short a);\n\
	int main ();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);\n\
        int printf(const char *format, ...);";
    char *global_decls[] = {
""};

    char *func_decls[] = {
	"unsigned short test1 (unsigned short w);",
	"unsigned long test2 (unsigned long w);",
	"int test3 (unsigned short a);",
	"int main ();",
	""};

    char *func_bodies[] = {

/* body for test1 */
"{\n\
  if ((w & 0xff00) == 0)\n\
    {\n\
      if (w == 0)\n\
	w = 2;\n\
    }\n\
  return w;\n\
}",

/* body for test2 */
"{\n\
  if ((w & 0xffff0000) == 0)\n\
    {\n\
      if (w == 0)\n\
	w = 2;\n\
    }\n\
  return w;\n\
}",

/* body for test3 */
"{\n\
  if (a & 1)\n\
    return 1;\n\
  else if (a)\n\
    return 1;\n\
  else\n\
    return 0;\n\
}",

/* body for main */
"{\n\
  if (test1 (1) != 1)\n\
    abort ();\n\
\n\
  if (test2 (1) != 1)\n\
    abort ();\n\
\n\
  if (test3 (2) != 1)\n\
    abort ();\n\
\n\
  exit (0);\n\
}",
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
                printf("Test ./generated/20030120-1.c failed\n");
                exit(exit_value);
            }
        } else {
            context = cod_copy_globals(context);
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp 20030120-1.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/20030120-1.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/20030120-1.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/20030120-1.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/20030120-1.c Succeeded\n");
    return 0;
}
