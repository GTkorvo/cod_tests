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
	test_output = fopen("20020404-1.c.output", "w");
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
	{"dump_bfd_file", (void*)(long)-1},
	{"bfd_openw_with_cleanup", (void*)(long)-1},
	{"bfd_make_section_anyway", (void*)(long)-1},
	{"bfd_set_section_size", (void*)(long)-1},
	{"bfd_set_section_flags", (void*)(long)-1},
	{"bfd_set_section_contents", (void*)(long)-1},
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{"printf", (void*)printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	static void dump_bfd_file (char *filename, char *mode,                char *target, CORE_ADDR vaddr,                char *buf, int len);\n\
	static bfd * bfd_openw_with_cleanup (char *filename, const char *target, char *mode);\n\
	static asection * bfd_make_section_anyway (bfd *abfd, const char *name);\n\
	static boolean bfd_set_section_size (bfd *abfd, asection *sec, bfd_size_type val);\n\
	static boolean bfd_set_section_flags (bfd *abfd, asection *sec, flagword flags);\n\
	static boolean bfd_set_section_contents (bfd *abfd, asection *section, void * data, file_ptr offset, bfd_size_type count);\n\
	int main();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);\n\
        int printf(const char *format, ...);";
    char *func_bodies[] = {

/* body for dump_bfd_file */
"{\n\
  bfd *obfd;\n\
  asection *osection;\n\
\n\
  obfd = bfd_openw_with_cleanup (filename, target, mode);\n\
  osection = bfd_make_section_anyway (obfd, \".newsec\");\n\
  bfd_set_section_size (obfd, osection, len);\n\
  (((osection)->vma = (osection)->lma= (vaddr)), ((osection)->user_set_vma = (boolean)true), true);\n\
  (((osection)->alignment_power = (0)),true);\n\
  bfd_set_section_flags (obfd, osection, 0x203);\n\
  osection->entsize = 0;\n\
  bfd_set_section_contents (obfd, osection, buf, 0, len);\n\
}",

/* body for bfd_openw_with_cleanup */
"{\n\
	static bfd foo_bfd = { 0 };\n\
	return &foo_bfd;\n\
}",

/* body for bfd_make_section_anyway */
"{\n\
	static asection foo_section = { false, 0x0, 0x0, 0 };\n\
\n\
	return &foo_section;\n\
}",

/* body for bfd_set_section_size */
"{\n\
	return true;\n\
}",

/* body for bfd_set_section_flags */
"{\n\
}",

/* body for bfd_set_section_contents */
"{\n\
	if (count != (bfd_size_type)0x1eadbeef)\n\
		abort();\n\
}",

/* body for main */
"{\n\
	dump_bfd_file(0, 0, 0, (CORE_ADDR)0xdeadbeef, hello, (int)0x1eadbeef);\n\
	exit(0);\n\
}",
""};

    char *func_decls[] = {
	"static void dump_bfd_file (char *filename, char *mode,                char *target, CORE_ADDR vaddr,                char *buf, int len);",
	"static bfd * bfd_openw_with_cleanup (char *filename, const char *target, char *mode);",
	"static asection * bfd_make_section_anyway (bfd *abfd, const char *name);",
	"static boolean bfd_set_section_size (bfd *abfd, asection *sec, bfd_size_type val);",
	"static boolean bfd_set_section_flags (bfd *abfd, asection *sec, flagword flags);",
	"static boolean bfd_set_section_contents (bfd *abfd, asection *section, void * data, file_ptr offset, bfd_size_type count);",
	"int main();",
	""};

    char *global_decls[] = {
	"typedef long long bfd_signed_vma;\n\
typedef bfd_signed_vma file_ptr;",
	"typedef enum bfd_boolean {false, true} boolean;\n\
\n\
typedef unsigned long long bfd_size_type;\n\
\n\
typedef unsigned int flagword;\n\
\n\
typedef unsigned long long CORE_ADDR;\n\
typedef unsigned long long bfd_vma;",
	"struct bfd_struct {\n\
	int x;\n\
};",
	"struct asection_struct {\n\
  unsigned int user_set_vma : 1;\n\
  bfd_vma vma;\n\
  bfd_vma lma;\n\
  unsigned int alignment_power;\n\
  unsigned int entsize;\n\
};\n\
\n\
typedef struct bfd_struct bfd;\n\
typedef struct asection_struct asection;\n\
\n\
static bfd *\n\
bfd_openw_with_cleanup (char *filename, const char *target, char *mode);\n\
\n\
static asection *\n\
bfd_make_section_anyway (bfd *abfd, const char *name);\n\
\n\
static boolean\n\
bfd_set_section_size (bfd *abfd, asection *sec, bfd_size_type val);\n\
\n\
static boolean\n\
bfd_set_section_flags (bfd *abfd, asection *sec, flagword flags);\n\
\n\
static boolean\n\
bfd_set_section_contents (bfd *abfd, asection *section, void * data, file_ptr offset, bfd_size_type count);",
	"static char hello[] = \"hello\";",
""};

    int i;
    cod_code gen_code[7];
    cod_parse_context context;
    for (i=0; i < 7; i++) {
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
        if (i == 6) {
            int (*func)() = (int(*)()) externs[i].extern_value;
            if (setjmp(env) == 0) {
                func();
            }
            if (exit_value != 0) {
                printf("Test ./generated/20020404-1.c failed\n");
                exit(exit_value);
            }
        } else {
            context = cod_copy_globals(context);
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp 20020404-1.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/20020404-1.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/20020404-1.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/20020404-1.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/20020404-1.c Succeeded\n");
    return 0;
}
