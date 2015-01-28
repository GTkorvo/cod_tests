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
	test_output = fopen("20001124-1.c.output", "w");
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
	{"isofs_bread", (void*)(long)-1},
	{"do_isofs_readdir", (void*)(long)-1},
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	static char * isofs_bread(unsigned int block);\n\
	static int do_isofs_readdir(struct inode *inode, struct file *filp);\n\
	int main(int argc, char **argv);\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);";
    char *func_bodies[] = {

/* body for isofs_bread */
"{\n\
	if (block)\n\
	  abort ();\n\
	exit(0);\n\
}",

/* body for do_isofs_readdir */
"{\n\
	int bufsize = inode->i_sb->s_blocksize;\n\
	unsigned char bufbits = inode->i_sb->s_blocksize_bits;\n\
	unsigned int block, offset;\n\
	char *bh = 0;\n\
	int hs;\n\
\n\
 	if (filp->f_pos >= inode->i_size)\n\
		return 0;\n\
 \n\
	offset = filp->f_pos & (bufsize - 1);\n\
	block = filp->f_pos >> bufbits;\n\
	hs = inode->i_sb->s_hs;\n\
\n\
	while (filp->f_pos < inode->i_size) {\n\
		if (!bh)\n\
			bh = isofs_bread(block);\n\
\n\
		hs += block << bufbits;\n\
\n\
		if (hs == 0)\n\
			filp->f_pos++;\n\
\n\
		if (offset >= bufsize)\n\
			offset &= bufsize - 1;\n\
\n\
		if (*bh)\n\
			filp->f_pos++;\n\
\n\
		filp->f_pos++;\n\
	}\n\
	return 0;\n\
}",

/* body for main */
"{\n\
	s.s_blocksize = 512;\n\
	s.s_blocksize_bits = 9;\n\
	i.i_size = 2048;\n\
	i.i_sb = &s;\n\
	f.f_pos = 0;\n\
\n\
	do_isofs_readdir(&i,&f);\n\
	abort ();\n\
}",
""};

    char *func_decls[] = {
	"static char * isofs_bread(unsigned int block);",
	"static int do_isofs_readdir(struct inode *inode, struct file *filp);",
	"int main(int argc, char **argv);",
	""};

    char *global_decls[] = {
	"struct inode {\n\
	long long		i_size;\n\
	struct super_block	*i_sb;\n\
};",
	"struct file {\n\
	long long		f_pos;\n\
};",
	"struct super_block {\n\
	int			s_blocksize;\n\
	unsigned char		s_blocksize_bits;\n\
	int			s_hs;\n\
};",
	"struct super_block s;\n\
struct inode i;\n\
struct file f;",
""};

    int i;
    cod_code gen_code[3];
    for (i=0; i < 3; i++) {
        int j;
        if (verbose) {
             printf("Working on subroutine %s\n", externs[i].extern_name);
        }
        cod_parse_context context = new_cod_parse_context();
        cod_assoc_externs(context, externs);
        for (j=0; j < 4; j++) {
            cod_parse_for_globals(global_decls[j], context);
        }
        cod_parse_for_context(extern_string, context);
        cod_subroutine_declaration(func_decls[i], context);
        gen_code[i] = cod_code_gen(func_bodies[i], context);
        externs[i].extern_value = (void*) gen_code[i]->func;
        if (i == 2) {
            int (*func)() = (int(*)()) externs[i].extern_value;
            if (setjmp(env) == 0) {
                func();
            }
            if (exit_value != 0) {
                printf("Test ./generated/20001124-1.c failed\n");
                exit(exit_value);
            }
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp 20001124-1.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/20001124-1.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/20001124-1.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/20001124-1.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/20001124-1.c Succeeded\n");
    return 0;
}