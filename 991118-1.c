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
// struct tmp
// {
//   long long int pad : 12;
//   long long int field : 52;
// };
// 
// struct tmp2
// {
//   long long int field : 52;
//   long long int pad : 12;
// };
// 
// struct tmp3
// {
//   long long int pad : 11;
//   long long int field : 53;
// };
// 
// struct tmp4
// {
//   long long int field : 53;
//   long long int pad : 11;
// };
// 
// struct tmp
// sub (struct tmp tmp)
// {
//   tmp.field ^= 0x0008765412345678LL;
//   return tmp;
// }
// 
// struct tmp2
// sub2 (struct tmp2 tmp2)
// {
//   tmp2.field ^= 0x0008765412345678LL;
//   return tmp2;
// }
// 
// struct tmp3
// sub3 (struct tmp3 tmp3)
// {
//   tmp3.field ^= 0x0018765412345678LL;
//   return tmp3;
// }
// 
// struct tmp4
// sub4 (struct tmp4 tmp4)
// {
//   tmp4.field ^= 0x0018765412345678LL;
//   return tmp4;
// }
// 
// struct tmp tmp = {0x123, 0x123456789ABCDLL};
// struct tmp2 tmp2 = {0x123456789ABCDLL, 0x123};
// struct tmp3 tmp3 = {0x123, 0x1FFFF00000000LL};
// struct tmp4 tmp4 = {0x1FFFF00000000LL, 0x123};
// 
// main()
// {
// 
//   if (sizeof (long long) != 8)
//     exit (0);
// 
//   tmp = sub (tmp);
//   tmp2 = sub2 (tmp2);
// 
//   if (tmp.pad != 0x123 || tmp.field != 0xFFF9551175BDFDB5LL)
//     abort ();
//   if (tmp2.pad != 0x123 || tmp2.field != 0xFFF9551175BDFDB5LL)
//     abort ();
// 
//   tmp3 = sub3 (tmp3);
//   tmp4 = sub4 (tmp4);
//   if (tmp3.pad != 0x123 || tmp3.field != 0xFFF989AB12345678LL)
//     abort ();
//   if (tmp4.pad != 0x123 || tmp4.field != 0xFFF989AB12345678LL)
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
	test_output = fopen("991118-1.c.output", "w");
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
	{"sub", (void*)(long)-1},
	{"sub2", (void*)(long)-1},
	{"sub3", (void*)(long)-1},
	{"sub4", (void*)(long)-1},
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{"printf", (void*)printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	struct tmp sub (struct tmp tmp);\n\
	struct tmp2 sub2 (struct tmp2 tmp2);\n\
	struct tmp3 sub3 (struct tmp3 tmp3);\n\
	struct tmp4 sub4 (struct tmp4 tmp4);\n\
	void main();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);\n\
        int printf(const char *format, ...);";
    char *func_bodies[] = {

/* body for sub */
"{\n\
  tmp.field ^= 0x0008765412345678LL;\n\
  return tmp;\n\
}",

/* body for sub2 */
"{\n\
  tmp2.field ^= 0x0008765412345678LL;\n\
  return tmp2;\n\
}",

/* body for sub3 */
"{\n\
  tmp3.field ^= 0x0018765412345678LL;\n\
  return tmp3;\n\
}",

/* body for sub4 */
"{\n\
  tmp4.field ^= 0x0018765412345678LL;\n\
  return tmp4;\n\
}",

/* body for main */
"{\n\
\n\
  if (sizeof (long long) != 8)\n\
    exit (0);\n\
\n\
  tmp = sub (tmp);\n\
  tmp2 = sub2 (tmp2);\n\
\n\
  if (tmp.pad != 0x123 || tmp.field != 0xFFF9551175BDFDB5LL)\n\
    abort ();\n\
  if (tmp2.pad != 0x123 || tmp2.field != 0xFFF9551175BDFDB5LL)\n\
    abort ();\n\
\n\
  tmp3 = sub3 (tmp3);\n\
  tmp4 = sub4 (tmp4);\n\
  if (tmp3.pad != 0x123 || tmp3.field != 0xFFF989AB12345678LL)\n\
    abort ();\n\
  if (tmp4.pad != 0x123 || tmp4.field != 0xFFF989AB12345678LL)\n\
    abort ();\n\
  exit (0);\n\
}",
""};

    char *func_decls[] = {
	"struct tmp sub (struct tmp tmp);",
	"struct tmp2 sub2 (struct tmp2 tmp2);",
	"struct tmp3 sub3 (struct tmp3 tmp3);",
	"struct tmp4 sub4 (struct tmp4 tmp4);",
	"void main();",
	""};

    char *global_decls[] = {
	"struct tmp\n\
{\n\
  long long int pad : 12;\n\
  long long int field : 52;\n\
};",
	"struct tmp2\n\
{\n\
  long long int field : 52;\n\
  long long int pad : 12;\n\
};",
	"struct tmp3\n\
{\n\
  long long int pad : 11;\n\
  long long int field : 53;\n\
};",
	"struct tmp4\n\
{\n\
  long long int field : 53;\n\
  long long int pad : 11;\n\
};",
	"struct tmp tmp ={0x123, 0x123456789ABCDLL};",
	"struct tmp2 tmp2 ={0x123456789ABCDLL, 0x123};",
	"struct tmp3 tmp3 ={0x123, 0x1FFFF00000000LL};",
	"struct tmp4 tmp4 ={0x1FFFF00000000LL, 0x123};",
""};

    int i;
    cod_code gen_code[5];
    cod_parse_context context;
    for (i=0; i < 5; i++) {
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
        if (i == 4) {
            int (*func)() = (int(*)()) externs[i].extern_value;
            if (setjmp(env) == 0) {
                func();
            }
            if (exit_value != 0) {
                printf("Test ./generated/991118-1.c failed\n");
                exit(exit_value);
            }
        } else {
            context = cod_copy_globals(context);
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp 991118-1.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/991118-1.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/991118-1.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/991118-1.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/991118-1.c Succeeded\n");
    return 0;
}
