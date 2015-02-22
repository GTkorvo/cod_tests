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
	test_output = fopen("20000314-3.c.output", "w");
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
	{"attr_rtx", (void*)(long)-1},
	{"attr_string", (void*)(long)-1},
	{"attr_eq", (void*)(long)-1},
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	static void  attr_rtx (char *varg0, char *varg1);\n\
	static char * attr_string (char *str);\n\
	static void  attr_eq (char *value, char *name);\n\
	int main();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);";
    char *func_bodies[] = {

/* body for attr_rtx */
"{\n\
  if (varg0 != arg0)\n\
    abort ();\n\
\n\
  if (varg1 != arg1)\n\
    abort ();\n\
\n\
  return;\n\
}",

/* body for attr_string */
"{\n\
  return str;\n\
}",

/* body for attr_eq */
"{\n\
  return attr_rtx (attr_string (name),\n\
		   attr_string (value));\n\
}",

/* body for main */
"{\n\
  attr_eq (arg0, arg1);\n\
  exit (0);\n\
}",
""};

    char *func_decls[] = {
	"static void  attr_rtx (char *varg0, char *varg1);",
	"static char * attr_string (char *str);",
	"static void  attr_eq (char *value, char *name);",
	"int main();",
	""};

    char *global_decls[] = {
	"\n\
\n\
static char arg0[] = \"arg0\";\n\
static char arg1[] = \"arg1\";",
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
                printf("Test ./generated/20000314-3.c failed\n");
                exit(exit_value);
            }
        } else {
            context = cod_copy_globals(context);
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp 20000314-3.c.output ./pre_patch/20000314-3.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/20000314-3.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/20000314-3.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/20000314-3.c Succeeded\n");
    return 0;
}
