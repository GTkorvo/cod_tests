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
// typedef unsigned int dev_t;
// typedef unsigned int kdev_t;
// 
// static inline kdev_t to_kdev_t(int dev)
// {
// 	int major, minor;
// 	
// 	if (sizeof(kdev_t) == 16)
// 		return (kdev_t)dev;
// 	major = (dev >> 8);
// 	minor = (dev & 0xff);
// 	return ((( major ) << 22 ) | (  minor )) ;
// 
// }
// 
// void do_mknod(const char * filename, int mode, kdev_t dev)
// {
// 	if (dev==0x15800078)
// 		exit(0);
// 	else
// 		abort();
// }
// 
// 
// char * getname(const char * filename)
// {
// 	register unsigned int a1,a2,a3,a4,a5,a6,a7,a8,a9;
// 	a1 = (unsigned int)(filename) *5 + 1;
// 	a2 = (unsigned int)(filename) *6 + 2;
// 	a3 = (unsigned int)(filename) *7 + 3;
// 	a4 = (unsigned int)(filename) *8 + 4;
// 	a5 = (unsigned int)(filename) *9 + 5;
// 	a6 = (unsigned int)(filename) *10 + 5;
// 	a7 = (unsigned int)(filename) *11 + 5;
// 	a8 = (unsigned int)(filename) *12 + 5;
// 	a9 = (unsigned int)(filename) *13 + 5;
// 	return (char *)(a1*a2+a3*a4+a5*a6+a7*a8+a9);
// }
// 
// int sys_mknod(const char * filename, int mode, dev_t dev)
// {
// 	int error;
// 	char * tmp;
// 
// 	tmp = getname(filename);
// 	error = ((long)( tmp )) ;
// 	do_mknod(tmp,mode,to_kdev_t(dev));
// 	return error;
// }
// 
// int main(void)
// {
// 	if (sizeof (int) < 4)
// 	  exit (0);
// 
// 	return sys_mknod("test",1,0x12345678);
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
	test_output = fopen("980526-2.c.output", "w");
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
	{"to_kdev_t", (void*)(long)-1},
	{"do_mknod", (void*)(long)-1},
	{"getname", (void*)(long)-1},
	{"sys_mknod", (void*)(long)-1},
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{"printf", (void*)printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	static inline kdev_t to_kdev_t(int dev);\n\
	void do_mknod(const char * filename, int mode, kdev_t dev);\n\
	char * getname(const char * filename);\n\
	int sys_mknod(const char * filename, int mode, dev_t dev);\n\
	int main();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);\n\
        int printf(const char *format, ...);";
    char *global_decls[] = {
	"typedef unsigned int dev_t;\n\
typedef unsigned int kdev_t;",
""};

    char *func_decls[] = {
	"static inline kdev_t to_kdev_t(int dev);",
	"void do_mknod(const char * filename, int mode, kdev_t dev);",
	"char * getname(const char * filename);",
	"int sys_mknod(const char * filename, int mode, dev_t dev);",
	"int main();",
	""};

    char *func_bodies[] = {

/* body for to_kdev_t */
"{\n\
	int major, minor;\n\
	\n\
	if (sizeof(kdev_t) == 16)\n\
		return (kdev_t)dev;\n\
	major = (dev >> 8);\n\
	minor = (dev & 0xff);\n\
	return ((( major ) << 22 ) | (  minor )) ;\n\
\n\
}",

/* body for do_mknod */
"{\n\
	if (dev==0x15800078)\n\
		exit(0);\n\
	else\n\
		abort();\n\
}",

/* body for getname */
"{\n\
	register unsigned int a1,a2,a3,a4,a5,a6,a7,a8,a9;\n\
	a1 = (unsigned int)(filename) *5 + 1;\n\
	a2 = (unsigned int)(filename) *6 + 2;\n\
	a3 = (unsigned int)(filename) *7 + 3;\n\
	a4 = (unsigned int)(filename) *8 + 4;\n\
	a5 = (unsigned int)(filename) *9 + 5;\n\
	a6 = (unsigned int)(filename) *10 + 5;\n\
	a7 = (unsigned int)(filename) *11 + 5;\n\
	a8 = (unsigned int)(filename) *12 + 5;\n\
	a9 = (unsigned int)(filename) *13 + 5;\n\
	return (char *)(a1*a2+a3*a4+a5*a6+a7*a8+a9);\n\
}",

/* body for sys_mknod */
"{\n\
	int error;\n\
	char * tmp;\n\
\n\
	tmp = getname(filename);\n\
	error = ((long)( tmp )) ;\n\
	do_mknod(tmp,mode,to_kdev_t(dev));\n\
	return error;\n\
}",

/* body for main */
"{\n\
	if (sizeof (int) < 4)\n\
	  exit (0);\n\
\n\
	return sys_mknod(\"test\",1,0x12345678);\n\
}",
""};

    int i;
    cod_code gen_code[5];
    cod_parse_context context;
    for (i=0; i < 5; i++) {
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
        if (i == 4) {
            int (*func)() = (int(*)()) externs[i].extern_value;
            if (setjmp(env) == 0) {
                func();
            }
            if (exit_value != 0) {
                printf("Test ./generated/980526-2.c failed\n");
                exit(exit_value);
            }
        } else {
            context = cod_copy_globals(context);
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp 980526-2.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/980526-2.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/980526-2.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/980526-2.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/980526-2.c Succeeded\n");
    return 0;
}
