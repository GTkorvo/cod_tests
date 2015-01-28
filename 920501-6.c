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
	test_output = fopen("920501-6.c.output", "w");
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
	{"str2llu", (void*)(long)-1},
	{"sqrtllu", (void*)(long)-1},
	{"plist", (void*)(long)-1},
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	long long unsigned str2llu (char *str);\n\
	long unsigned sqrtllu (long long unsigned t);\n\
	int plist (long long unsigned *tab, long long unsigned p0, p1);\n\
	void main (char *argv[], int argc);\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);";
    char *func_bodies[] = {

/* body for str2llu */
"{\n\
  long long unsigned acc;\n\
  int d;\n\
  acc =  *str++ - '0';\n\
  for (;;)\n\
    {\n\
      d = *str++;\n\
      if (d == '\\0')\n\
	break;\n\
      d -= '0';\n\
      acc = acc * 10 + d;\n\
    }\n\
\n\
  return acc;\n\
}",

/* body for sqrtllu */
"{\n\
  long long unsigned s;\n\
  long long unsigned b;\n\
\n\
  for (b = 0, s = t;  b++, (s >>= 1) != 0; )\n\
    ;\n\
\n\
  s = 1LL << (b >> 1);\n\
\n\
  if (b & 1)\n\
    s += s >> 1;\n\
\n\
  do\n\
    {\n\
      b = t / s;\n\
      s = (s + b) >> 1;\n\
    }\n\
  while (b < s);\n\
\n\
  return s;\n\
}",

/* body for plist */
"{\n\
  long long unsigned p;\n\
  long unsigned d;\n\
  long unsigned s;\n\
  long long unsigned *xp = tab;\n\
\n\
  for (p = p0;  p <= p1;  p += 2)\n\
    {\n\
      s = sqrtllu (p);\n\
\n\
      for (d = 3;  d <= s;  d += 2)\n\
	{\n\
	  long long unsigned q = p % d;\n\
	  if (q == 0)\n\
	    goto not_prime;\n\
	}\n\
\n\
      *xp++ = p;\n\
    not_prime:;\n\
    }\n\
  *xp = 0;\n\
  return xp - tab;\n\
}",

/* body for main */
"{\n\
  long long tab[10];\n\
  int nprimes;\n\
  nprimes = plist (str2llu (\"1234111111\"), str2llu (\"1234111127\"), tab);\n\
\n\
  if(tab[0]!=1234111117LL||tab[1]!=1234111121LL||tab[2]!=1234111127LL||tab[3]!=0)\n\
    abort();\n\
\n\
  exit(0);\n\
}",
""};

    char *func_decls[] = {
	"long long unsigned str2llu (char *str);",
	"long unsigned sqrtllu (long long unsigned t);",
	"int plist (long long unsigned *tab, long long unsigned p0, p1);",
	"void main (char *argv[], int argc);",
	""};

    char *global_decls[] = {
	"#include <stdio.h>\n\
\n\
/* Convert a decimal string to a long long unsigned.  No error check is\n\
   performed.  */",
""};

    int i;
    cod_code gen_code[4];
    for (i=0; i < 4; i++) {
        int j;
        if (verbose) {
             printf("Working on subroutine %s\n", externs[i].extern_name);
        }
        cod_parse_context context = new_cod_parse_context();
        cod_assoc_externs(context, externs);
        for (j=0; j < 1; j++) {
            cod_parse_for_globals(global_decls[j], context);
        }
        cod_parse_for_context(extern_string, context);
        cod_subroutine_declaration(func_decls[i], context);
        gen_code[i] = cod_code_gen(func_bodies[i], context);
        externs[i].extern_value = (void*) gen_code[i]->func;
        if (i == 3) {
            int (*func)() = (int(*)()) externs[i].extern_value;
            if (setjmp(env) == 0) {
                func();
            }
            if (exit_value != 0) {
                printf("Test ./generated/920501-6.c failed\n");
                exit(exit_value);
            }
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp 920501-6.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/920501-6.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/920501-6.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/920501-6.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/920501-6.c Succeeded\n");
    return 0;
}