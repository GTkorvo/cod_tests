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
// /* Source: PR 321 modified for test suite by Neil Booth 14 Jan 2001.  */
// 
// typedef unsigned long long uint64;
// unsigned long pars;
// 
// uint64 b[32];
// uint64 *r = b;
// 
// void alpha_ep_extbl_i_eq_0()
// {
//   unsigned int rb, ra, rc;
// 
//   rb  = (((unsigned long)(pars) >> 27)) & 0x1fUL;
//   ra  = (((unsigned int)(pars) >> 5)) & 0x1fUL;
//   rc  = (((unsigned int)(pars) >> 0)) & 0x1fUL;
//   {
//     uint64 temp = ((r[ra] >> ((r[rb] & 0x7) << 3)) & 0x00000000000000FFLL); 
//     if (rc != 31) 
//       r[rc] = temp;  
//   }
// }
// 
// int 
// main(void)
// {
//   if (sizeof (uint64) == 8)
//     {
//       b[17] = 0x0000000000303882ULL; /* rb */
//       b[2] = 0x534f4f4c494d000aULL; /* ra & rc */
// 
//       pars = 0x88000042;	/* 17, 2, 2 coded */
//       alpha_ep_extbl_i_eq_0();
// 
//       if (b[2] != 0x4d)
// 	abort ();
//     }
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
	test_output = fopen("longlong.c.output", "w");
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
	{"alpha_ep_extbl_i_eq_0", (void*)(long)-1},
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{"printf", (void*)printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	void alpha_ep_extbl_i_eq_0();\n\
	int  main();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);\n\
        int printf(const char *format, ...);";
    char *global_decls[] = {
	"typedef unsigned long long uint64;\n\
unsigned long pars;\n\
\n\
uint64 b[32];\n\
uint64 *r = b;",
""};

    char *func_decls[] = {
	"void alpha_ep_extbl_i_eq_0();",
	"int  main();",
	""};

    char *func_bodies[] = {

/* body for alpha_ep_extbl_i_eq_0 */
"{\n\
  unsigned int rb, ra, rc;\n\
\n\
  rb  = (((unsigned long)(pars) >> 27)) & 0x1fUL;\n\
  ra  = (((unsigned int)(pars) >> 5)) & 0x1fUL;\n\
  rc  = (((unsigned int)(pars) >> 0)) & 0x1fUL;\n\
  {\n\
    uint64 temp = ((r[ra] >> ((r[rb] & 0x7) << 3)) & 0x00000000000000FFLL); \n\
    if (rc != 31) \n\
      r[rc] = temp;  \n\
  }\n\
}",

/* body for main */
"{\n\
  if (sizeof (uint64) == 8)\n\
    {\n\
      b[17] = 0x0000000000303882ULL; /* rb */\n\
      b[2] = 0x534f4f4c494d000aULL; /* ra & rc */\n\
\n\
      pars = 0x88000042;	/* 17, 2, 2 coded */\n\
      alpha_ep_extbl_i_eq_0();\n\
\n\
      if (b[2] != 0x4d)\n\
	abort ();\n\
    }\n\
\n\
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
                printf("Test ./generated/longlong.c failed\n");
                exit(exit_value);
            }
        } else {
            context = cod_copy_globals(context);
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp longlong.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/longlong.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/longlong.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/longlong.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/longlong.c Succeeded\n");
    return 0;
}
