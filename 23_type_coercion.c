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
// void charfunc(char a)
// {
//    printf("char: %c\n", a);
// }
// 
// void intfunc(int a)
// {
//    printf("int: %d\n", a);
// }
// 
// void floatfunc(float a)
// {
//    printf("float: %f\n", a);
// }
// 
// int main()
// {
//    charfunc('a');
//    charfunc(98);
//    charfunc(99.0);
// 
//    intfunc('a');
//    intfunc(98);
//    intfunc(99.0);
// 
//    floatfunc('a');
//    floatfunc(98);
//    floatfunc(99.0);
// 
//    /* printf("%c %d %f\n", 'a', 'b', 'c'); */
//    /* printf("%c %d %f\n", 97, 98, 99); */
//    /* printf("%c %d %f\n", 97.0, 98.0, 99.0); */
// 
//    char b = 97;
//    char c = 97.0;
// 
//    printf("%d %d\n", b, c);
// 
//    int d = 'a';
//    int e = 97.0;
// 
//    printf("%d %d\n", d, e);
// 
//    float f = 'a';
//    float g = 97;
// 
//    printf("%f %f\n", f, g);
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
	test_output = fopen("23_type_coercion.c.output", "w");
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
	{"charfunc", (void*)(long)-1},
	{"intfunc", (void*)(long)-1},
	{"floatfunc", (void*)(long)-1},
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{"printf", (void*)printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	void charfunc(char a);\n\
	void intfunc(int a);\n\
	void floatfunc(float a);\n\
	int main();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);\n\
        int printf(const char *format, ...);";
    char *func_bodies[] = {

/* body for charfunc */
"{\n\
   test_printf(\"char: %c\\n\", a);\n\
}",

/* body for intfunc */
"{\n\
   test_printf(\"int: %d\\n\", a);\n\
}",

/* body for floatfunc */
"{\n\
   test_printf(\"float: %f\\n\", a);\n\
}",

/* body for main */
"{\n\
   charfunc('a');\n\
   charfunc(98);\n\
   charfunc(99.0);\n\
\n\
   intfunc('a');\n\
   intfunc(98);\n\
   intfunc(99.0);\n\
\n\
   floatfunc('a');\n\
   floatfunc(98);\n\
   floatfunc(99.0);\n\
\n\
   /* test_printf(\"%c %d %f\\n\", 'a', 'b', 'c'); */\n\
   /* test_printf(\"%c %d %f\\n\", 97, 98, 99); */\n\
   /* test_printf(\"%c %d %f\\n\", 97.0, 98.0, 99.0); */\n\
\n\
   char b = 97;\n\
   char c = 97.0;\n\
\n\
   test_printf(\"%d %d\\n\", b, c);\n\
\n\
   int d = 'a';\n\
   int e = 97.0;\n\
\n\
   test_printf(\"%d %d\\n\", d, e);\n\
\n\
   float f = 'a';\n\
   float g = 97;\n\
\n\
   test_printf(\"%f %f\\n\", f, g);\n\
\n\
   return 0;\n\
}",
""};

    char *func_decls[] = {
	"void charfunc(char a);",
	"void intfunc(int a);",
	"void floatfunc(float a);",
	"int main();",
	""};

    char *global_decls[] = {
	"#include <stdio.h>",
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
                printf("Test ./generated/23_type_coercion.c failed\n");
                exit(exit_value);
            }
        } else {
            context = cod_copy_globals(context);
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp 23_type_coercion.c.output /Users/eisen/prog/tinycc/tests/tests2/23_type_coercion.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/23_type_coercion.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/23_type_coercion.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/23_type_coercion.c Succeeded\n");
    return 0;
}
