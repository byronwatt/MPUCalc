#!/usr/bin/env bats

load "../libs/bats-support/load"
load "../libs/bats-assert/load"

@test "memory map with entry set to all memory set to 'no_access'" {
  run ../../build/mpu_calc memory_map=memory_map.yaml output_filename=memory_map.h
  [ $status -eq 0 ]

  assert_output --partial --stdin <<END
Loading 'memory_map.yaml'
END

  diff memory_map.h expected_memory_map.h
  [ $status -eq 0 ]

}

