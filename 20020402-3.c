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
	test_output = fopen("20020402-3.c.output", "w");
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
	{"blockvector_for_pc_sect", (void*)(long)-1},
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	struct blockvector *blockvector_for_pc_sect(register CORE_ADDR pc, 					    struct symtab *symtab);\n\
	int main();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);";
    char *func_bodies[] = {

/* body for blockvector_for_pc_sect */
"{\n\
  register struct block *b;\n\
  register int bot, top, half;\n\
  struct blockvector *bl;\n\
\n\
  bl = symtab->blockvector;\n\
  b = bl->block[0];\n\
\n\
  bot = 0;\n\
  top = bl->nblocks;\n\
\n\
  while (top - bot > 1)\n\
    {\n\
      half = (top - bot + 1) >> 1;\n\
      b = bl->block[bot + half];\n\
      if (b->startaddr <= pc)\n\
	bot += half;\n\
      else\n\
	top = bot + half;\n\
    }\n\
\n\
  while (bot >= 0)\n\
    {\n\
      b = bl->block[bot];\n\
      if (b->endaddr > pc)\n\
	{\n\
	  return bl;\n\
	}\n\
      bot--;\n\
    }\n\
  return 0;\n\
}",

/* body for main */
"{\n\
  struct block a = { 0, 0x10000, 0, 0, 1, 20 };\n\
  struct block b = { 0x10000, 0x20000, 0, 0, 1, 20 };\n\
  struct blockvector bv = { 2, { &a, &b } };\n\
  struct symtab s = { &bv };\n\
\n\
  struct blockvector *ret;\n\
\n\
  ret = blockvector_for_pc_sect(0x500, &s);\n\
\n\
  return 0;\n\
}",
""};

    char *func_decls[] = {
	"struct blockvector *blockvector_for_pc_sect(register CORE_ADDR pc, 					    struct symtab *symtab);",
	"int main();",
	""};

    char *global_decls[] = {
	"typedef unsigned long long CORE_ADDR;\n\
\n\
struct blockvector;",
	"struct symtab {\n\
  struct blockvector *blockvector;\n\
};",
	"struct sec {\n\
  void *unused;\n\
};",
	"struct symbol {\n\
  int len;\n\
  char *name;\n\
};",
	"struct block {\n\
	CORE_ADDR startaddr, endaddr;\n\
	struct symbol *function;\n\
	struct block *superblock;\n\
	unsigned char gcc_compile_flag;\n\
	int nsyms;\n\
	struct symbol syms[1];\n\
};",
	"struct blockvector {\n\
	int nblocks;\n\
	struct block *block[2];\n\
};",
""};

    int i;
    cod_code gen_code[2];
    cod_parse_context context;
    for (i=0; i < 2; i++) {
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
        if (i == 1) {
            int (*func)() = (int(*)()) externs[i].extern_value;
            if (setjmp(env) == 0) {
                func();
            }
            if (exit_value != 0) {
                printf("Test ./generated/20020402-3.c failed\n");
                exit(exit_value);
            }
        } else {
            context = cod_copy_globals(context);
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp 20020402-3.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/20020402-3.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/20020402-3.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/20020402-3.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/20020402-3.c Succeeded\n");
    return 0;
}
