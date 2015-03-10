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
// void fail1(void)
// {
//   abort ();
// }
// void fail2(void)
// {
//   abort ();
// }
// void fail3(void)
// {
//   abort ();
// }
// void fail4(void)
// {
//   abort ();
// }
// 
// 
// void foo(long x)
// {
//   switch (x)
//     {
//     case -6: 
//       fail1 (); break;
//     case 0: 
//       fail2 (); break;
//     case 1: case 2: 
//       break;
//     case 3: case 4: case 5: 
//       fail3 ();
//       break;
//     default:
//       fail4 ();
//       break;
//     }
//   switch (x)
//     {
//       
//     case -3: 
//       fail1 (); break;
//     case 0: case 4: 
//       fail2 (); break;
//     case 1: case 3: 
//       break;
//     case 2: case 8: 
//       abort ();
//       break;
//     default:
//       fail4 ();
//       break;
//     }
// }
// 
// int main(void)
// {
//   foo (1);
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
	test_output = fopen("20011109-1.c.output", "w");
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
	{"fail1", (void*)(long)-1},
	{"fail2", (void*)(long)-1},
	{"fail3", (void*)(long)-1},
	{"fail4", (void*)(long)-1},
	{"foo", (void*)(long)-1},
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{"printf", (void*)printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	void fail1();\n\
	void fail2();\n\
	void fail3();\n\
	void fail4();\n\
	void foo(long x);\n\
	int main();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);\n\
        int printf(const char *format, ...);";
    char *func_bodies[] = {

/* body for fail1 */
"{\n\
  abort ();\n\
}",

/* body for fail2 */
"{\n\
  abort ();\n\
}",

/* body for fail3 */
"{\n\
  abort ();\n\
}",

/* body for fail4 */
"{\n\
  abort ();\n\
}",

/* body for foo */
"{\n\
  switch (x)\n\
    {\n\
    case -6: \n\
      fail1 (); break;\n\
    case 0: \n\
      fail2 (); break;\n\
    case 1: case 2: \n\
      break;\n\
    case 3: case 4: case 5: \n\
      fail3 ();\n\
      break;\n\
    default:\n\
      fail4 ();\n\
      break;\n\
    }\n\
  switch (x)\n\
    {\n\
      \n\
    case -3: \n\
      fail1 (); break;\n\
    case 0: case 4: \n\
      fail2 (); break;\n\
    case 1: case 3: \n\
      break;\n\
    case 2: case 8: \n\
      abort ();\n\
      break;\n\
    default:\n\
      fail4 ();\n\
      break;\n\
    }\n\
}",

/* body for main */
"{\n\
  foo (1);\n\
  exit (0);\n\
}",
""};

    char *func_decls[] = {
	"void fail1();",
	"void fail2();",
	"void fail3();",
	"void fail4();",
	"void foo(long x);",
	"int main();",
	""};

    char *global_decls[] = {
""};

    int i;
    cod_code gen_code[6];
    cod_parse_context context;
    for (i=0; i < 6; i++) {
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
        if (i == 5) {
            int (*func)() = (int(*)()) externs[i].extern_value;
            if (setjmp(env) == 0) {
                func();
            }
            if (exit_value != 0) {
                printf("Test ./generated/20011109-1.c failed\n");
                exit(exit_value);
            }
        } else {
            context = cod_copy_globals(context);
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp 20011109-1.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/20011109-1.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/20011109-1.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/20011109-1.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/20011109-1.c Succeeded\n");
    return 0;
}
