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
	test_output = fopen("builtin-prefetch-2.c.output", "w");
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
	{"simple_global", (void*)(long)-1},
	{"simple_file", (void*)(long)-1},
	{"simple_static_local", (void*)(long)-1},
	{"simple_local", (void*)(long)-1},
	{"simple_arg", (void*)(long)-1},
	{"expr_global", (void*)(long)-1},
	{"expr_local", (void*)(long)-1},
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	void simple_global ();\n\
	void simple_file ();\n\
	void simple_static_local ();\n\
	void simple_local ();\n\
	void simple_arg (int g[100], int *h, int i);\n\
	void expr_global ();\n\
	void expr_local ();\n\
	int main ();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);";
    char *func_bodies[] = {

/* body for simple_global */
"{\n\
  __builtin_prefetch (glob_int_arr, 0, 0);\n\
  __builtin_prefetch (glob_ptr_int, 0, 0);\n\
  __builtin_prefetch (&glob_int, 0, 0);\n\
}",

/* body for simple_file */
"{\n\
  __builtin_prefetch (stat_int_arr, 0, 0);\n\
  __builtin_prefetch (stat_ptr_int, 0, 0);\n\
  __builtin_prefetch (&stat_int, 0, 0);\n\
}",

/* body for simple_static_local */
"{\n\
  static int gx[100];\n\
  static int *hx = gx;\n\
  static int ix;\n\
  __builtin_prefetch (gx, 0, 0);\n\
  __builtin_prefetch (hx, 0, 0);\n\
  __builtin_prefetch (&ix, 0, 0);\n\
}",

/* body for simple_local */
"{\n\
  int gx[100];\n\
  int *hx = gx;\n\
  int ix;\n\
  __builtin_prefetch (gx, 0, 0);\n\
  __builtin_prefetch (hx, 0, 0);\n\
  __builtin_prefetch (&ix, 0, 0);\n\
}",

/* body for simple_arg */
"{\n\
  __builtin_prefetch (g, 0, 0);\n\
  __builtin_prefetch (h, 0, 0);\n\
  __builtin_prefetch (&i, 0, 0);\n\
}",

/* body for expr_global */
"{\n\
  __builtin_prefetch (&str, 0, 0);\n\
  __builtin_prefetch (ptr_str, 0, 0);\n\
  __builtin_prefetch (&str.b, 0, 0);\n\
  __builtin_prefetch (&ptr_str->b, 0, 0);\n\
  __builtin_prefetch (&str.d, 0, 0);\n\
  __builtin_prefetch (&ptr_str->d, 0, 0);\n\
  __builtin_prefetch (str.next, 0, 0);\n\
  __builtin_prefetch (ptr_str->next, 0, 0);\n\
  __builtin_prefetch (str.next->d, 0, 0);\n\
  __builtin_prefetch (ptr_str->next->d, 0, 0);\n\
\n\
  __builtin_prefetch (&glob_int_arr, 0, 0);\n\
  __builtin_prefetch (glob_ptr_int, 0, 0);\n\
  __builtin_prefetch (&glob_int_arr[2], 0, 0);\n\
  __builtin_prefetch (&glob_ptr_int[3], 0, 0);\n\
  __builtin_prefetch (glob_int_arr+3, 0, 0);\n\
  __builtin_prefetch (glob_int_arr+glob_int, 0, 0);\n\
  __builtin_prefetch (glob_ptr_int+5, 0, 0);\n\
  __builtin_prefetch (glob_ptr_int+glob_int, 0, 0);\n\
}",

/* body for expr_local */
"{\n\
  int b[10];\n\
  int *pb = b;\n\
  struct S t;\n\
  struct S *pt = &t;\n\
  int j = 4;\n\
\n\
  __builtin_prefetch (&t, 0, 0);\n\
  __builtin_prefetch (pt, 0, 0);\n\
  __builtin_prefetch (&t.b, 0, 0);\n\
  __builtin_prefetch (&pt->b, 0, 0);\n\
  __builtin_prefetch (&t.d, 0, 0);\n\
  __builtin_prefetch (&pt->d, 0, 0);\n\
  __builtin_prefetch (t.next, 0, 0);\n\
  __builtin_prefetch (pt->next, 0, 0);\n\
  __builtin_prefetch (t.next->d, 0, 0);\n\
  __builtin_prefetch (pt->next->d, 0, 0);\n\
\n\
  __builtin_prefetch (&b, 0, 0);\n\
  __builtin_prefetch (pb, 0, 0);\n\
  __builtin_prefetch (&b[2], 0, 0);\n\
  __builtin_prefetch (&pb[3], 0, 0);\n\
  __builtin_prefetch (b+3, 0, 0);\n\
  __builtin_prefetch (b+j, 0, 0);\n\
  __builtin_prefetch (pb+5, 0, 0);\n\
  __builtin_prefetch (pb+j, 0, 0);\n\
}",

/* body for main */
"{\n\
  simple_global ();\n\
  simple_file ();\n\
  simple_static_local ();\n\
  simple_local ();\n\
  simple_arg (glob_int_arr, glob_ptr_int, glob_int);\n\
\n\
  str.next = &str;\n\
  expr_global ();\n\
  expr_local ();\n\
\n\
  exit (0);\n\
}",
""};

    char *func_decls[] = {
	"void simple_global ();",
	"void simple_file ();",
	"void simple_static_local ();",
	"void simple_local ();",
	"void simple_arg (int g[100], int *h, int i);",
	"void expr_global ();",
	"void expr_local ();",
	"int main ();",
	""};

    char *global_decls[] = {
	"int glob_int_arr[100];\n\
int *glob_ptr_int = glob_int_arr;\n\
int glob_int = 4;\n\
\n\
static stat_int_arr[100];\n\
static int *stat_ptr_int = stat_int_arr;\n\
static int stat_int;",
	"struct S {\n\
  int a;\n\
  short b, c;\n\
  char d[8];\n\
  struct S *next;\n\
};\n\
\n\
struct S str;\n\
struct S *ptr_str = &str;",
""};

    int i;
    cod_code gen_code[8];
    cod_parse_context context;
    for (i=0; i < 8; i++) {
        int j;
        if (verbose) {
             printf("Working on subroutine %s\n", externs[i].extern_name);
        }
        if (i==0) {
            context = new_cod_parse_context();
            cod_assoc_externs(context, externs);
            for (j=0; j < 2; j++) {
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
        if (i == 7) {
            int (*func)() = (int(*)()) externs[i].extern_value;
            if (setjmp(env) == 0) {
                func();
            }
            if (exit_value != 0) {
                printf("Test ./generated/builtin-prefetch-2.c failed\n");
                exit(exit_value);
            }
        } else {
            context = cod_copy_globals(context);
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp builtin-prefetch-2.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/builtin-prefetch-2.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/builtin-prefetch-2.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/builtin-prefetch-2.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/builtin-prefetch-2.c Succeeded\n");
    return 0;
}
