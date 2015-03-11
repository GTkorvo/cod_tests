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
// /* PR target/7042.  When reorg.c changed branches into return insns, it
//    completely forgot about any current_function_epilogue_delay_list and
//    dropped those insns.  Uncovered on cris-axis-elf, where an insn in an
//    epilogue delay-slot set the return-value register with the test-case
//    below.  Derived from ghostscript-6.52 (GPL) by hp@axis.com.  */
// 
// typedef struct font_hints_s {
//   int axes_swapped;
//   int x_inverted, y_inverted;
// } font_hints;
// typedef struct gs_fixed_point_s {
//   long x, y;
// } gs_fixed_point;
// 
// int
// line_hints(const font_hints *fh, const gs_fixed_point *p0,
// 	   const gs_fixed_point *p1)
// {
//   long dx = p1->x - p0->x;
//   long dy = p1->y - p0->y;
//   long adx, ady;
//   int xi = fh->x_inverted, yi = fh->y_inverted;
//   int hints;
//   if (xi)
//     dx = -dx;
//   if (yi)
//     dy = -dy;
//   if (fh->axes_swapped) {
//     long t = dx;
//     int ti = xi;
//     dx = dy, xi = yi;
//     dy = t, yi = ti;
//   }
//   adx = dx < 0 ? -dx : dx;
//   ady = dy < 0 ? -dy : dy;
//   if (dy != 0 && (adx <= ady >> 4)) {
//     hints = dy > 0 ? 2 : 1;
//     if (xi)
//       hints ^= 3;
//   } else if (dx != 0 && (ady <= adx >> 4)) {
//     hints = dx < 0 ? 8 : 4;
//     if (yi)
//       hints ^= 12;
//   } else
//     hints = 0;
//   return hints;
// }
// int main ()
// {
//   static font_hints fh[] = {{0, 1, 0}, {0, 0, 1}, {0, 0, 0}};
//   static gs_fixed_point gsf[]
//     = {{0x30000, 0x13958}, {0x30000, 0x18189},
//        {0x13958, 0x30000}, {0x18189, 0x30000}};
//   if (line_hints (fh, gsf, gsf + 1) != 1
//       || line_hints (fh + 1, gsf + 2, gsf + 3) != 8
//       || line_hints (fh + 2, gsf + 2, gsf + 3) != 4)
//     abort ();
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
	test_output = fopen("20020615-1.c.output", "w");
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
	{"line_hints", (void*)(long)-1},
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{"printf", (void*)printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	int line_hints(const font_hints *fh, const gs_fixed_point *p0, 	   const gs_fixed_point *p1);\n\
	int main ();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);\n\
        int printf(const char *format, ...);";
    char *global_decls[] = {
	"typedef struct font_hints_s {\n\
  int axes_swapped;\n\
  int x_inverted, y_inverted;\n\
} font_hints;",
	"typedef struct gs_fixed_point_s {\n\
  long x, y;\n\
} gs_fixed_point;",
""};

    char *func_decls[] = {
	"int line_hints(const font_hints *fh, const gs_fixed_point *p0, 	   const gs_fixed_point *p1);",
	"int main ();",
	""};

    char *func_bodies[] = {

/* body for line_hints */
"{\n\
  long dx = p1->x - p0->x;\n\
  long dy = p1->y - p0->y;\n\
  long adx, ady;\n\
  int xi = fh->x_inverted, yi = fh->y_inverted;\n\
  int hints;\n\
  if (xi)\n\
    dx = -dx;\n\
  if (yi)\n\
    dy = -dy;\n\
  if (fh->axes_swapped) {\n\
    long t = dx;\n\
    int ti = xi;\n\
    dx = dy, xi = yi;\n\
    dy = t, yi = ti;\n\
  }\n\
  adx = dx < 0 ? -dx : dx;\n\
  ady = dy < 0 ? -dy : dy;\n\
  if (dy != 0 && (adx <= ady >> 4)) {\n\
    hints = dy > 0 ? 2 : 1;\n\
    if (xi)\n\
      hints ^= 3;\n\
  } else if (dx != 0 && (ady <= adx >> 4)) {\n\
    hints = dx < 0 ? 8 : 4;\n\
    if (yi)\n\
      hints ^= 12;\n\
  } else\n\
    hints = 0;\n\
  return hints;\n\
}",

/* body for main */
"{\n\
  static font_hints fh[] = {{0, 1, 0}, {0, 0, 1}, {0, 0, 0}};\n\
  static gs_fixed_point gsf[]\n\
    = {{0x30000, 0x13958}, {0x30000, 0x18189},\n\
       {0x13958, 0x30000}, {0x18189, 0x30000}};\n\
  if (line_hints (fh, gsf, gsf + 1) != 1\n\
      || line_hints (fh + 1, gsf + 2, gsf + 3) != 8\n\
      || line_hints (fh + 2, gsf + 2, gsf + 3) != 4)\n\
    abort ();\n\
  exit (0);\n\
}",
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
                printf("Test ./generated/20020615-1.c failed\n");
                exit(exit_value);
            }
        } else {
            context = cod_copy_globals(context);
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp 20020615-1.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/20020615-1.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/20020615-1.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/20020615-1.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/20020615-1.c Succeeded\n");
    return 0;
}
