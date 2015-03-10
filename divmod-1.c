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
// div1 (signed char x)
// {
//   return x / -1;
// }
// 
// div2 (signed short x)
// {
//   return x / -1;
// }
// 
// div3 (signed char x, signed char y)
// {
//   return x / y;
// }
// 
// div4 (signed short x, signed short y)
// {
//   return x / y;
// }
// 
// mod1 (signed char x)
// {
//   return x % -1;
// }
// 
// mod2 (signed short x)
// {
//   return x % -1;
// }
// 
// mod3 (signed char x, signed char y)
// {
//   return x % y;
// }
// 
// mod4 (signed short x, signed short y)
// {
//   return x % y;
// }
// 
// signed long
// mod5 (signed long x, signed long y)
// {
//   return x % y;
// }
//      
// unsigned long
// mod6 (unsigned long x, unsigned long y)
// {
//   return x % y;
// }
//      
// main ()
// {
//   if (div1 (-(1 << 7)) != 1 << 7)
//     abort ();
//   if (div2 (-(1 << 15)) != 1 << 15)
//     abort ();
//   if (div3 (-(1 << 7), -1) != 1 << 7)
//     abort ();
//   if (div4 (-(1 << 15), -1) != 1 << 15)
//     abort ();
//   if (mod1 (-(1 << 7)) != 0)
//     abort ();
//   if (mod2 (-(1 << 15)) != 0)
//     abort ();
//   if (mod3 (-(1 << 7), -1) != 0)
//     abort ();
//   if (mod4 (-(1 << 15), -1) != 0)
//     abort ();
//   if (mod5 (0x50000000, 2) != 0)
//     abort ();
//   if (mod6 (0x50000000, 2) != 0)
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
	test_output = fopen("divmod-1.c.output", "w");
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
	{"div1", (void*)(long)-1},
	{"div2", (void*)(long)-1},
	{"div3", (void*)(long)-1},
	{"div4", (void*)(long)-1},
	{"mod1", (void*)(long)-1},
	{"mod2", (void*)(long)-1},
	{"mod3", (void*)(long)-1},
	{"mod4", (void*)(long)-1},
	{"mod5", (void*)(long)-1},
	{"mod6", (void*)(long)-1},
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{"printf", (void*)printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	div1 (signed char x);\n\
	div2 (signed short x);\n\
	div3 (signed char x, signed char y);\n\
	div4 (signed short x, signed short y);\n\
	mod1 (signed char x);\n\
	mod2 (signed short x);\n\
	mod3 (signed char x, signed char y);\n\
	mod4 (signed short x, signed short y);\n\
	signed long mod5 (signed long x, signed long y);\n\
	unsigned long mod6 (unsigned long x, unsigned long y);\n\
	void main ();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);\n\
        int printf(const char *format, ...);";
    char *func_bodies[] = {

/* body for div1 */
"{\n\
  return x / -1;\n\
}",

/* body for div2 */
"{\n\
  return x / -1;\n\
}",

/* body for div3 */
"{\n\
  return x / y;\n\
}",

/* body for div4 */
"{\n\
  return x / y;\n\
}",

/* body for mod1 */
"{\n\
  return x % -1;\n\
}",

/* body for mod2 */
"{\n\
  return x % -1;\n\
}",

/* body for mod3 */
"{\n\
  return x % y;\n\
}",

/* body for mod4 */
"{\n\
  return x % y;\n\
}",

/* body for mod5 */
"{\n\
  return x % y;\n\
}",

/* body for mod6 */
"{\n\
  return x % y;\n\
}",

/* body for main */
"{\n\
  if (div1 (-(1 << 7)) != 1 << 7)\n\
    abort ();\n\
  if (div2 (-(1 << 15)) != 1 << 15)\n\
    abort ();\n\
  if (div3 (-(1 << 7), -1) != 1 << 7)\n\
    abort ();\n\
  if (div4 (-(1 << 15), -1) != 1 << 15)\n\
    abort ();\n\
  if (mod1 (-(1 << 7)) != 0)\n\
    abort ();\n\
  if (mod2 (-(1 << 15)) != 0)\n\
    abort ();\n\
  if (mod3 (-(1 << 7), -1) != 0)\n\
    abort ();\n\
  if (mod4 (-(1 << 15), -1) != 0)\n\
    abort ();\n\
  if (mod5 (0x50000000, 2) != 0)\n\
    abort ();\n\
  if (mod6 (0x50000000, 2) != 0)\n\
    abort ();\n\
  \n\
  exit (0);\n\
}",
""};

    char *func_decls[] = {
	"div1 (signed char x);",
	"div2 (signed short x);",
	"div3 (signed char x, signed char y);",
	"div4 (signed short x, signed short y);",
	"mod1 (signed char x);",
	"mod2 (signed short x);",
	"mod3 (signed char x, signed char y);",
	"mod4 (signed short x, signed short y);",
	"signed long mod5 (signed long x, signed long y);",
	"unsigned long mod6 (unsigned long x, unsigned long y);",
	"void main ();",
	""};

    char *global_decls[] = {
""};

    int i;
    cod_code gen_code[11];
    cod_parse_context context;
    for (i=0; i < 11; i++) {
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
        if (i == 10) {
            int (*func)() = (int(*)()) externs[i].extern_value;
            if (setjmp(env) == 0) {
                func();
            }
            if (exit_value != 0) {
                printf("Test ./generated/divmod-1.c failed\n");
                exit(exit_value);
            }
        } else {
            context = cod_copy_globals(context);
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp divmod-1.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/divmod-1.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/divmod-1.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/divmod-1.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/divmod-1.c Succeeded\n");
    return 0;
}
