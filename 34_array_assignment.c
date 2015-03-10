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
// #include <stdio.h>
// 
// int main()
// {
//    int a[4];
// 
//    a[0] = 12;
//    a[1] = 23;
//    a[2] = 34;
//    a[3] = 45;
// 
//    printf("%d %d %d %d\n", a[0], a[1], a[2], a[3]);
// 
//    int b[4];
// 
//    b = a;
// 
//    printf("%d %d %d %d\n", b[0], b[1], b[2], b[3]);
// 
//    return 0;
// }
// 
// /* vim: set expandtab ts=4 sw=3 sts=3 tw=80 :*/

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
	test_output = fopen("34_array_assignment.c.output", "w");
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
	int main();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);\n\
        int printf(const char *format, ...);";
    char *func_bodies[] = {

/* body for main */
"{\n\
   int a[4];\n\
\n\
   a[0] = 12;\n\
   a[1] = 23;\n\
   a[2] = 34;\n\
   a[3] = 45;\n\
\n\
   test_printf(\"%d %d %d %d\\n\", a[0], a[1], a[2], a[3]);\n\
\n\
   int b[4];\n\
\n\
   b = a;\n\
\n\
   test_printf(\"%d %d %d %d\\n\", b[0], b[1], b[2], b[3]);\n\
\n\
   return 0;\n\
}",
""};

    char *func_decls[] = {
	"int main();",
	""};

    char *global_decls[] = {
	"#include <stdio.h>",
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
                printf("Test ./generated/34_array_assignment.c failed\n");
                exit(exit_value);
            }
        } else {
            context = cod_copy_globals(context);
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp 34_array_assignment.c.output /Users/eisen/prog/tinycc/tests/tests2/34_array_assignment.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/34_array_assignment.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/34_array_assignment.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/34_array_assignment.c Succeeded\n");
    return 0;
}
