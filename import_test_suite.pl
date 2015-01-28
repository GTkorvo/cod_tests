#perl
use strict;
use warnings;
use File::Basename;
my $GCC_TESTSUITE_PATH="$ENV{HOME}/prog/gcc-3.3.1-3/gcc/testsuite/gcc.c-torture/execute";

my $TCC_TESTSUITE_PATH="$ENV{HOME}/prog/tinycc/tests/tests2";

my @files = glob "${GCC_TESTSUITE_PATH}/* ${TCC_TESTSUITE_PATH}/*";

my $compile_failed_count = 0;
my $execute_failed_count = 0;
my $expected_failure_count = 0;

my %tcc_exceptions =  ( "06_case" => "No CASE!",
		     "12_hashdefine" => "Uses #define",
		     "15_recursion" => "CoD can't do recursion",
		     "18_include" => "CoD has special #include",
		     "25_quicksort" => "CoD can't do recursion",
		     "30_hanoi" => "Uses #define, recurses",
		     "31_args" => "CoD doesn't have ARGV",
		     "32_led" => "Uses #define",
		     "40_stdio" => "No stdio in CoD",
		     "41_hashif" => "Uses #if",
		     "42_function_pointer" => "No function pointers in CoD",
		     "46_grep" => "Grep requires ARGV, opening files, etc.  Too ambitious",
		     "47_switch_return" => "No CASE!",
		     "59_function_array" => "No function pointers",
		     "55_lshift_type" => "Uses #define",
		     "64_macro_nesting" => "No Macros",
		     "65_macro_concat_start" => "No Macros",
		     "66_macro_concat_end" => "No Macros",
		     "67_macro_concat" => "No Macros",
		     "68_macro_param_list_err_1" => "No Macros",
		     "69_macro_param_list_err_2" => "No Macros",
		     "test" => "reason");

my %gcc_exceptions =  ( "20000205-1" => "Recurses",
		     "20000113-1" => "Uses bitfields",
		     "20000227-1" => "Funky string inits, don't generate properly",
		     "20000223-1" => "Uses #define",
		     "20000402-1" => "Uses #if",
		     "20000403-1" => "forward declarations",
		     "20000412-1" => "Recurses",
		     "20000412-2" => "Recurses",
		     "20000412-3" => "gen_test.pl can't parse the param decl properly",
		     "20000419-1" => "complex constant",
		     "20000519-1" => "Uses stdarg.h",
		     "20000818-1" => "Uses #if",
		     "20000822-1" => "Uses #if",
		     "20000914-1" => "Uses union",
		     "20001203-2" => "Uses union",
		     "20001228-1" => "Uses union",
		     "20001229-1" => "Uses #if",
		     "20010119-1" => "Uses #if",
		     "20010122-1" => "Uses #define",
		     "20010124-1" => "Uses union",
		     "20010724-1" => "Uses #define / union",
		     "20011008-3" => "Uses #define / union",
		     "20011115-1" => "Uses #if",
		     "20020108-1" => "Uses #define",
		     "20020225-2" => "Uses union",
		     "20020226-1" => "Uses #define",
		     "20020307-1" => "Uses #define",
		     "20020402-1" => "Uses #define",
		     "20020402-2" => "Uses union",
		     "20020412-1" => "Uses stdarg.h",
		     "20020506-1" => "Uses #define",
		     "20020508-1" => "Uses #define",
		     "20020508-2" => "Uses #define",
		     "20020508-3" => "Uses #define",
		     "20020619-1" => "Uses #define / union",
		     "20020720-1" => "Uses #if",
		     "20021120-1" => "Uses #define",
		     "20021120-3" => "Uses #define",
		     "20030613-1" => "Uses #define",
		     "20030714-1" => "Uses union",
		     "920302-1" => "Uses #if",
		     "920410-1" => "Uses #define",
		     "920415-1" => "Uses #if",
		     "920428-2" => "Uses #if",
		     "920501-3" => "Uses #if",
		     "920501-4" => "Uses #if",
		     "920501-5" => "Uses #if",
		     "920501-7" => "Uses #define",
		     "920501-8" => "Uses stdarg.h",
		     "920612-2" => "Uses #if",
		     "920625-1" => "Uses stdarg.h",
		     "920721-4" => "Uses #if",
		     "920726-1" => "Uses stdarg.h",
		     "920908-1" => "Uses stdarg.h",
		     "921007-1" => "Uses #define",
		     "921017-1" => "Uses #if",
		     "921112-1" => "Uses union",
		     "921113-1" => "Uses #define",
		     "921202-1" => "Uses #define",
		     "921204-1" => "Uses union",
		     "930208-1" => "Uses union",
		     "921208-2" => "Uses #define",
		     "921215-1" => "Uses #if",
		     "930106-1" => "Uses #define",
		     "930406-1" => "Uses #if",
		     "930930-2" => "Uses union",
		     "931002-1" => "Uses #if",
		     "931004-10" => "Uses stdarg.h",
		     "931004-12" => "Uses stdarg.h",
		     "931004-14" => "Uses stdarg.h",
		     "931004-2" => "Uses stdarg.h",
		     "931004-4" => "Uses stdarg.h",
		     "931004-6" => "Uses stdarg.h",
		     "931004-8" => "Uses stdarg.h",
		     "931102-1" => "Uses union",
		     "931102-2" => "Uses union",
		     "950221-1" => "Uses #if",
		     "960117-1" => "Uses union",
		     "960311-1" => "Uses #define",
		     "960311-2" => "Uses #define",
		     "960311-3" => "Uses #define",
		     "960405-1" => "Uses #define",
		     "960416-1" => "Uses #define / union",
		     "960419-2" => "Uses #define",
		     "960521-1" => "Uses #define",
		     "960830-1" => "Uses #if",
		     "970214-1" => "Uses #define",
		     "970214-2" => "Uses #define",
		     "980205" => "Uses stdarg.h",
		     "980526-1" => "Uses #if",
		     "980605-1" => "Uses #define",
		     "980608-1" => "Uses stdarg.h",
		     "980716-1" => "Uses stdarg.h",
		     "981001-1" => "Uses #define",
		     "990128-1" => "Uses #define",
		     "990208-1" => "Uses #if",
		     "990211-1" => "Uses #define",
		     "990413-2" => "Uses union",
		     "990531-1" => "Uses union",
		     "990923-1" => "Uses #define",
		     "991014-1" => "Uses #define / union",
		     "991216-1" => "Uses #define",
		     "991216-2" => "Uses #define",
		     "991216-3" => "Uses #define",
		     "991228-1" => "Uses #define / union",
		     "alloca-1" => "Uses #define",
		     "anon-1" => "Uses union",
		     "arith-rand-ll" => "Uses #define",
		     "arith-rand" => "Uses #define",
		     "ashldi-1" => "Uses #define",
		     "ashrdi-1" => "Uses #define",
		     "bcp-1" => "Uses #define",
		     "builtin-abs-1" => "Uses #define",
		     "builtin-abs-2" => "Uses #define",
		     "builtin-complex-1" => "Uses #if",
		     "builtin-constant" => "Uses #define",
		     "builtin-noret-1" => "Uses #if",
		     "builtin-prefetch-1" => "Uses #define",
		     "builtin-prefetch-4" => "Uses #define",
		     "builtin-prefetch-6" => "Uses #define",
		     "cbrt" => "Uses #if / union",
		     "cmpdi-1" => "Uses #define",
		     "comp-goto-1" => "Uses union",
		     "comp-goto-2" => "Uses #define",
		     "compare-3" => "Uses #if",
		     "conversion" => "Uses #if",
		     "comp-goto-1" => "Uses #if",
		     "complex-6" => "Uses #define",
		     "eeprof-1" => "Uses #define",
		     "ffs-2" => "Uses #define",
		     "inst-check" => "Uses stdarg.h",
		     "loop-13" => "Uses #define",
		     "loop-2f" => "Uses #define",
		     "loop-2g" => "Uses #define",
		     "lshrdi-1" => "Uses #define",
		     "memcpy-1" => "Uses #define",
		     "memcpy-2" => "Uses #define / union",
		     "memcpy-bi" => "Uses #define",
		     "memset-1" => "Uses #define / union",
		     "memset-2" => "Uses #define / union",
		     "memset-3" => "Uses #define / union",
		     "nestfunc-4" => "Uses #define",
		     "nestfunc-1" => "Uses #if",
		     "nestfunc-2" => "Uses #if",
		     "nestfunc-3" => "Uses #if",
		     "nest-stdar-1" => "Uses stdarg.h",
		     "pure-1" => "Uses #if",
		     "stdio-opt-1" => "Uses #if",
		     "stdio-opt-2" => "Uses #if",
		     "stdio-opt-3" => "Uses #if",
		     "simd-1" => "Uses union",
		     "simd-2" => "Uses union",
		     "strcmp-1" => "Uses #define / union",
		     "strcpy-1" => "Uses #define / union",
		     "strct-stdarg-1" => "Uses stdarg.h",
		     "strct-varg-1" => "Uses stdarg.h",
		     "string-opt-1" => "Uses #if",
		     "string-opt-10" => "Uses #if",
		     "string-opt-11" => "Uses #if",
		     "string-opt-12" => "Uses #if",
		     "string-opt-13" => "Uses #if",
		     "string-opt-14" => "Uses #if",
		     "string-opt-15" => "Uses #if",
		     "string-opt-16" => "Uses #if",
		     "string-opt-17" => "Uses #if",
		     "string-opt-2" => "Uses #if",
		     "string-opt-3" => "Uses #if",
		     "string-opt-4" => "Uses #if",
		     "string-opt-6" => "Uses #if",
		     "string-opt-7" => "Uses #if",
		     "string-opt-8" => "Uses #if",
		     "string-opt-9" => "Uses #if",
		     "strlen-1" => "Uses #define / union",
		     "strncmp-1" => "Uses #define / union",
		     "tstdi-1" => "Uses #define",
		     "unroll-1" => "Uses inline",
		     "va-arg-1" => "Uses stdarg.h",
		     "va-arg-10" => "Uses stdarg.h",
		     "va-arg-11" => "Uses stdarg.h",
		     "va-arg-12" => "Uses stdarg.h",
		     "va-arg-13" => "Uses stdarg.h",
		     "va-arg-14" => "Uses stdarg.h",
		     "va-arg-15" => "Uses stdarg.h",
		     "va-arg-16" => "Uses stdarg.h",
		     "va-arg-17" => "Uses stdarg.h",
		     "va-arg-18" => "Uses stdarg.h",
		     "va-arg-19" => "Uses stdarg.h",
		     "va-arg-2" => "Uses stdarg.h",
		     "va-arg-20" => "Uses stdarg.h",
		     "va-arg-21" => "Uses stdarg.h",
		     "va-arg-22" => "Uses stdarg.h",
		     "va-arg-23" => "Uses stdarg.h",
		     "va-arg-4" => "Uses stdarg.h",
		     "va-arg-5" => "Uses stdarg.h",
		     "va-arg-6" => "Uses stdarg.h",
		     "va-arg-7" => "Uses stdarg.h",
		     "va-arg-8" => "Uses stdarg.h",
		     "va-arg-9" => "Uses stdarg.h",
		     "widechar-1" => "Uses #define",
		     "zerolen-1" => "Uses union",
		     "alpha" => "beta" 
                    );

if (! -d "generated") {
  mkdir("generated");
}
open my $test_list, '>cod_tests';

foreach my $file (@files) {
  my($filename, $directories, $suffix) = fileparse($file, qr/\.[^.]*/);
  next if ("$suffix" ne ".c");
  print $test_list "$filename\n";
  if (defined $gcc_exceptions{$filename} || defined $tcc_exceptions{$filename}) {
      my $reason;
      open my $filehandle, '>', "$filename.c";
      print $filehandle "#include <stdio.h>\nint main() \n{\n";
      if (defined $gcc_exceptions{$filename}) {
	$reason = $gcc_exceptions{$filename};
      } else {
	$reason = $tcc_exceptions{$filename};
      }
      print $filehandle "    printf(\"Skipping: ", "${reason}", "\\n\");\n";
      print $filehandle "    return 77;\n}\n";
      close $filehandle;
      $expected_failure_count++;
      print "generating skip for $filename\n";
      next;
  }
  if (-f "./pre_patch/$filename.patch") {
    system("cd pre_patch; cp $file . ; patch < $filename.patch");
    $file = "./pre_patch/$filename.c";
    print "pre_patching $filename - ";
  }
  print "generating $filename\n";
  system("perl ./gen_tests.pl $file ./generated > /dev/null");
  system("cp ./generated/$filename.c .");
  if (-f "./post_patch/$filename.patch") {
    system("patch < ./post_patch/$filename.patch");
  }
}
close $test_list;

