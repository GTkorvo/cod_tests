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
// struct a {
// 	char a, b;
// 	short c;
// };
// 
// int
// a1()
// {
// 	static struct a x = { 1, 2, ~1 }, y = { 65, 2, ~2 };
// 
// 	return (x.a == (y.a & ~64) && x.b == y.b);
// }
// 
// int
// a2()
// {
// 	static struct a x = { 1, 66, ~1 }, y = { 1, 2, ~2 };
// 
// 	return (x.a == y.a && (x.b & ~64) == y.b);
// }
// 
// int
// a3()
// {
// 	static struct a x = { 9, 66, ~1 }, y = { 33, 18, ~2 };
// 
// 	return ((x.a & ~8) == (y.a & ~32) && (x.b & ~64) == (y.b & ~16));
// }
// 
// struct b {
// 	int c;
// 	short b, a;
// };
// 
// int
// b1()
// {
// 	static struct b x = { ~1, 2, 1 }, y = { ~2, 2, 65 };
// 
// 	return (x.a == (y.a & ~64) && x.b == y.b);
// }
// 
// int
// b2()
// {
// 	static struct b x = { ~1, 66, 1 }, y = { ~2, 2, 1 };
// 
// 	return (x.a == y.a && (x.b & ~64) == y.b);
// }
// 
// int
// b3()
// {
// 	static struct b x = { ~1, 66, 9 }, y = { ~2, 18, 33 };
// 
// 	return ((x.a & ~8) == (y.a & ~32) && (x.b & ~64) == (y.b & ~16));
// }
// 
// struct c {
// 	unsigned int c:4, b:14, a:14;
// } __attribute__ ((aligned));
// 
// int
// c1()
// {
// 	static struct c x = { ~1, 2, 1 }, y = { ~2, 2, 65 };
// 
// 	return (x.a == (y.a & ~64) && x.b == y.b);
// }
// 
// int
// c2()
// {
// 	static struct c x = { ~1, 66, 1 }, y = { ~2, 2, 1 };
// 
// 	return (x.a == y.a && (x.b & ~64) == y.b);
// }
// 
// int
// c3()
// {
// 	static struct c x = { ~1, 66, 9 }, y = { ~2, 18, 33 };
// 
// 	return ((x.a & ~8) == (y.a & ~32) && (x.b & ~64) == (y.b & ~16));
// }
// 
// struct d {
// 	unsigned int a:14, b:14, c:4;
// } __attribute__ ((aligned));
// 
// int
// d1()
// {
// 	static struct d x = { 1, 2, ~1 }, y = { 65, 2, ~2 };
// 
// 	return (x.a == (y.a & ~64) && x.b == y.b);
// }
// 
// int
// d2()
// {
// 	static struct d x = { 1, 66, ~1 }, y = { 1, 2, ~2 };
// 
// 	return (x.a == y.a && (x.b & ~64) == y.b);
// }
// 
// int
// d3()
// {
// 	static struct d x = { 9, 66, ~1 }, y = { 33, 18, ~2 };
// 
// 	return ((x.a & ~8) == (y.a & ~32) && (x.b & ~64) == (y.b & ~16));
// }
// 
// struct e {
// 	int c:4, b:14, a:14;
// } __attribute__ ((aligned));
// 
// int
// e1()
// {
// 	static struct e x = { ~1, -2, -65 }, y = { ~2, -2, -1 };
// 
// 	return (x.a == (y.a & ~64) && x.b == y.b);
// }
// 
// int
// e2()
// {
// 	static struct e x = { ~1, -2, -1 }, y = { ~2, -66, -1 };
// 
// 	return (x.a == y.a && (x.b & ~64) == y.b);
// }
// 
// int
// e3()
// {
// 	static struct e x = { ~1, -18, -33 }, y = { ~2, -66, -9 };
// 
// 	return ((x.a & ~8) == (y.a & ~32) && (x.b & ~64) == (y.b & ~16));
// }
// 
// int
// e4()
// {
// 	static struct e x = { -1, -1, 0 };
// 
// 	return x.a == 0 && x.b & 0x2000;
// }
// 
// struct f {
// 	int a:14, b:14, c:4;
// } __attribute__ ((aligned));
// 
// int
// f1()
// {
// 	static struct f x = { -65, -2, ~1 }, y = { -1, -2, ~2 };
// 
// 	return (x.a == (y.a & ~64) && x.b == y.b);
// }
// 
// int
// f2()
// {
// 	static struct f x = { -1, -2, ~1 }, y = { -1, -66, ~2 };
// 
// 	return (x.a == y.a && (x.b & ~64) == y.b);
// }
// 
// int
// f3()
// {
// 	static struct f x = { -33, -18, ~1 }, y = { -9, -66, ~2 };
// 
// 	return ((x.a & ~8) == (y.a & ~32) && (x.b & ~64) == (y.b & ~16));
// }
// 
// int
// f4()
// {
// 	static struct f x = { 0, -1, -1 };
// 
// 	return x.a == 0 && x.b & 0x2000;
// }
// 
// struct gx {
// 	int c:4, b:14, a:14;
// } __attribute__ ((aligned));
// struct gy {
// 	int b:14, a:14, c:4;
// } __attribute__ ((aligned));
// 
// int
// g1()
// {
// 	static struct gx x = { ~1, -2, -65 };
// 	static struct gy y = { -2, -1, ~2 };
// 
// 	return (x.a == (y.a & ~64) && x.b == y.b);
// }
// 
// int
// g2()
// {
// 	static struct gx x = { ~1, -2, -1 };
// 	static struct gy y = { -66, -1, ~2 };
// 
// 	return (x.a == y.a && (x.b & ~64) == y.b);
// }
// 
// int
// g3()
// {
// 	static struct gx x = { ~1, -18, -33 };
// 	static struct gy y = { -66, -9, ~2 };
// 
// 	return ((x.a & ~8) == (y.a & ~32) && (x.b & ~64) == (y.b & ~16));
// }
// 
// int
// g4()
// {
// 	static struct gx x = { ~1, 0x0020, 0x0010 };
// 	static struct gy y = { 0x0200, 0x0100, ~2 };
// 
// 	return ((x.a & 0x00f0) == (y.a & 0x0f00) &&
// 		(x.b & 0x00f0) == (y.b & 0x0f00));
// }
// 
// int
// g5()
// {
// 	static struct gx x = { ~1, 0x0200, 0x0100 };
// 	static struct gy y = { 0x0020, 0x0010, ~2 };
// 
// 	return ((x.a & 0x0f00) == (y.a & 0x00f0) &&
// 		(x.b & 0x0f00) == (y.b & 0x00f0));
// }
// 
// int
// g6()
// {
// 	static struct gx x = { ~1, 0xfe20, 0xfd10 };
// 	static struct gy y = { 0xc22f, 0xc11f, ~2 };
// 
// 	return ((x.a & 0x03ff) == (y.a & 0x3ff0) &&
// 		(x.b & 0x03ff) == (y.b & 0x3ff0));
// }
// 
// int
// g7()
// {
// 	static struct gx x = { ~1, 0xc22f, 0xc11f };
// 	static struct gy y = { 0xfe20, 0xfd10, ~2 };
// 
// 	return ((x.a & 0x3ff0) == (y.a & 0x03ff) &&
// 		(x.b & 0x3ff0) == (y.b & 0x03ff));
// }
// 
// struct hx {
// 	int a:14, b:14, c:4;
// } __attribute__ ((aligned));
// struct hy {
// 	int c:4, a:14, b:14;
// } __attribute__ ((aligned));
// 
// int
// h1()
// {
// 	static struct hx x = { -65, -2, ~1 };
// 	static struct hy y = { ~2, -1, -2 };
// 
// 	return (x.a == (y.a & ~64) && x.b == y.b);
// }
// 
// int
// h2()
// {
// 	static struct hx x = { -1, -2, ~1 };
// 	static struct hy y = { ~2, -1, -66 };
// 
// 	return (x.a == y.a && (x.b & ~64) == y.b);
// }
// 
// int
// h3()
// {
// 	static struct hx x = { -33, -18, ~1 };
// 	static struct hy y = { ~2, -9, -66 };
// 
// 	return ((x.a & ~8) == (y.a & ~32) && (x.b & ~64) == (y.b & ~16));
// }
// 
// int
// h4()
// {
// 	static struct hx x = { 0x0010, 0x0020, ~1 };
// 	static struct hy y = { ~2, 0x0100, 0x0200 };
// 
// 	return ((x.a & 0x00f0) == (y.a & 0x0f00) &&
// 		(x.b & 0x00f0) == (y.b & 0x0f00));
// }
// 
// int
// h5()
// {
// 	static struct hx x = { 0x0100, 0x0200, ~1 };
// 	static struct hy y = { ~2, 0x0010, 0x0020 };
// 
// 	return ((x.a & 0x0f00) == (y.a & 0x00f0) &&
// 		(x.b & 0x0f00) == (y.b & 0x00f0));
// }
// 
// int
// h6()
// {
// 	static struct hx x = { 0xfd10, 0xfe20, ~1 };
// 	static struct hy y = { ~2, 0xc11f, 0xc22f };
// 
// 	return ((x.a & 0x03ff) == (y.a & 0x3ff0) &&
// 		(x.b & 0x03ff) == (y.b & 0x3ff0));
// }
// 
// int
// h7()
// {
// 	static struct hx x = { 0xc11f, 0xc22f, ~1 };
// 	static struct hy y = { ~2, 0xfd10, 0xfe20 };
// 
// 	return ((x.a & 0x3ff0) == (y.a & 0x03ff) &&
// 		(x.b & 0x3ff0) == (y.b & 0x03ff));
// }
// 
// int
// main()
// {
//   if (!a1 ())
//     abort ();
//   if (!a2 ())
//     abort ();
//   if (!a3 ())
//     abort ();
//   if (!b1 ())
//     abort ();
//   if (!b2 ())
//     abort ();
//   if (!b3 ())
//     abort ();
//   if (!c1 ())
//     abort ();
//   if (!c2 ())
//     abort ();
//   if (!c3 ())
//     abort ();
//   if (!d1 ())
//     abort ();
//   if (!d2 ())
//     abort ();
//   if (!d3 ())
//     abort ();
//   if (!e1 ())
//     abort ();
//   if (!e2 ())
//     abort ();
//   if (!e3 ())
//     abort ();
//   if (!e4 ())
//     abort ();
//   if (!f1 ())
//     abort ();
//   if (!f2 ())
//     abort ();
//   if (!f3 ())
//     abort ();
//   if (!f4 ())
//     abort ();
//   if (!g1 ())
//     abort ();
//   if (!g2 ())
//     abort ();
//   if (!g3 ())
//     abort ();
//   if (g4 ())
//     abort ();
//   if (g5 ())
//     abort ();
//   if (!g6 ())
//     abort ();
//   if (!g7 ())
//     abort ();
//   if (!h1 ())
//     abort ();
//   if (!h2 ())
//     abort ();
//   if (!h3 ())
//     abort ();
//   if (h4 ())
//     abort ();
//   if (h5 ())
//     abort ();
//   if (!h6 ())
//     abort ();
//   if (!h7 ())
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
	test_output = fopen("990326-1.c.output", "w");
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
	{"a1", (void*)(long)-1},
	{"a2", (void*)(long)-1},
	{"a3", (void*)(long)-1},
	{"b1", (void*)(long)-1},
	{"b2", (void*)(long)-1},
	{"b3", (void*)(long)-1},
	{"c1", (void*)(long)-1},
	{"c2", (void*)(long)-1},
	{"c3", (void*)(long)-1},
	{"d1", (void*)(long)-1},
	{"d2", (void*)(long)-1},
	{"d3", (void*)(long)-1},
	{"e1", (void*)(long)-1},
	{"e2", (void*)(long)-1},
	{"e3", (void*)(long)-1},
	{"e4", (void*)(long)-1},
	{"f1", (void*)(long)-1},
	{"f2", (void*)(long)-1},
	{"f3", (void*)(long)-1},
	{"f4", (void*)(long)-1},
	{"g1", (void*)(long)-1},
	{"g2", (void*)(long)-1},
	{"g3", (void*)(long)-1},
	{"g4", (void*)(long)-1},
	{"g5", (void*)(long)-1},
	{"g6", (void*)(long)-1},
	{"g7", (void*)(long)-1},
	{"h1", (void*)(long)-1},
	{"h2", (void*)(long)-1},
	{"h3", (void*)(long)-1},
	{"h4", (void*)(long)-1},
	{"h5", (void*)(long)-1},
	{"h6", (void*)(long)-1},
	{"h7", (void*)(long)-1},
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{"printf", (void*)printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	int a1();\n\
	int a2();\n\
	int a3();\n\
	int b1();\n\
	int b2();\n\
	int b3();\n\
	int c1();\n\
	int c2();\n\
	int c3();\n\
	int d1();\n\
	int d2();\n\
	int d3();\n\
	int e1();\n\
	int e2();\n\
	int e3();\n\
	int e4();\n\
	int f1();\n\
	int f2();\n\
	int f3();\n\
	int f4();\n\
	int g1();\n\
	int g2();\n\
	int g3();\n\
	int g4();\n\
	int g5();\n\
	int g6();\n\
	int g7();\n\
	int h1();\n\
	int h2();\n\
	int h3();\n\
	int h4();\n\
	int h5();\n\
	int h6();\n\
	int h7();\n\
	int main();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);\n\
        int printf(const char *format, ...);";
    char *global_decls[] = {
	"struct a {\n\
	char a, b;\n\
	short c;\n\
};",
	"struct b {\n\
	int c;\n\
	short b, a;\n\
};",
	"struct c {\n\
	unsigned int c:4, b:14, a:14;\n\
} __attribute__ ((aligned));",
	"struct d {\n\
	unsigned int a:14, b:14, c:4;\n\
} __attribute__ ((aligned));",
	"struct e {\n\
	int c:4, b:14, a:14;\n\
} __attribute__ ((aligned));",
	"struct f {\n\
	int a:14, b:14, c:4;\n\
} __attribute__ ((aligned));",
	"struct gx {\n\
	int c:4, b:14, a:14;\n\
} __attribute__ ((aligned));",
	"struct gy {\n\
	int b:14, a:14, c:4;\n\
} __attribute__ ((aligned));",
	"struct hx {\n\
	int a:14, b:14, c:4;\n\
} __attribute__ ((aligned));",
	"struct hy {\n\
	int c:4, a:14, b:14;\n\
} __attribute__ ((aligned));",
""};

    char *func_decls[] = {
	"int a1();",
	"int a2();",
	"int a3();",
	"int b1();",
	"int b2();",
	"int b3();",
	"int c1();",
	"int c2();",
	"int c3();",
	"int d1();",
	"int d2();",
	"int d3();",
	"int e1();",
	"int e2();",
	"int e3();",
	"int e4();",
	"int f1();",
	"int f2();",
	"int f3();",
	"int f4();",
	"int g1();",
	"int g2();",
	"int g3();",
	"int g4();",
	"int g5();",
	"int g6();",
	"int g7();",
	"int h1();",
	"int h2();",
	"int h3();",
	"int h4();",
	"int h5();",
	"int h6();",
	"int h7();",
	"int main();",
	""};

    char *func_bodies[] = {

/* body for a1 */
"{\n\
	static struct a x = { 1, 2, ~1 }, y = { 65, 2, ~2 };\n\
\n\
	return (x.a == (y.a & ~64) && x.b == y.b);\n\
}",

/* body for a2 */
"{\n\
	static struct a x = { 1, 66, ~1 }, y = { 1, 2, ~2 };\n\
\n\
	return (x.a == y.a && (x.b & ~64) == y.b);\n\
}",

/* body for a3 */
"{\n\
	static struct a x = { 9, 66, ~1 }, y = { 33, 18, ~2 };\n\
\n\
	return ((x.a & ~8) == (y.a & ~32) && (x.b & ~64) == (y.b & ~16));\n\
}",

/* body for b1 */
"{\n\
	static struct b x = { ~1, 2, 1 }, y = { ~2, 2, 65 };\n\
\n\
	return (x.a == (y.a & ~64) && x.b == y.b);\n\
}",

/* body for b2 */
"{\n\
	static struct b x = { ~1, 66, 1 }, y = { ~2, 2, 1 };\n\
\n\
	return (x.a == y.a && (x.b & ~64) == y.b);\n\
}",

/* body for b3 */
"{\n\
	static struct b x = { ~1, 66, 9 }, y = { ~2, 18, 33 };\n\
\n\
	return ((x.a & ~8) == (y.a & ~32) && (x.b & ~64) == (y.b & ~16));\n\
}",

/* body for c1 */
"{\n\
	static struct c x = { ~1, 2, 1 }, y = { ~2, 2, 65 };\n\
\n\
	return (x.a == (y.a & ~64) && x.b == y.b);\n\
}",

/* body for c2 */
"{\n\
	static struct c x = { ~1, 66, 1 }, y = { ~2, 2, 1 };\n\
\n\
	return (x.a == y.a && (x.b & ~64) == y.b);\n\
}",

/* body for c3 */
"{\n\
	static struct c x = { ~1, 66, 9 }, y = { ~2, 18, 33 };\n\
\n\
	return ((x.a & ~8) == (y.a & ~32) && (x.b & ~64) == (y.b & ~16));\n\
}",

/* body for d1 */
"{\n\
	static struct d x = { 1, 2, ~1 }, y = { 65, 2, ~2 };\n\
\n\
	return (x.a == (y.a & ~64) && x.b == y.b);\n\
}",

/* body for d2 */
"{\n\
	static struct d x = { 1, 66, ~1 }, y = { 1, 2, ~2 };\n\
\n\
	return (x.a == y.a && (x.b & ~64) == y.b);\n\
}",

/* body for d3 */
"{\n\
	static struct d x = { 9, 66, ~1 }, y = { 33, 18, ~2 };\n\
\n\
	return ((x.a & ~8) == (y.a & ~32) && (x.b & ~64) == (y.b & ~16));\n\
}",

/* body for e1 */
"{\n\
	static struct e x = { ~1, -2, -65 }, y = { ~2, -2, -1 };\n\
\n\
	return (x.a == (y.a & ~64) && x.b == y.b);\n\
}",

/* body for e2 */
"{\n\
	static struct e x = { ~1, -2, -1 }, y = { ~2, -66, -1 };\n\
\n\
	return (x.a == y.a && (x.b & ~64) == y.b);\n\
}",

/* body for e3 */
"{\n\
	static struct e x = { ~1, -18, -33 }, y = { ~2, -66, -9 };\n\
\n\
	return ((x.a & ~8) == (y.a & ~32) && (x.b & ~64) == (y.b & ~16));\n\
}",

/* body for e4 */
"{\n\
	static struct e x = { -1, -1, 0 };\n\
\n\
	return x.a == 0 && x.b & 0x2000;\n\
}",

/* body for f1 */
"{\n\
	static struct f x = { -65, -2, ~1 }, y = { -1, -2, ~2 };\n\
\n\
	return (x.a == (y.a & ~64) && x.b == y.b);\n\
}",

/* body for f2 */
"{\n\
	static struct f x = { -1, -2, ~1 }, y = { -1, -66, ~2 };\n\
\n\
	return (x.a == y.a && (x.b & ~64) == y.b);\n\
}",

/* body for f3 */
"{\n\
	static struct f x = { -33, -18, ~1 }, y = { -9, -66, ~2 };\n\
\n\
	return ((x.a & ~8) == (y.a & ~32) && (x.b & ~64) == (y.b & ~16));\n\
}",

/* body for f4 */
"{\n\
	static struct f x = { 0, -1, -1 };\n\
\n\
	return x.a == 0 && x.b & 0x2000;\n\
}",

/* body for g1 */
"{\n\
	static struct gx x = { ~1, -2, -65 };\n\
	static struct gy y = { -2, -1, ~2 };\n\
\n\
	return (x.a == (y.a & ~64) && x.b == y.b);\n\
}",

/* body for g2 */
"{\n\
	static struct gx x = { ~1, -2, -1 };\n\
	static struct gy y = { -66, -1, ~2 };\n\
\n\
	return (x.a == y.a && (x.b & ~64) == y.b);\n\
}",

/* body for g3 */
"{\n\
	static struct gx x = { ~1, -18, -33 };\n\
	static struct gy y = { -66, -9, ~2 };\n\
\n\
	return ((x.a & ~8) == (y.a & ~32) && (x.b & ~64) == (y.b & ~16));\n\
}",

/* body for g4 */
"{\n\
	static struct gx x = { ~1, 0x0020, 0x0010 };\n\
	static struct gy y = { 0x0200, 0x0100, ~2 };\n\
\n\
	return ((x.a & 0x00f0) == (y.a & 0x0f00) &&\n\
		(x.b & 0x00f0) == (y.b & 0x0f00));\n\
}",

/* body for g5 */
"{\n\
	static struct gx x = { ~1, 0x0200, 0x0100 };\n\
	static struct gy y = { 0x0020, 0x0010, ~2 };\n\
\n\
	return ((x.a & 0x0f00) == (y.a & 0x00f0) &&\n\
		(x.b & 0x0f00) == (y.b & 0x00f0));\n\
}",

/* body for g6 */
"{\n\
	static struct gx x = { ~1, 0xfe20, 0xfd10 };\n\
	static struct gy y = { 0xc22f, 0xc11f, ~2 };\n\
\n\
	return ((x.a & 0x03ff) == (y.a & 0x3ff0) &&\n\
		(x.b & 0x03ff) == (y.b & 0x3ff0));\n\
}",

/* body for g7 */
"{\n\
	static struct gx x = { ~1, 0xc22f, 0xc11f };\n\
	static struct gy y = { 0xfe20, 0xfd10, ~2 };\n\
\n\
	return ((x.a & 0x3ff0) == (y.a & 0x03ff) &&\n\
		(x.b & 0x3ff0) == (y.b & 0x03ff));\n\
}",

/* body for h1 */
"{\n\
	static struct hx x = { -65, -2, ~1 };\n\
	static struct hy y = { ~2, -1, -2 };\n\
\n\
	return (x.a == (y.a & ~64) && x.b == y.b);\n\
}",

/* body for h2 */
"{\n\
	static struct hx x = { -1, -2, ~1 };\n\
	static struct hy y = { ~2, -1, -66 };\n\
\n\
	return (x.a == y.a && (x.b & ~64) == y.b);\n\
}",

/* body for h3 */
"{\n\
	static struct hx x = { -33, -18, ~1 };\n\
	static struct hy y = { ~2, -9, -66 };\n\
\n\
	return ((x.a & ~8) == (y.a & ~32) && (x.b & ~64) == (y.b & ~16));\n\
}",

/* body for h4 */
"{\n\
	static struct hx x = { 0x0010, 0x0020, ~1 };\n\
	static struct hy y = { ~2, 0x0100, 0x0200 };\n\
\n\
	return ((x.a & 0x00f0) == (y.a & 0x0f00) &&\n\
		(x.b & 0x00f0) == (y.b & 0x0f00));\n\
}",

/* body for h5 */
"{\n\
	static struct hx x = { 0x0100, 0x0200, ~1 };\n\
	static struct hy y = { ~2, 0x0010, 0x0020 };\n\
\n\
	return ((x.a & 0x0f00) == (y.a & 0x00f0) &&\n\
		(x.b & 0x0f00) == (y.b & 0x00f0));\n\
}",

/* body for h6 */
"{\n\
	static struct hx x = { 0xfd10, 0xfe20, ~1 };\n\
	static struct hy y = { ~2, 0xc11f, 0xc22f };\n\
\n\
	return ((x.a & 0x03ff) == (y.a & 0x3ff0) &&\n\
		(x.b & 0x03ff) == (y.b & 0x3ff0));\n\
}",

/* body for h7 */
"{\n\
	static struct hx x = { 0xc11f, 0xc22f, ~1 };\n\
	static struct hy y = { ~2, 0xfd10, 0xfe20 };\n\
\n\
	return ((x.a & 0x3ff0) == (y.a & 0x03ff) &&\n\
		(x.b & 0x3ff0) == (y.b & 0x03ff));\n\
}",

/* body for main */
"{\n\
  if (!a1 ())\n\
    abort ();\n\
  if (!a2 ())\n\
    abort ();\n\
  if (!a3 ())\n\
    abort ();\n\
  if (!b1 ())\n\
    abort ();\n\
  if (!b2 ())\n\
    abort ();\n\
  if (!b3 ())\n\
    abort ();\n\
  if (!c1 ())\n\
    abort ();\n\
  if (!c2 ())\n\
    abort ();\n\
  if (!c3 ())\n\
    abort ();\n\
  if (!d1 ())\n\
    abort ();\n\
  if (!d2 ())\n\
    abort ();\n\
  if (!d3 ())\n\
    abort ();\n\
  if (!e1 ())\n\
    abort ();\n\
  if (!e2 ())\n\
    abort ();\n\
  if (!e3 ())\n\
    abort ();\n\
  if (!e4 ())\n\
    abort ();\n\
  if (!f1 ())\n\
    abort ();\n\
  if (!f2 ())\n\
    abort ();\n\
  if (!f3 ())\n\
    abort ();\n\
  if (!f4 ())\n\
    abort ();\n\
  if (!g1 ())\n\
    abort ();\n\
  if (!g2 ())\n\
    abort ();\n\
  if (!g3 ())\n\
    abort ();\n\
  if (g4 ())\n\
    abort ();\n\
  if (g5 ())\n\
    abort ();\n\
  if (!g6 ())\n\
    abort ();\n\
  if (!g7 ())\n\
    abort ();\n\
  if (!h1 ())\n\
    abort ();\n\
  if (!h2 ())\n\
    abort ();\n\
  if (!h3 ())\n\
    abort ();\n\
  if (h4 ())\n\
    abort ();\n\
  if (h5 ())\n\
    abort ();\n\
  if (!h6 ())\n\
    abort ();\n\
  if (!h7 ())\n\
    abort ();\n\
  exit (0);\n\
}",
""};

    int i;
    cod_code gen_code[35];
    cod_parse_context context;
    for (i=0; i < 35; i++) {
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
        if (i == 34) {
            int (*func)() = (int(*)()) externs[i].extern_value;
            if (setjmp(env) == 0) {
                func();
            }
            if (exit_value != 0) {
                printf("Test ./generated/990326-1.c failed\n");
                exit(exit_value);
            }
        } else {
            context = cod_copy_globals(context);
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp 990326-1.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/990326-1.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/990326-1.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/990326-1.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/990326-1.c Succeeded\n");
    return 0;
}
