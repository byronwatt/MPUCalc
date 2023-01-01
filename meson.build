project('MPUCalc', ['c','cpp'])

if meson.is_cross_build() == true

  # global arguments for both c and c++ (for firmware)
  add_global_arguments( 
	    '-march=armv7e-m',
			'-mtune=cortex-m7',
      '-DMDX2_SMALL_MEMORY',
      '-DMDX2_FREERTOS_TARGET',
	# for some reason the arm port of gcc that we are using
	# defines uint32_t as unsigned long int
	# which is the same as unsigned int in every way except
	# that printf(%x%d%u) of an unsigned long int generates a compiler warning
	# changing the printfs to %lx%ld%lu fixes the problem for the freertos build,
	# but breaks it for the 64 bit linux build.
	# so instead we tell the compiler that uint32_t should be unsigned int instead of unsigned long int
	# and this changes nothing except the erroneous printf warning.
			'-U__UINT32_TYPE__',
			'-D__UINT32_TYPE__=unsigned int',
			'-U__INT32_TYPE__',
			'-D__INT32_TYPE__=int',
      '--specs=nano.specs',
      language:['c','cpp'] )
      
  # global arguments for c++ (for firmware)
  add_global_arguments( 
      '-std=c++2a',   
      # compound assignment with 'volatile'-qualified left operand is deprecated [-Werror=volatile]
      '-Wno-volatile',   
      language: 'cpp' )

  # global arguments for c (for firmware)
  add_global_arguments( 
      '-Wno-unused-value',      
      language: 'c' )
else
      
  # global arguments for c++ (for firmware)
  add_global_arguments( 
      '-std=c++2a',   
      language: 'cpp' )

  # global arguments for c (for firmware)
  add_global_arguments( 
      '-Wno-unused-value',      
      language: 'c' )

  # host compilation
  coverage = get_option('b_coverage')
  if coverage == true
    message( 'b_coverage not true ???' )
    add_global_arguments('-fprofile-abs-path', '-fno-exceptions', '-fno-rtti', language: 'cpp' )
  else
    message( 'b_coverage not true ???' )
  endif
endif

mpucalc_dep = declare_dependency(
    include_directories : [
      'CmdLineOptions/inc',
      'cmsis',
      'src'
    ],
    sources : [
      'src/configure_mpu.cpp',
      'src/mpu_calculator.cpp',
      'src/mpu_display.cpp',
      'src/mpu_table.cpp',
'CmdLineOptions/src/cmd_line_options.cpp',
    ],
)


yaml_dep = declare_dependency(
    include_directories : [
      'libyaml/inc',
    ],
    sources : [
'libyaml/src/libyaml_api.c',
'libyaml/src/libyaml_dumper.c',
'libyaml/src/libyaml_emitter.c',
'libyaml/src/libyaml_loader.c',
'libyaml/src/libyaml_parser.c',
'libyaml/src/libyaml_reader.c',
'libyaml/src/libyaml_scanner.c',
'libyaml/src/libyaml_writer.c',
    ],
)


gtest_dep = declare_dependency(
    include_directories : [
      'gtest/googletest/include',
      'gtest/googletest',
    ],
    sources : [
'gtest/googletest/src/gtest_main.cc',
'gtest/googletest/src/gtest-all.cc',
    ],
)

cmdlineoptions_dep = declare_dependency(
    include_directories : [
      'CmdLineOptions/inc',
    ],
    sources : [
'CmdLineOptions/src/cmd_line_options.cpp',
    ],
)
if meson.is_cross_build() == false
  executable('mpu_calc', 
    'mpu_calc/mpu_calc.cpp',
    dependencies: [mpucalc_dep,
       cmdlineoptions_dep,
       yaml_dep] )
  executable('unit_test',
    'unit_test/mpu_calculator_test.cpp',
    'unit_test/capture_and_compare.cpp',
    dependencies: [mpucalc_dep,
       cmdlineoptions_dep,
       yaml_dep,
       gtest_dep] )
endif