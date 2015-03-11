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
// #include <stdlib.h>
// 
// struct {
//     long sqlcode;
// } sqlca;
// 
// 
// struct data_record {
//     int dummy;
//     int a[100];
// } *data_ptr, data_tmp;
// 
// 
// int
// num_records()
// {
//     return 1;
// }
// 
// 
// void
// fetch()
// {
//     static int fetch_count;
// 
//     memset(&data_tmp, 0x55, sizeof(data_tmp));
//     sqlca.sqlcode = (++fetch_count > 1 ? 100 : 0);
// }
// 
// 
// void
// load_data() {
//     struct data_record *p;
//     int num = num_records();
// 
//     data_ptr = malloc(num * sizeof(struct data_record));
//     memset(data_ptr, 0xaa, num * sizeof(struct data_record));
// 
//     fetch();
//     p = data_ptr;
//     while (sqlca.sqlcode == 0) {
//         *p++ = data_tmp;
//         fetch();
//     }
// }
// 
// 
// main()
// {
//     load_data();
//     if (sizeof (int) == 2 && data_ptr[0].dummy != 0x5555)
//       abort ();
//     else if (sizeof (int) > 2 && data_ptr[0].dummy != 0x55555555)
//       abort ();
//     exit (0);
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
	test_output = fopen("990628-1.c.output", "w");
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
	{"num_records", (void*)(long)-1},
	{"fetch", (void*)(long)-1},
	{"load_data", (void*)(long)-1},
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{"printf", (void*)printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	int num_records();\n\
	void fetch();\n\
	void load_data();\n\
	void main();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);\n\
        int printf(const char *format, ...);";
    char *global_decls[] = {
	"#include <stdlib.h>",
	"struct {\n\
    long sqlcode;\n\
} sqlca;",
	"struct data_record {\n\
    int dummy;\n\
    int a[100];\n\
} *data_ptr, data_tmp;",
""};

    char *func_decls[] = {
	"int num_records();",
	"void fetch();",
	"void load_data();",
	"void main();",
	""};

    char *func_bodies[] = {

/* body for num_records */
"{\n\
    return 1;\n\
}",

/* body for fetch */
"{\n\
    static int fetch_count;\n\
\n\
    memset(&data_tmp, 0x55, sizeof(data_tmp));\n\
    sqlca.sqlcode = (++fetch_count > 1 ? 100 : 0);\n\
}",

/* body for load_data */
"{\n\
    struct data_record *p;\n\
    int num = num_records();\n\
\n\
    data_ptr = malloc(num * sizeof(struct data_record));\n\
    memset(data_ptr, 0xaa, num * sizeof(struct data_record));\n\
\n\
    fetch();\n\
    p = data_ptr;\n\
    while (sqlca.sqlcode == 0) {\n\
        *p++ = data_tmp;\n\
        fetch();\n\
    }\n\
}",

/* body for main */
"{\n\
    load_data();\n\
    if (sizeof (int) == 2 && data_ptr[0].dummy != 0x5555)\n\
      abort ();\n\
    else if (sizeof (int) > 2 && data_ptr[0].dummy != 0x55555555)\n\
      abort ();\n\
    exit (0);\n\
}",
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
                printf("Test ./generated/990628-1.c failed\n");
                exit(exit_value);
            }
        } else {
            context = cod_copy_globals(context);
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp 990628-1.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/990628-1.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/990628-1.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/990628-1.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/990628-1.c Succeeded\n");
    return 0;
}
