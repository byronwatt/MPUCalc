#!/usr/bin/env bats

load "../libs/bats-support/load"
load "../libs/bats-assert/load"

@test "bad addr" {
cat > memory_map.yaml << END
region:
  start_addr:       hello
END
  run ../../build/mpu_calc memory_map=memory_map.yaml output_filename=memory_map.h
  [ $status -eq 255 ]

  assert_output  --stdin <<END
Loading 'memory_map.yaml'
expected decimal in 'hello' at line 1
END
}


@test "bad DisableExec" {
cat > memory_map.yaml << END
region:
  DisableExec:      hello
END
  run ../../build/mpu_calc memory_map=memory_map.yaml output_filename=memory_map.h
  [ $status -eq 255 ]

  assert_output  --stdin <<END
Loading 'memory_map.yaml'
unknown DisableExec in 'hello' at line 1
valid values:
    EXECUTE
    NEVER_EXECUTE
END
}

@test "bad AccessAttributes" {
cat > memory_map.yaml << END
region:
  AccessAttributes:      hello
END
  run ../../build/mpu_calc memory_map=memory_map.yaml output_filename=memory_map.h
  [ $status -eq 255 ]

  assert_output  --stdin <<END
Loading 'memory_map.yaml'
unknown AccessAttributes in 'hello' at line 1
valid values:
    NO_ACCESS
    STRONGLY_ORDERED
    DEVICE_SHAREABLE
    DEVICE_NON_SHAREABLE
    NORMAL_UNCACHED
    NORMAL_WRITE_THROUGH_NO_WRITE_ALLOCATE
    NORMAL_WRITE_BACK_NO_WRITE_ALLOCATE
    NORMAL_WRITE_BACK_READ_AND_WRITE_ALLOCATE
    NORMAL_WRITE_BACK_READ_AND_WRITE_ALLOCATE_NON_SHAREABLE
    UNCACHED
    WRITE_THROUGH_NO_WRITE_ALLOCATE
    WRITE_BACK_NO_WRITE_ALLOCATE
    WRITE_BACK_READ_AND_WRITE_ALLOCATE
    WRITE_BACK_READ_AND_WRITE_ALLOCATE_NON_SHAREABLE
END
}


@test "bad AccessPermission" {
cat > memory_map.yaml << END
region:
  AccessPermission:      hello
END
  run ../../build/mpu_calc memory_map=memory_map.yaml output_filename=memory_map.h
  [ $status -eq 255 ]

  assert_output  --stdin <<END
Loading 'memory_map.yaml'
unknown AccessPermission in 'hello' at line 1
valid values:
    ARM_MPU_AP_RO
    ARM_MPU_AP_NONE
    ARM_MPU_AP_FULL
END
}
