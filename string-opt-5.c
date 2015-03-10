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
// /* Copyright (C) 2000  Free Software Foundation.
// 
//    Ensure builtin strlen, strcmp, strchr, strrchr and strncpy
//    perform correctly.
// 
//    Written by Jakub Jelinek, 11/7/2000.  */
// 
// extern void abort (void);
// extern __SIZE_TYPE__ strlen (const char *);
// extern int strcmp (const char *, const char *);
// extern char *strchr (const char *, int);
// extern char *strrchr (const char *, int);
// extern char *strncpy (char *, const char *, __SIZE_TYPE__);
// extern void *memset (void *, int, __SIZE_TYPE__);
// extern int memcmp (const void *, const void *, __SIZE_TYPE__);
// 
// int x = 6;
// int y = 1;
// char *bar = "hi world";
// char buf [64];
// 
// int main()
// {
//   const char *const foo = "hello world";
//   char dst [64];
// 
//   if (strlen (bar) != 8)
//     abort ();
//   if (strlen (bar + (++x & 2)) != 6)
//     abort ();
//   if (x != 7)
//     abort ();
//   if (strlen (foo + (x++, 6)) != 5)
//     abort ();
//   if (x != 8)
//     abort ();
//   if (strlen (foo + (++x & 1)) != 10)
//     abort ();
//   if (x != 9)
//     abort ();
//   if (strcmp (foo + (x -= 6), "lo world"))
//     abort ();
//   if (x != 3)
//     abort ();
//   if (strcmp (foo, bar) >= 0)
//     abort ();
//   if (strcmp (foo, bar + (x++ & 1)) >= 0)
//     abort ();
//   if (x != 4)
//     abort ();
//   if (strchr (foo + (x++ & 7), 'l') != foo + 9)
//     abort ();
//   if (x != 5)
//     abort ();
//   if (strchr (bar, 'o') != bar + 4)
//     abort ();
//   if (strchr (bar, '\0')  != bar + 8)
//     abort ();
//   if (strrchr (bar, 'x'))
//     abort ();
//   if (strrchr (bar, 'o') != bar + 4)
//     abort ();
//   if (strcmp (foo + (x++ & 1), "ello world" + (--y & 1)))
//     abort ();
//   if (x != 6 || y != 0)
//     abort ();
//   dst[5] = ' ';
//   dst[6] = '\0';
//   x = 5;
//   y = 1;
//   if (strncpy (dst + 1, foo + (x++ & 3), 4) != dst + 1
//       || x != 6
//       || strcmp (dst + 1, "ello "))
//     abort ();
//   memset (dst, ' ', sizeof dst);
//   if (strncpy (dst + (++x & 1), (y++ & 3) + "foo", 10) != dst + 1
//       || x != 7
//       || y != 2
//       || memcmp (dst, " oo\0\0\0\0\0\0\0\0 ", 12))
//     abort ();
//   memset (dst, ' ', sizeof dst);
//   if (strncpy (dst, "hello", 8) != dst || memcmp (dst, "hello\0\0\0 ", 9))
//     abort ();
//   x = '!';
//   memset (buf, ' ', sizeof buf);
//   if (memset (buf, x++, ++y) != buf
//       || x != '!' + 1
//       || y != 3
//       || memcmp (buf, "!!!", 3))
//     abort ();
//   if (memset (buf + y++, '-', 8) != buf + 3
//       || y != 4
//       || memcmp (buf, "!!!--------", 11))
//     abort ();
//   x = 10;
//   if (memset (buf + ++x, 0, y++) != buf + 11
//       || x != 11
//       || y != 5
//       || memcmp (buf + 8, "---\0\0\0", 7))
//     abort ();
//   if (memset (buf + (x += 4), 0, 6) != buf + 15
//       || x != 15
//       || memcmp (buf + 10, "-\0\0\0\0\0\0\0\0\0", 11))
//     abort ();
// 
//   return 0;
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
	test_output = fopen("string-opt-5.c.output", "w");
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
  const char *const foo = \"hello world\";\n\
  char dst [64];\n\
\n\
  if (strlen (bar) != 8)\n\
    abort ();\n\
  if (strlen (bar + (++x & 2)) != 6)\n\
    abort ();\n\
  if (x != 7)\n\
    abort ();\n\
  if (strlen (foo + (x++, 6)) != 5)\n\
    abort ();\n\
  if (x != 8)\n\
    abort ();\n\
  if (strlen (foo + (++x & 1)) != 10)\n\
    abort ();\n\
  if (x != 9)\n\
    abort ();\n\
  if (strcmp (foo + (x -= 6), \"lo world\"))\n\
    abort ();\n\
  if (x != 3)\n\
    abort ();\n\
  if (strcmp (foo, bar) >= 0)\n\
    abort ();\n\
  if (strcmp (foo, bar + (x++ & 1)) >= 0)\n\
    abort ();\n\
  if (x != 4)\n\
    abort ();\n\
  if (strchr (foo + (x++ & 7), 'l') != foo + 9)\n\
    abort ();\n\
  if (x != 5)\n\
    abort ();\n\
  if (strchr (bar, 'o') != bar + 4)\n\
    abort ();\n\
  if (strchr (bar, '\\0')  != bar + 8)\n\
    abort ();\n\
  if (strrchr (bar, 'x'))\n\
    abort ();\n\
  if (strrchr (bar, 'o') != bar + 4)\n\
    abort ();\n\
  if (strcmp (foo + (x++ & 1), \"ello world\" + (--y & 1)))\n\
    abort ();\n\
  if (x != 6 || y != 0)\n\
    abort ();\n\
  dst[5] = ' ';\n\
  dst[6] = '\\0';\n\
  x = 5;\n\
  y = 1;\n\
  if (strncpy (dst + 1, foo + (x++ & 3), 4) != dst + 1\n\
      || x != 6\n\
      || strcmp (dst + 1, \"ello \"))\n\
    abort ();\n\
  memset (dst, ' ', sizeof dst);\n\
  if (strncpy (dst + (++x & 1), (y++ & 3) + \"foo\", 10) != dst + 1\n\
      || x != 7\n\
      || y != 2\n\
      || memcmp (dst, \" oo\\0\\0\\0\\0\\0\\0\\0\\0 \", 12))\n\
    abort ();\n\
  memset (dst, ' ', sizeof dst);\n\
  if (strncpy (dst, \"hello\", 8) != dst || memcmp (dst, \"hello\\0\\0\\0 \", 9))\n\
    abort ();\n\
  x = '!';\n\
  memset (buf, ' ', sizeof buf);\n\
  if (memset (buf, x++, ++y) != buf\n\
      || x != '!' + 1\n\
      || y != 3\n\
      || memcmp (buf, \"!!!\", 3))\n\
    abort ();\n\
  if (memset (buf + y++, '-', 8) != buf + 3\n\
      || y != 4\n\
      || memcmp (buf, \"!!!--------\", 11))\n\
    abort ();\n\
  x = 10;\n\
  if (memset (buf + ++x, 0, y++) != buf + 11\n\
      || x != 11\n\
      || y != 5\n\
      || memcmp (buf + 8, \"---\\0\\0\\0\", 7))\n\
    abort ();\n\
  if (memset (buf + (x += 4), 0, 6) != buf + 15\n\
      || x != 15\n\
      || memcmp (buf + 10, \"-\\0\\0\\0\\0\\0\\0\\0\\0\\0\", 11))\n\
    abort ();\n\
\n\
  return 0;\n\
}",
""};

    char *func_decls[] = {
	"int main();",
	""};

    char *global_decls[] = {
	"\n\
extern __SIZE_TYPE__ strlen (const char *);\n\
extern int strcmp (const char *, const char *);\n\
extern char *strchr (const char *, int);\n\
extern char *strrchr (const char *, int);\n\
extern char *strncpy (char *, const char *, __SIZE_TYPE__);\n\
extern void *memset (void *, int, __SIZE_TYPE__);\n\
extern int memcmp (const void *, const void *, __SIZE_TYPE__);\n\
\n\
int x = 6;\n\
int y = 1;\n\
char *bar = \"hi world\";\n\
char buf [64];",
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
                printf("Test ./generated/string-opt-5.c failed\n");
                exit(exit_value);
            }
        } else {
            context = cod_copy_globals(context);
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp string-opt-5.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/string-opt-5.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/string-opt-5.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/string-opt-5.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/string-opt-5.c Succeeded\n");
    return 0;
}
