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
	test_output = fopen("10_pointer.c.output", "w");
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
	int main();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);";
    char *func_bodies[] = {

/* body for main */
"{\n\
   int a;\n\
   int *b;\n\
   int c;\n\
\n\
   a = 42;\n\
   b = &a;\n\
   test_printf(\"a = %d\\n\", *b);\n\
\n\
   bolshevic.a = 12;\n\
   bolshevic.b = 34;\n\
   bolshevic.c = 56;\n\
\n\
   test_printf(\"bolshevic.a = %d\\n\", bolshevic.a);\n\
   test_printf(\"bolshevic.b = %d\\n\", bolshevic.b);\n\
   test_printf(\"bolshevic.c = %d\\n\", bolshevic.c);\n\
\n\
   struct ziggy *tsar = &bolshevic;\n\
\n\
   test_printf(\"tsar->a = %d\\n\", tsar->a);\n\
   test_printf(\"tsar->b = %d\\n\", tsar->b);\n\
   test_printf(\"tsar->c = %d\\n\", tsar->c);\n\
\n\
   b = &(bolshevic.b);\n\
   test_printf(\"bolshevic.b = %d\\n\", *b);\n\
\n\
   return 0;\n\
}",
""};

    char *func_decls[] = {
	"int main();",
	""};

    char *global_decls[] = {
	"#include <stdio.h>",
	"struct ziggy\n\
{\n\
   int a;\n\
   int b;\n\
   int c;\n\
} bolshevic;",
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
        for (j=0; j < 2; j++) {
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
                printf("Test ./generated/10_pointer.c failed\n");
                exit(exit_value);
            }
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp 10_pointer.c.output /Users/eisen/prog/tinycc/tests/tests2/10_pointer.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/10_pointer.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/10_pointer.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/10_pointer.c Succeeded\n");
    return 0;
}