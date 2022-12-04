/* copyright Microchip 2022, MIT License */
/*
 * sample memory map for lite that gets created in builds/freertos/bin
 *
 * (this version is used for the first pass firmware linking)
 */

// start    end      size   #  description
// -------- -------- ------ -- -----------
// 00000000 003fffff     4M  . unmapped
// 00400000 0043ffff   256K  3 WRITE_BACK_READ_AND_WRITE_ALLOCATE (read-only, execute allowed)
// 00440000 0044dfff    56K  4 WRITE_BACK_READ_AND_WRITE_ALLOCATE (read-only, execute allowed)
// 0044e000 0044f7ff     6K  5 WRITE_BACK_READ_AND_WRITE_ALLOCATE (read-only, execute allowed)
// 0044f800 004867ff   220K  2 WRITE_BACK_READ_AND_WRITE_ALLOCATE (fully cached)
// 00486800 00487fff     6K  8 WRITE_THROUGH_NO_WRITE_ALLOCATE (logging)
// 00488000 0048ffff    32K  7 WRITE_THROUGH_NO_WRITE_ALLOCATE (logging)
// 00490000 004effff   384K  6 WRITE_THROUGH_NO_WRITE_ALLOCATE (logging)
// 004f0000 004f7fff    32K  2 WRITE_BACK_READ_AND_WRITE_ALLOCATE (fully cached)
// 004f8000 004fbfff    16K 10 UNCACHED (inbox/outbox)
// 004fc000 004fffff    16K  9 WRITE_THROUGH_NO_WRITE_ALLOCATE (logging)
// 00500000 00efffff    10M  . unmapped
// 00f00000 00ffffff     1M  1 DEVICE_SHAREABLE
// 01000000 02ffffff    32M  0 DEVICE_SHAREABLE
// 03000000 fffffffc     4G  . unmapped

    // DEV_CFG
    // 0: 0x00000000, size=64M, XN=1, AP=0x3, TEX=0x0, S=0x1, C=0x0, B=0x1, SRD=0xc3
    //    subregion_size=8M, subregions=0x3c
    //    region mask enabled start      end
    //    ------ ---- ------- ---------- ----------
    //       0   0x01    N    0x00000000 0x007fffff
    //       1   0x02    N    0x00800000 0x00ffffff
    //       2   0x04    Y    0x01000000 0x017fffff <-- enabled
    //       3   0x08    Y    0x01800000 0x01ffffff <-- enabled
    //       4   0x10    Y    0x02000000 0x027fffff <-- enabled
    //       5   0x20    Y    0x02800000 0x02ffffff <-- enabled
    //       6   0x40    N    0x03000000 0x037fffff
    //       7   0x80    N    0x03800000 0x03ffffff
    {
        .RBAR = ARM_MPU_RBAR(0UL, 0x00000000UL),
        .RASR = ARM_MPU_RASR_EX(NEVER_EXECUTE, ARM_MPU_AP_FULL, DEVICE_SHAREABLE, 0xc3, ARM_MPU_REGION_SIZE_64MB)
    },
    // DEV_CFG
    // 1: 0x00f00000, size=1M, XN=1, AP=0x3, TEX=0x0, S=0x1, C=0x0, B=0x1, SRD=0x0
    {
        .RBAR = ARM_MPU_RBAR(1UL, 0x00f00000UL),
        .RASR = ARM_MPU_RASR_EX(NEVER_EXECUTE, ARM_MPU_AP_FULL, DEVICE_SHAREABLE, 0x0, ARM_MPU_REGION_SIZE_1MB)
    },
    // OCR (default is shareable and works for LDREX/STREX, does it work for jtag?)
    // 2: 0x00400000, size=1M, XN=1, AP=0x3, TEX=0x1, S=0x1, C=0x1, B=0x1, SRD=0x0
    {
        .RBAR = ARM_MPU_RBAR(2UL, 0x00400000UL),
        .RASR = ARM_MPU_RASR_EX(NEVER_EXECUTE, ARM_MPU_AP_FULL, NORMAL_WRITE_BACK_READ_AND_WRITE_ALLOCATE, 0x0, ARM_MPU_REGION_SIZE_1MB)
    },
    // executable and read only for both .text and .rodata (__data_start__ = 0044f800)
    // 3: 0x00400000, size=256K, XN=0, AP=0x6, TEX=0x1, S=0x1, C=0x1, B=0x1, SRD=0x0
    {
        .RBAR = ARM_MPU_RBAR(3UL, 0x00400000UL),
        .RASR = ARM_MPU_RASR_EX(EXECUTE, ARM_MPU_AP_RO, NORMAL_WRITE_BACK_READ_AND_WRITE_ALLOCATE, 0x0, ARM_MPU_REGION_SIZE_256KB)
    },
    // executable and read only for both .text and .rodata (__data_start__ = 0044f800)
    // 4: 0x00440000, size=64K, XN=0, AP=0x6, TEX=0x1, S=0x1, C=0x1, B=0x1, SRD=0x80
    //    subregion_size=8K, subregions=0x7f
    //    region mask enabled start      end
    //    ------ ---- ------- ---------- ----------
    //       0   0x01    Y    0x00440000 0x00441fff <-- enabled
    //       1   0x02    Y    0x00442000 0x00443fff <-- enabled
    //       2   0x04    Y    0x00444000 0x00445fff <-- enabled
    //       3   0x08    Y    0x00446000 0x00447fff <-- enabled
    //       4   0x10    Y    0x00448000 0x00449fff <-- enabled
    //       5   0x20    Y    0x0044a000 0x0044bfff <-- enabled
    //       6   0x40    Y    0x0044c000 0x0044dfff <-- enabled
    //       7   0x80    N    0x0044e000 0x0044ffff
    {
        .RBAR = ARM_MPU_RBAR(4UL, 0x00440000UL),
        .RASR = ARM_MPU_RASR_EX(EXECUTE, ARM_MPU_AP_RO, NORMAL_WRITE_BACK_READ_AND_WRITE_ALLOCATE, 0x80, ARM_MPU_REGION_SIZE_64KB)
    },
    // executable and read only for both .text and .rodata (__data_start__ = 0044f800)
    // 5: 0x0044e000, size=8K, XN=0, AP=0x6, TEX=0x1, S=0x1, C=0x1, B=0x1, SRD=0xc0
    //    subregion_size=1K, subregions=0x3f
    //    region mask enabled start      end
    //    ------ ---- ------- ---------- ----------
    //       0   0x01    Y    0x0044e000 0x0044e3ff <-- enabled
    //       1   0x02    Y    0x0044e400 0x0044e7ff <-- enabled
    //       2   0x04    Y    0x0044e800 0x0044ebff <-- enabled
    //       3   0x08    Y    0x0044ec00 0x0044efff <-- enabled
    //       4   0x10    Y    0x0044f000 0x0044f3ff <-- enabled
    //       5   0x20    Y    0x0044f400 0x0044f7ff <-- enabled
    //       6   0x40    N    0x0044f800 0x0044fbff
    //       7   0x80    N    0x0044fc00 0x0044ffff
    {
        .RBAR = ARM_MPU_RBAR(5UL, 0x0044e000UL),
        .RASR = ARM_MPU_RASR_EX(EXECUTE, ARM_MPU_AP_RO, NORMAL_WRITE_BACK_READ_AND_WRITE_ALLOCATE, 0xc0, ARM_MPU_REGION_SIZE_8KB)
    },
    // stats and logging - write through (__logging_start__ = 00486800 .. __logging_end__=004f0000)
    // 6: 0x00480000, size=512K, XN=1, AP=0x3, TEX=0x0, S=0x1, C=0x1, B=0x0, SRD=0x81
    //    subregion_size=64K, subregions=0x7e
    //    region mask enabled start      end
    //    ------ ---- ------- ---------- ----------
    //       0   0x01    N    0x00480000 0x0048ffff
    //       1   0x02    Y    0x00490000 0x0049ffff <-- enabled
    //       2   0x04    Y    0x004a0000 0x004affff <-- enabled
    //       3   0x08    Y    0x004b0000 0x004bffff <-- enabled
    //       4   0x10    Y    0x004c0000 0x004cffff <-- enabled
    //       5   0x20    Y    0x004d0000 0x004dffff <-- enabled
    //       6   0x40    Y    0x004e0000 0x004effff <-- enabled
    //       7   0x80    N    0x004f0000 0x004fffff
    {
        .RBAR = ARM_MPU_RBAR(6UL, 0x00480000UL),
        .RASR = ARM_MPU_RASR_EX(NEVER_EXECUTE, ARM_MPU_AP_FULL, NORMAL_WRITE_THROUGH_NO_WRITE_ALLOCATE, 0x81, ARM_MPU_REGION_SIZE_512KB)
    },
    // stats and logging - write through (__logging_start__ = 00486800 .. __logging_end__=004f0000)
    // 7: 0x00488000, size=32K, XN=1, AP=0x3, TEX=0x0, S=0x1, C=0x1, B=0x0, SRD=0x0
    {
        .RBAR = ARM_MPU_RBAR(7UL, 0x00488000UL),
        .RASR = ARM_MPU_RASR_EX(NEVER_EXECUTE, ARM_MPU_AP_FULL, NORMAL_WRITE_THROUGH_NO_WRITE_ALLOCATE, 0x0, ARM_MPU_REGION_SIZE_32KB)
    },
    // stats and logging - write through (__logging_start__ = 00486800 .. __logging_end__=004f0000)
    // 8: 0x00486000, size=8K, XN=1, AP=0x3, TEX=0x0, S=0x1, C=0x1, B=0x0, SRD=0x3
    //    subregion_size=1K, subregions=0xfc
    //    region mask enabled start      end
    //    ------ ---- ------- ---------- ----------
    //       0   0x01    N    0x00486000 0x004863ff
    //       1   0x02    N    0x00486400 0x004867ff
    //       2   0x04    Y    0x00486800 0x00486bff <-- enabled
    //       3   0x08    Y    0x00486c00 0x00486fff <-- enabled
    //       4   0x10    Y    0x00487000 0x004873ff <-- enabled
    //       5   0x20    Y    0x00487400 0x004877ff <-- enabled
    //       6   0x40    Y    0x00487800 0x00487bff <-- enabled
    //       7   0x80    Y    0x00487c00 0x00487fff <-- enabled
    {
        .RBAR = ARM_MPU_RBAR(8UL, 0x00486000UL),
        .RASR = ARM_MPU_RASR_EX(NEVER_EXECUTE, ARM_MPU_AP_FULL, NORMAL_WRITE_THROUGH_NO_WRITE_ALLOCATE, 0x3, ARM_MPU_REGION_SIZE_8KB)
    },
    // after the inbox/outbox is mdx2_device_log_buffers_t which contains a description of where to find logging & stats and assert message, but we could just make it uncached, or manually flush
    // 9: 0x004f8000, size=32K, XN=1, AP=0x3, TEX=0x0, S=0x1, C=0x1, B=0x0, SRD=0x0
    {
        .RBAR = ARM_MPU_RBAR(9UL, 0x004f8000UL),
        .RASR = ARM_MPU_RASR_EX(NEVER_EXECUTE, ARM_MPU_AP_FULL, NORMAL_WRITE_THROUGH_NO_WRITE_ALLOCATE, 0x0, ARM_MPU_REGION_SIZE_32KB)
    },
    // inbox/outbox (hostmsg request/response) currently configured as uncached, but we should manually do the cache flush/invalidate.
    // 10: 0x004f8000, size=16K, XN=1, AP=0x3, TEX=0x1, S=0x1, C=0x0, B=0x0, SRD=0x0
    {
        .RBAR = ARM_MPU_RBAR(10UL, 0x004f8000UL),
        .RASR = ARM_MPU_RASR_EX(NEVER_EXECUTE, ARM_MPU_AP_FULL, NORMAL_UNCACHED, 0x0, ARM_MPU_REGION_SIZE_16KB)
    },
    // unused
    {
        .RBAR = ARM_MPU_RBAR(11UL, 0x00000000UL),
        .RASR = 0
    },
    // unused
    {
        .RBAR = ARM_MPU_RBAR(12UL, 0x00000000UL),
        .RASR = 0
    },
    // unused
    {
        .RBAR = ARM_MPU_RBAR(13UL, 0x00000000UL),
        .RASR = 0
    },
