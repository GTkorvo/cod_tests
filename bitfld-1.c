#include "cod.h"
#undef NDEBUG
#include "assert.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <setjmp.h>

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
	test_output = fopen("bitfld-1.c.output", "w");
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
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	int main(int argc, char *argv[]);\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);";
    char *func_bodies[] = {

/* body for main */
"{\n\
  struct x { signed int i : 7; unsigned int u : 7; } bit;\n\
\n\
  unsigned int u;\n\
  int i;\n\
  unsigned int unsigned_result = -13U % 61;\n\
  int signed_result = -13 % 61;\n\
\n\
  bit.u = 61, u = 61; \n\
  bit.i = -13, i = -13;\n\
\n\
  if (i % u != unsigned_result)\n\
    abort ();\n\
  if (i % (unsigned int) u != unsigned_result)\n\
    abort ();\n\
\n\
  /* Somewhat counter-intuitively, bit.u is promoted to an int, making\n\
     the operands and result an int.  */\n\
  if (i % bit.u != signed_result)\n\
    abort ();\n\
\n\
  if (bit.i % bit.u != signed_result)\n\
    abort ();\n\
\n\
  /* But with a cast to unsigned int, the unsigned int is promoted to\n\
     itself as a no-op, and the operands and result are unsigned.  */\n\
  if (i % (unsigned int) bit.u != unsigned_result)\n\
    abort ();\n\
\n\
  if (bit.i % (unsigned int) bit.u != unsigned_result)\n\
    abort ();\n\
\n\
  return 0;\n\
}",
""};

    char *func_decls[] = {
	"int main(int argc, char *argv[]);",
	""};

    char *global_decls[] = {
	"/* Copyright 2002 Free Software Foundation, Inc.\n\
\n\
   Tests correct signedness of operations on bitfields; in particular\n\
   that integer promotions are done correctly, including the case when\n\
   casts are present.\n\
\n\
   The C front end was eliding the cast of an unsigned bitfield to\n\
   unsigned as a no-op, when in fact it forces a conversion to a\n\
   full-width unsigned int. (At the time of writing, the C++ front end\n\
   has a different bug; it erroneously promotes the uncast unsigned\n\
   bitfield to an unsigned int).\n\
\n\
   Source: Neil Booth, 25 Jan 2002, based on PR 3325 (and 3326, which\n\
   is a different manifestation of the same bug).\n\
\n\
\n\
extern void abort ();",
""};

    int i;
    cod_code gen_code[1];
    for (i=0; i < 1; i++) {
        int j;
        if (verbose) {
             printf("Working on subroutine %s\n", externs[i].extern_name);
        }
        cod_parse_context context = new_cod_parse_context();
        cod_assoc_externs(context, externs);
        for (j=0; j < 1; j++) {
            cod_parse_for_globals(global_decls[j], context);
        }
        cod_parse_for_context(extern_string, context);
        cod_subroutine_declaration(func_decls[i], context);
        gen_code[i] = cod_code_gen(func_bodies[i], context);
        externs[i].extern_value = (void*) gen_code[i]->func;
        if (i == 0) {
            int (*func)() = (int(*)()) externs[i].extern_value;
            if (setjmp(env) == 0) {
                func();
            }
            if (exit_value != 0) {
                printf("Test ./generated/bitfld-1.c failed\n");
                exit(exit_value);
            }
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp bitfld-1.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/bitfld-1.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/bitfld-1.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/bitfld-1.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/bitfld-1.c Succeeded\n");
    return 0;
}
