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
	test_output = fopen("align-2.c.output", "w");
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
	{"printf", (void*)printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	int main ();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);\n\
        int printf(const char *format, ...);";
    char *func_bodies[] = {

/* body for main */
"{\n\
  if (s_c_s.c != 'a') abort ();\n\
  if (s_c_s.s != 13) abort ();\n\
  if (s_c_i.c != 'b') abort ();\n\
  if (s_c_i.i != 14) abort ();\n\
  if (s_s_i.s != 15) abort ();\n\
  if (s_s_i.i != 16) abort ();\n\
  if (s_c_f.c != 'c') abort ();\n\
  if (s_c_f.f != 17.0) abort ();\n\
  if (s_s_f.s != 18) abort ();\n\
  if (s_s_f.f != 19.0) abort ();\n\
  if (s_c_d.c != 'd') abort ();\n\
  if (s_c_d.d != 20.0) abort ();\n\
  if (s_s_d.s != 21) abort ();\n\
  if (s_s_d.d != 22.0) abort ();\n\
  if (s_i_d.i != 23) abort ();\n\
  if (s_i_d.d != 24.0) abort ();\n\
  if (s_f_d.f != 25.0) abort ();\n\
  if (s_f_d.d != 26.0) abort ();\n\
  if (s_c_ld.c != 'e') abort ();\n\
  if (s_c_ld.ld != 27.0) abort ();\n\
  if (s_s_ld.s != 28) abort ();\n\
  if (s_s_ld.ld != 29.0) abort ();\n\
  if (s_i_ld.i != 30) abort ();\n\
  if (s_i_ld.ld != 31.0) abort ();\n\
  if (s_f_ld.f != 32.0) abort ();\n\
  if (s_f_ld.ld != 33.0) abort ();\n\
  if (s_d_ld.d != 34.0) abort ();\n\
  if (s_d_ld.ld != 35.0) abort ();\n\
  return 0;\n\
}",
""};

    char *func_decls[] = {
	"int main ();",
	""};

    char *global_decls[] = {
	"struct a_short { char c; short s; }",
	"s_c_s ={ 'a', 13 };",
	"struct a_int { char c ; int i; }",
	"s_c_i ={ 'b', 14 };",
	"struct b_int { short s; int i; }",
	"s_s_i  ={ 15, 16 };",
	"struct a_float { char c; float f; }",
	"s_c_f ={ 'c', 17.0 };",
	"struct b_float { short s; float f; }",
	"s_s_f ={ 18, 19.0 };",
	"struct a_double { char c; double d; }",
	"s_c_d ={ 'd', 20.0 };",
	"struct b_double { short s; double d; }",
	"s_s_d ={ 21, 22.0 };",
	"struct c_double { int i; double d; }",
	"s_i_d ={ 23, 24.0 };",
	"struct d_double { float f; double d; }",
	"s_f_d ={ 25.0, 26.0 };",
	"struct a_ldouble { char c; long double ld; }",
	"s_c_ld ={ 'e', 27.0 };",
	"struct b_ldouble { short s; long double ld; }",
	"s_s_ld ={ 28, 29.0 };",
	"struct c_ldouble { int i; long double ld; }",
	"s_i_ld ={ 30, 31.0 };",
	"struct d_ldouble { float f; long double ld; }",
	"s_f_ld ={ 32.0, 33.0 };",
	"struct e_ldouble { double d; long double ld; }",
	"s_d_ld ={ 34.0, 35.0 };",
""};

    int i;
    cod_code gen_code[1];
    cod_parse_context context;
    for (i=0; i < 1; i++) {
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
        if (i == 0) {
            int (*func)() = (int(*)()) externs[i].extern_value;
            if (setjmp(env) == 0) {
                func();
            }
            if (exit_value != 0) {
                printf("Test ./generated/align-2.c failed\n");
                exit(exit_value);
            }
        } else {
            context = cod_copy_globals(context);
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp align-2.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/align-2.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/align-2.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/align-2.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/align-2.c Succeeded\n");
    return 0;
}
