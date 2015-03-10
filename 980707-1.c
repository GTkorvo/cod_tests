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
// #include <string.h>
// 
// char **
// buildargv (char *input)
// {
//   static char *arglist[256];
//   int numargs = 0;
// 
//   while (1)
//     {
//       while (*input == ' ')
// 	input++;
//       if (*input == 0)
// 	break;
//       arglist [numargs++] = input;
//       while (*input != ' ' && *input != 0)
// 	input++;
//       if (*input == 0)
// 	break;
//       *(input++) = 0;
//     }
//   arglist [numargs] = NULL;
//   return arglist;
// }
// 
// 
// int main()
// {
//   char **args;
//   char input[256];
//   int i;
// 
//   strcpy(input, " a b");
//   args = buildargv(input);
// 
//   if (strcmp (args[0], "a"))
//     abort ();
//   if (strcmp (args[1], "b"))
//     abort ();
//   if (args[2] != NULL)
//     abort ();
//   
//   exit (0);
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
	test_output = fopen("980707-1.c.output", "w");
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
	{"buildargv", (void*)(long)-1},
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{"printf", (void*)printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	char ** buildargv (char *input);\n\
	int main();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);\n\
        int printf(const char *format, ...);";
    char *func_bodies[] = {

/* body for buildargv */
"{\n\
  static char *arglist[256];\n\
  int numargs = 0;\n\
\n\
  while (1)\n\
    {\n\
      while (*input == ' ')\n\
	input++;\n\
      if (*input == 0)\n\
	break;\n\
      arglist [numargs++] = input;\n\
      while (*input != ' ' && *input != 0)\n\
	input++;\n\
      if (*input == 0)\n\
	break;\n\
      *(input++) = 0;\n\
    }\n\
  arglist [numargs] = (void*)0;\n\
  return arglist;\n\
}",

/* body for main */
"{\n\
  char **args;\n\
  char input[256];\n\
  int i;\n\
\n\
  strcpy(input, \" a b\");\n\
  args = buildargv(input);\n\
\n\
  if (strcmp (args[0], \"a\"))\n\
    abort ();\n\
  if (strcmp (args[1], \"b\"))\n\
    abort ();\n\
  if (args[2] != NULL)\n\
    abort ();\n\
  \n\
  exit (0);\n\
}",
""};

    char *func_decls[] = {
	"char ** buildargv (char *input);",
	"int main();",
	""};

    char *global_decls[] = {
	"#include <stdlib.h>\n\
#include <string.h>",
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
                printf("Test ./generated/980707-1.c failed\n");
                exit(exit_value);
            }
        } else {
            context = cod_copy_globals(context);
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp 980707-1.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/980707-1.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/980707-1.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/980707-1.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/980707-1.c Succeeded\n");
    return 0;
}
