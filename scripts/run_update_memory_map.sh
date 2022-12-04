#! /bin/bash

#################################################################################
#
#   the firmware is linked, and device.syms is created with objdump
#   this script greps for the value of __data_start__, __logging_end__ and __logging_start__
#   and replaces $__data_start__ $__logging_end__ and $__logging_start__ in memory_map.yaml
#   then runs the program mpu_calc to create memory_map.h
#   see readme_mpu_calc.md for development notes.
#
# Usage:
#   sh ../scripts/run_update_memory_map.sh symbols_file memory_map.yaml memory_map.h <table_size>
#
# Example:
#   sh ../scripts/run_update_memory_map.sh builds/lab/device.syms memory_map.yaml builds/lab/memory_map.h 15
#
# --> extracting symbols
# arm-none-eabi-objdump -t builds/bin/device_first_pass > builds/bin/device_first_pass.syms
# sh ../scripts/run_update_memory_map.sh \
#      builds/bin/device_first_pass.syms \
#      memory_map.yaml builds/bin/memory_map.h
#
# grep -E \'__data_start__|__logging_end__|__logging_start__\' in builds/bin/device_first_pass.syms
# 004f0000 g       .ctxt_space  00000000 __logging_end__
# 0048ac00 g       .logging 00000000 __logging_start__
# 0044f800 g       .data    00000000 __data_start__
#
# creating /home/wattbyro/dx/builds/bin/search_and_replace.pl
# s/\$__logging_end__/004f0000/;
# s/\$__logging_start__/0048ac00/;
# s/\$__data_start__/0044f800/;
#
# cp memory_map.yaml /home/wattbyro/dx/builds/bin/memory_map.yaml
#
# perl -pi /home/wattbyro/dx/builds/bin/search_and_replace.pl  /home/wattbyro/dx/builds/bin/memory_map.yaml
#
# diff memory_map.yaml /home/wattbyro/dx/builds/bin/memory_map.yaml
# 60c60
# <         comment:          executable and read only for both .text and .rodata (__data_start__=$__data_start__)
# ---
# >         comment:          executable and read only for both .text and .rodata (__data_start__=0044f800)
# 62c62
# <         end_addr:         0x$__data_start__
# ---
# >         end_addr:         0x0044f800
# 68,70c68,70
# <         comment:          stats and logging - write through (__logging_start__=$__logging_start__ .. __logging_end__=$__logging_end__)
# <         start_addr:       0x$__logging_start__
# <         end_addr:         0x$__logging_end__
# ---
# >         comment:          stats and logging - write through (__logging_start__=0048ac00 .. __logging_end__=004f0000)
# >         start_addr:       0x0048ac00
# >         end_addr:         0x004f0000
#
# mpu_calc memory_map=/home/wattbyro/dx/builds/bin/memory_map.yaml output_filename=builds/bin/memory_map.h
# Loading '/home/wattbyro/dx/builds/bin/memory_map.yaml'
#
#  compiling device (-mthumb) mpu_table.cpp (note with -Ibuilds/bin to use new memory_map.h)
#  g++ linking device builds/bin/device
#
################################################################################

mpu_calc=mpu_calc
symbols_file=$1
bin_dir=$(dirname "$(readlink -f "$1")")
template_memory_map_yaml=$2
memory_map_yaml=${bin_dir}/memory_map.yaml
memory_map_h=$3
mpu_table_size=$4
symbols='__data_start__|__logging_end__|__logging_start__'

echo "grep -E \'${symbols}\' in ${symbols_file}"
grep -E ${symbols} ${symbols_file}

echo "creating ${bin_dir}/search_and_replace.pl"
grep -E ${symbols} ${symbols_file} \
 | sed -r 's%^([0-9a-f]*)\s.*\s(__.*)$%s/\\$\2/\1/;%' \
 > ${bin_dir}/search_and_replace.pl

cat ${bin_dir}/search_and_replace.pl

echo "cp ${template_memory_map_yaml} ${memory_map_yaml}"
cp ${template_memory_map_yaml} ${memory_map_yaml}

echo "perl -pi ${bin_dir}/search_and_replace.pl  ${memory_map_yaml}"
perl -pi ${bin_dir}/search_and_replace.pl  ${memory_map_yaml}

echo "diff ${template_memory_map_yaml} ${memory_map_yaml}"
diff ${template_memory_map_yaml} ${memory_map_yaml}

echo "${mpu_calc} memory_map=${memory_map_yaml} output_filename=${memory_map_h} mpu_table_size=${mpu_table_size}"
${mpu_calc} memory_map=${memory_map_yaml} output_filename=${memory_map_h} mpu_table_size=${mpu_table_size}

