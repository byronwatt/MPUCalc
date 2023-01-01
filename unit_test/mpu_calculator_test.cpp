/* copyright Microchip 2022, MIT License */

/**
* @file
* @brief
* Simple unit test for the embedded_ostream.h
*
* these tests can be run without the simulator like:
*   make builds/sim_reva_pm621x/bin/core_unit_test BUILD=sim_reva_pm621x
*   builds/sim_reva_pm621x/bin/core_unit_test --gtest_list_tests
*   builds/sim_reva_pm621x/bin/core_unit_test --gtest_filter=MPU_CALCULATOR.jira_dx2sw_339
*/
#include "gtest/gtest.h"
#include "mpu_calculator.h"
#include "configure_mpu.h"
#include "mpu_display.h"
#include "capture_and_compare.h"
#include "cmd_line_options.h"
//#include "mdx2_stat.h"

#define EXPECT_HEX_EQ(expected,found)                  EXPECT_PRED_FORMAT2(ExpectHexVal,expected,found)
// A GoogleTest predicate-formatter for asserting that an address is a particular value.
::testing::AssertionResult ExpectHexVal(const char *expected_expr, const char *found_expr, uint32_t expected, uint32_t found);
::testing::AssertionResult ExpectHexVal(const char *expected_expr, const char *found_expr, uint32_t expected, uint32_t found)
{
    if (expected == found)
      return ::testing::AssertionSuccess();

    std::stringstream ss;
    ss << "\n"
        << "===============\n"
        << "Expected: " << expected_expr << "\n"
        << "Expected: 0x" << std::hex << std::setw(8) << std::setfill('0') << expected << "\n"
        << "Found:    " << found_expr << "\n"
        << "Found:    0x" << std::hex << std::setw(8) << std::setfill('0') << found << "\n"
        << "===============\n"
        << "\n"        ;

    return ::testing::AssertionFailure() 
            << ss.str();
}


TEST(MPU_CALCULATOR, write_through)
{
    mpu_calculator_t mpu_calc;
    mpu_calc.mpu_region_number = 11;
    mpu_calc.build_best_mpu_entries(0x0046e800,0x004effff,NEVER_EXECUTE,ARM_MPU_AP_FULL,NORMAL_WRITE_THROUGH_NO_WRITE_ALLOCATE);
    EXPECT_EQ(mpu_calc.num_entries,3UL);
    EXPECT_HEX_EQ(mpu_calc.mpu_table[0].RBAR,0x0048001b);
    EXPECT_HEX_EQ(mpu_calc.mpu_table[0].RASR,0x13068025);
    EXPECT_HEX_EQ(mpu_calc.mpu_table[1].RBAR,0x0047001c);
    EXPECT_HEX_EQ(mpu_calc.mpu_table[1].RASR,0x1306001f);
    EXPECT_HEX_EQ(mpu_calc.mpu_table[2].RBAR,0x0046e01d);
    EXPECT_HEX_EQ(mpu_calc.mpu_table[2].RASR,0x13060319);

    mpu_display_t display;
    for (uint32_t i=0;i<3;i++)
    {
        display.mpu_table[i].RBAR = mpu_calc.mpu_table[i].RBAR;
        display.mpu_table[i].RASR = mpu_calc.mpu_table[i].RASR;
    }
    display.display_memory_map(stdout,"");
    display.display_entries(stdout,"");
}

TEST(MPU_CALCULATOR, device_shareable)
{
    mpu_calculator_t mpu_calc;
    mpu_calc.mpu_region_number = 0;
    mpu_calc.build_best_mpu_entries(0x00f00000,0x02ffffff,NEVER_EXECUTE,ARM_MPU_AP_FULL,NORMAL_WRITE_THROUGH_NO_WRITE_ALLOCATE);
    EXPECT_EQ(mpu_calc.num_entries,2UL);
    EXPECT_HEX_EQ(mpu_calc.mpu_table[0].RBAR,0x00000010);
    EXPECT_HEX_EQ(mpu_calc.mpu_table[0].RASR,0x1306c333);
    EXPECT_HEX_EQ(mpu_calc.mpu_table[1].RBAR,0x00f00011);
    EXPECT_HEX_EQ(mpu_calc.mpu_table[1].RASR,0x13060027);

    mpu_display_t display;
    for (uint32_t i=0;i<mpu_calc.num_entries;i++)
    {
        display.mpu_table[i].RBAR = mpu_calc.mpu_table[i].RBAR;
        display.mpu_table[i].RASR = mpu_calc.mpu_table[i].RASR;
    }
    display.display_memory_map(stdout,"");
    display.display_entries(stdout,"");
}

TEST(MPU_CALCULATOR, after_inbox_outbox)
{
    mpu_calculator_t mpu_calc;
    mpu_calc.mpu_region_number = 0;
    mpu_calc.build_best_mpu_entries(0x004fc000,0x004fffff,NEVER_EXECUTE,ARM_MPU_AP_FULL,NORMAL_WRITE_THROUGH_NO_WRITE_ALLOCATE);
    EXPECT_EQ(mpu_calc.num_entries,1UL);
    EXPECT_HEX_EQ(mpu_calc.mpu_table[0].RBAR,0x004fc010);
    EXPECT_HEX_EQ(mpu_calc.mpu_table[0].RASR,0x1306001b);

    mpu_display_t display;
    for (uint32_t i=0;i<mpu_calc.num_entries;i++)
    {
        display.mpu_table[i].RBAR = mpu_calc.mpu_table[i].RBAR;
        display.mpu_table[i].RASR = mpu_calc.mpu_table[i].RASR;
    }
    display.display_memory_map(stdout,"");
    display.display_entries(stdout,"");
}

TEST(MPU_CALCULATOR, read_only)
{
    mpu_calculator_t mpu_calc;
    mpu_calc.mpu_region_number = 7;
    mpu_calc.build_best_mpu_entries(0x00400000,0x00437c84,NEVER_EXECUTE,ARM_MPU_AP_FULL,NORMAL_WRITE_THROUGH_NO_WRITE_ALLOCATE);
    EXPECT_EQ(mpu_calc.num_entries,4UL);
    EXPECT_HEX_EQ(mpu_calc.mpu_table[0].RBAR,0x00400017);
    EXPECT_HEX_EQ(mpu_calc.mpu_table[0].RASR,0x1306c023);
    EXPECT_HEX_EQ(mpu_calc.mpu_table[1].RBAR,0x00430018);
    EXPECT_HEX_EQ(mpu_calc.mpu_table[1].RASR,0x1306801d);
    EXPECT_HEX_EQ(mpu_calc.mpu_table[2].RBAR,0x00437019);
    EXPECT_HEX_EQ(mpu_calc.mpu_table[2].RASR,0x1306c017);
    EXPECT_HEX_EQ(mpu_calc.mpu_table[3].RBAR,0x00437c1a);
    EXPECT_HEX_EQ(mpu_calc.mpu_table[3].RASR,0x1306f00f);

    mpu_display_t display;
    for (uint32_t i=0;i<mpu_calc.num_entries;i++)
    {
        display.mpu_table[i].RBAR = mpu_calc.mpu_table[i].RBAR;
        display.mpu_table[i].RASR = mpu_calc.mpu_table[i].RASR;
    }
    display.display_memory_map(stdout,"");
    display.display_entries(stdout,"");
}

TEST(MPU_CALCULATOR, display)
{
    mpu_display_t display;
    display.set(0x0,0x00f00000,0x13050027);
    display.set(0x1,0x00000001,0x1305c333);
    display.set(0x2,0x00400002,0x130f0027);
    display.set(0x3,0x004f8003,0x1306001d);
    display.set(0x4,0x004f8004,0x130c001b);
    display.set(0x5,0x004ffc05,0x130f0013);
    display.set(0x6,0xe0000006,0x1304001f);
    display.set(0x7,0x00400007,0x060f8023);
    display.set(0x8,0x00438008,0x060f801b);
    display.set(0x9,0x00000009,0x00000000);
    display.set(0xa,0x0000000a,0x00000000);
    display.set(0xb,0x0048000b,0x13068025);
    display.set(0xc,0x0047000c,0x1306001f);
    display.set(0xd,0x0046e00d,0x13060719);
    display.set(0xe,0x0000000e,0x00000000);
    display.set(0xf,0x0000000f,0x00000000);
    io_redirect_t io_redirect;
    // redirect stdout to a file
    // TODO: maybe redirect to a temporary file
    io_redirect.redirect_start("memory_map.txt");

    display.display_memory_map(stdout,"");
    display.display_entries(stdout,"");

   io_redirect.redirect_end();

// this only works when compiled with a decent version of c++11 (our version of clang doesn't cut it)
#if __GNUC__ >= 5
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
    // check that the following messages were displayed:
    // note: this doesn't check if there are extra message in addition to these,
    // but it at least checks that these are there in the right order.
    // it would be good to upgrade file_match_t to search for an exact match to a multi-line string.
    // and having to change '(' to '\(' is annoying.
    // plus '.' really should be '[.]'
    // and if there were any '*' those would need to be '[*]'
    match_patterns_t patterns[] = {
        {R"~(start    end      size   #  description)~"},
        {R"~(-------- -------- ------ -- -----------)~"},
        {R"~(00000000 003fffff     4M  . unmapped)~"},
        {R"~(00400000 00437fff   224K  7 WRITE_BACK_READ_AND_WRITE_ALLOCATE \(read-only, execute allowed\))~"},
        {R"~(00438000 0043b7ff    14K  8 WRITE_BACK_READ_AND_WRITE_ALLOCATE \(read-only, execute allowed\))~"},
        {R"~(0043b800 0046ebff   205K  2 WRITE_BACK_READ_AND_WRITE_ALLOCATE \(fully cached\))~"},
        {R"~(0046ec00 0046ffff     5K 13 WRITE_THROUGH_NO_WRITE_ALLOCATE \(logging\))~"},
        {R"~(00470000 0047ffff    64K 12 WRITE_THROUGH_NO_WRITE_ALLOCATE \(logging\))~"},
        {R"~(00480000 004effff   448K 11 WRITE_THROUGH_NO_WRITE_ALLOCATE \(logging\))~"},
        {R"~(004f0000 004f7fff    32K  2 WRITE_BACK_READ_AND_WRITE_ALLOCATE \(fully cached\))~"},
        {R"~(004f8000 004fbfff    16K  4 UNCACHED e.g. inbox/outbox, pktmem)~"},
        {R"~(004fc000 004ffbff    15K  3 WRITE_THROUGH_NO_WRITE_ALLOCATE \(logging\))~"},
        {R"~(004ffc00 004fffff     1K  5 WRITE_BACK_READ_AND_WRITE_ALLOCATE \(fully cached\))~"},
        {R"~(00500000 00efffff    10M  . unmapped)~"},
        {R"~(00f00000 00ffffff     1M  0 DEVICE_SHAREABLE)~"},
        {R"~(01000000 02ffffff    32M  1 DEVICE_SHAREABLE)~"},
        {R"~(03000000 dfffffff   3.5G  . unmapped)~"},
        {R"~(e0000000 e000ffff    64K  6 STRONGLY_ORDERED)~"},
        {R"~(e0010000 ffffffff 511.9M  . unmapped)~"},
    } ;
#pragma GCC diagnostic pop

    // scan the captured file looking for the sequence of patterns.
    file_match_t file_match(io_redirect.redirect_filename());

    ASSERT_TRUE( file_match.compare_patterns(patterns,sizeof(patterns)/sizeof(patterns[0])) );

#endif
}

typedef struct {
    uint32_t start_addr;
    uint32_t end_addr;
    const char *description;
} mem_range_t;

static const mem_range_t retimer_memory[] = {
 { 0x00e00000, 0x01000000, "cpu" },
 { 0x01000000, 0x01001000, "top" },
 { 0x01500000, 0x01800000, "m1" },
 { 0x01006000, 0x01007000, "sis" },
 { 0x01008000, 0x01040000, "shi" },
};

static const mem_range_t slice_memory[] = {
 { 0x00e00000, 0x01000000, "cpu" },
 { 0x01000000, 0x01001000, "top" },
 { 0x01500000, 0x01800000, "m1" },
 { 0x01008000, 0x0100C000, "slice_0_shi" },
 { 0x0100E000, 0x01010000, "slice_0_shi" },
 { 0x01006000, 0x01007000, "sis" },
 { 0x01100000, 0x01180000, "slice_0" },
 { 0x011a0000, 0x01200000, "slice_0" },
 { 0x01300000, 0x01400000, "slice_0" },
 { 0x02800000, 0x03000000, "slice_0" },
 { 0x01040000, 0x01080000, "slice_0" },
} ;

static void test_range(const mem_range_t *mem_range )
{
    printf("creating memory region for %s\n",mem_range->description);
    mpu_calculator_t mpu_calc;
    mpu_calc.mpu_region_number = 1;
    mpu_calc.build_best_mpu_entries(mem_range->start_addr,mem_range->end_addr,NEVER_EXECUTE,ARM_MPU_AP_FULL,DEVICE_SHAREABLE);

    mpu_display_t display;
    for (uint32_t i=0;i<mpu_calc.num_entries;i++)
    {
        display.mpu_table[i].RBAR = mpu_calc.mpu_table[i].RBAR;
        display.mpu_table[i].RASR = mpu_calc.mpu_table[i].RASR;
    }
    display.display_memory_map(stdout,"");
    display.display_entries(stdout,"");
}

static void test_ranges(const mem_range_t *mem_ranges, uint32_t num )
{
    for (uint32_t i=0;i<num;i++)
    {
        test_range(&mem_ranges[i]);
    }
}

static const mem_range_t crash_problem[] = {
{ 
    0x00400000, 0x00449200 , "ro" },
} ;
TEST(MPU_CALCULATOR, crash_problem)
{
    test_ranges( &crash_problem[0], sizeof(crash_problem)/sizeof(crash_problem[0]));
}

TEST(MPU_CALCULATOR, retimer)
{
    test_ranges( &retimer_memory[0], sizeof(retimer_memory)/sizeof(retimer_memory[0]));
}

TEST(MPU_CALCULATOR, slice)
{
    test_ranges( &slice_memory[0], sizeof(slice_memory)/sizeof(slice_memory[0]));
}

TEST(MPU_CALCULATOR, jira_dx2sw_339)
{
    mpu_calculator_t mpu_calc;
    mpu_calc.mpu_region_number = 0;
    mpu_calc.build_best_mpu_entries(0x00498c00,0x0060ffff,NEVER_EXECUTE,ARM_MPU_AP_FULL,NORMAL_WRITE_THROUGH_NO_WRITE_ALLOCATE);
    //EXPECT_EQ(mpu_calc.num_entries,2UL);
    //EXPECT_HEX_EQ(mpu_calc.mpu_table[0].RBAR,0x00000010);
    //EXPECT_HEX_EQ(mpu_calc.mpu_table[0].RASR,0x1306c333);
    //EXPECT_HEX_EQ(mpu_calc.mpu_table[1].RBAR,0x00f00011);
    //EXPECT_HEX_EQ(mpu_calc.mpu_table[1].RASR,0x13060027);

    mpu_display_t display;
    for (uint32_t i=0;i<mpu_calc.num_entries;i++)
    {
        display.mpu_table[i].RBAR = mpu_calc.mpu_table[i].RBAR;
        display.mpu_table[i].RASR = mpu_calc.mpu_table[i].RASR;
    }
    uint32_t first_addr, last_addr;
    display.get_first_and_last_address(&first_addr,&last_addr);
    EXPECT_HEX_EQ(first_addr,0x00498c00);
    EXPECT_HEX_EQ(last_addr,0x0060ffff);
    display.display_memory_map(stdout,"");
    display.display_entries(stdout,"");
}

/*
 * test covering 0xfd00 .. 0x10100
 */
TEST(MPU_CALCULATOR, jira_dx2sw_339_a)
{
    mpu_calculator_t mpu_calc;
    mpu_calc.mpu_region_number = 0;
    mpu_calc.build_best_mpu_entries(0xfd00,0x100ff,NEVER_EXECUTE,ARM_MPU_AP_FULL,NORMAL_WRITE_THROUGH_NO_WRITE_ALLOCATE);

    mpu_display_t display;
    for (uint32_t i=0;i<mpu_calc.num_entries;i++)
    {
        display.mpu_table[i].RBAR = mpu_calc.mpu_table[i].RBAR;
        display.mpu_table[i].RASR = mpu_calc.mpu_table[i].RASR;
    }
    uint32_t first_addr, last_addr;
    display.get_first_and_last_address(&first_addr,&last_addr);
    EXPECT_HEX_EQ(first_addr,0xfd00);
    EXPECT_HEX_EQ(last_addr,0x100ff);
    display.display_memory_map(stdout,"");
    display.display_entries(stdout,"");
}

/*
 * test covering 0xff00 .. 0x10300
 */
TEST(MPU_CALCULATOR, jira_dx2sw_339_b)
{
    mpu_calculator_t mpu_calc;
    mpu_calc.mpu_region_number = 0;
    mpu_calc.build_best_mpu_entries(0xff00,0x102ff,NEVER_EXECUTE,ARM_MPU_AP_FULL,NORMAL_WRITE_THROUGH_NO_WRITE_ALLOCATE);

    mpu_display_t display;
    for (uint32_t i=0;i<mpu_calc.num_entries;i++)
    {
        display.mpu_table[i].RBAR = mpu_calc.mpu_table[i].RBAR;
        display.mpu_table[i].RASR = mpu_calc.mpu_table[i].RASR;
    }
    uint32_t first_addr, last_addr;
    display.get_first_and_last_address(&first_addr,&last_addr);
    EXPECT_HEX_EQ(first_addr,0xff00);
    EXPECT_HEX_EQ(last_addr,0x102ff);
    display.display_memory_map(stdout,"");
    display.display_entries(stdout,"");
}


/*
 * test covering 0x09c0 .. 0x09e0
 */
TEST(MPU_CALCULATOR, random_fail)
{
    mpu_calculator_t mpu_calc;
    mpu_calc.mpu_region_number = 0;
    uint32_t start_addr = 0x09c0;
    uint32_t end_addr = 0x09df;

    mpu_calc.build_best_mpu_entries(start_addr,end_addr,NEVER_EXECUTE,ARM_MPU_AP_FULL,NORMAL_WRITE_THROUGH_NO_WRITE_ALLOCATE);

    mpu_display_t display;
    for (uint32_t i=0;i<mpu_calc.num_entries;i++)
    {
        display.mpu_table[i].RBAR = mpu_calc.mpu_table[i].RBAR;
        display.mpu_table[i].RASR = mpu_calc.mpu_table[i].RASR;
    }
    uint32_t first_addr, last_addr;
    display.get_first_and_last_address(&first_addr,&last_addr);
    EXPECT_HEX_EQ(first_addr,start_addr);
    EXPECT_HEX_EQ(last_addr,end_addr);
    display.display_memory_map(stdout,"");
    display.display_entries(stdout,"");
}
/*
 * test covering 0x79e2a9c0 .. 0x79e2a9e0
 */
TEST(MPU_CALCULATOR, random_fail3)
{
    mpu_calculator_t mpu_calc;
    mpu_calc.mpu_region_number = 0;
    uint32_t start_addr = 0x79e2a9c0;
    uint32_t end_addr = 0x79e2a9df;

    mpu_calc.build_best_mpu_entries(start_addr,end_addr,NEVER_EXECUTE,ARM_MPU_AP_FULL,NORMAL_WRITE_THROUGH_NO_WRITE_ALLOCATE);

    mpu_display_t display;
    for (uint32_t i=0;i<mpu_calc.num_entries;i++)
    {
        display.mpu_table[i].RBAR = mpu_calc.mpu_table[i].RBAR;
        display.mpu_table[i].RASR = mpu_calc.mpu_table[i].RASR;
    }
    uint32_t first_addr, last_addr;
    display.get_first_and_last_address(&first_addr,&last_addr);
    EXPECT_HEX_EQ(first_addr,start_addr);
    EXPECT_HEX_EQ(last_addr,end_addr);
    display.display_memory_map(stdout,"");
    display.display_entries(stdout,"");
}



/*
 * test covering 0x7545e140 .. 0x79e2a9e0
 */
TEST(MPU_CALCULATOR, random_fail2)
{
    mpu_calculator_t mpu_calc;
    mpu_calc.mpu_region_number = 0;
    uint32_t start_addr = 0x7545e140;
    uint32_t end_addr = 0x79e2a9df;

    mpu_calc.build_best_mpu_entries(start_addr,end_addr,NEVER_EXECUTE,ARM_MPU_AP_FULL,NORMAL_WRITE_THROUGH_NO_WRITE_ALLOCATE);

    mpu_display_t display;
    for (uint32_t i=0;i<mpu_calc.num_entries;i++)
    {
        display.mpu_table[i].RBAR = mpu_calc.mpu_table[i].RBAR;
        display.mpu_table[i].RASR = mpu_calc.mpu_table[i].RASR;
    }
    uint32_t first_addr, last_addr;
    display.get_first_and_last_address(&first_addr,&last_addr);
    EXPECT_HEX_EQ(first_addr,start_addr);
    EXPECT_HEX_EQ(last_addr,end_addr);
    display.display_memory_map(stdout,"");
    display.display_entries(stdout,"");
}

/*
 * test covering 0x327b23c0 .. 0xffffffe7
 */
TEST(MPU_CALCULATOR, random_fail4)
{
    mpu_calculator_t mpu_calc;
    mpu_calc.mpu_region_number = 0;

    uint32_t start_addr = 0x327b23c0;
    uint32_t end_addr = 0xffffffff;

    mpu_calc.build_best_mpu_entries(start_addr,end_addr,NEVER_EXECUTE,ARM_MPU_AP_FULL,NORMAL_WRITE_THROUGH_NO_WRITE_ALLOCATE);

    mpu_display_t display;
    for (uint32_t i=0;i<mpu_calc.num_entries;i++)
    {
        display.mpu_table[i].RBAR = mpu_calc.mpu_table[i].RBAR;
        display.mpu_table[i].RASR = mpu_calc.mpu_table[i].RASR;
    }
    uint32_t first_addr, last_addr;
    display.get_first_and_last_address(&first_addr,&last_addr);
    EXPECT_HEX_EQ(first_addr,start_addr);
    EXPECT_HEX_EQ(last_addr,end_addr);
    display.display_memory_map(stdout,"");
    display.display_entries(stdout,"");
}

//UintOption option_num_random_iterations( 1000000, "random_iterations", "number of iterations for the random test" );
UintOption option_num_random_iterations( 1000, "random_iterations", "number of iterations for the random test" );

// this option was added to avoid running the random test with valgrind for an enormous amount of time (10 minutes)
DoubleOption option_test_timeout( 5.0, "test_timeout", "maximum test time in seconds");

/*
 * test mpu calculator for random start and end addresses (cache line aligned)
 */
TEST(MPU_CALCULATOR, random)
{
    //mdx2_stat_t duration = {0,"test_time",""};
    //mdx2_stat_mark_start(&duration);
    uint32_t entry_overflows = 0;
    uint32_t num_tests = 0;
    FILE *f = fopen("/dev/null","w");
    for (uint32_t itr=0;itr<option_num_random_iterations.value;itr++)
    {
        mpu_calculator_t mpu_calc;
        uint32_t start_addr;
        uint32_t end_addr;
        // show progress every 2^n iterations
        if (__builtin_popcount(itr) == 1)
        {
            printf("random test count=%d, entry_overflows=%d\n",itr,entry_overflows);

#if 0
            if (mdx2_stat_running_time(&duration) > option_test_timeout.value * 1000 * 1000)
            {
                printf("exiting after %0.1f seconds\n",mdx2_stat_running_time(&duration)/1000.0/1000.0);
                break;
            }
#endif
        }
        while(1)
        {
            uint32_t x = random();
            uint32_t y = random();
            if (x > y) {
                start_addr = y & ~31;
                end_addr = x | 31;
            } else {
                start_addr = x & ~31;
                end_addr = y | 31;
            }
            if (start_addr < end_addr)
            {
                break;
            }
        }
        mpu_calc.mpu_region_number = 0;
        //printf("start_addr = %x, end_addr = %x\n",start_addr,end_addr);
        if (!mpu_calc.build_best_mpu_entries(start_addr,end_addr,NEVER_EXECUTE,ARM_MPU_AP_FULL,NORMAL_WRITE_THROUGH_NO_WRITE_ALLOCATE))
        {
            entry_overflows++;
            //probably used more than 16 entries.
            EXPECT_EQ(mpu_calc.num_entries,16UL); 
            if (HasFailure())
            {
                printf("failed on iteration %d\n",itr);
                printf("start_addr = %x, end_addr = %x\n",start_addr,end_addr);
                break;
            }
        }
        else
        {
            num_tests++;
            mpu_display_t display;
            for (uint32_t i=0;i<mpu_calc.num_entries;i++)
            {
                display.mpu_table[i].RBAR = mpu_calc.mpu_table[i].RBAR;
                display.mpu_table[i].RASR = mpu_calc.mpu_table[i].RASR;
            }
            uint32_t first_addr, last_addr;
            display.get_first_and_last_address(&first_addr,&last_addr);
            EXPECT_HEX_EQ(first_addr,start_addr);
            EXPECT_HEX_EQ(last_addr,end_addr);
            //display.display_memory_map(f,"");
            //display.display_entries(f,"");
            if (HasFailure())
            {
                printf("failed on iteration %d\n",itr);
                printf("start_addr = %x, end_addr = %x\n",start_addr,end_addr);
                display.display_memory_map(stdout,"");
                display.display_entries(stdout,"");
                break;
            }
        }
    }
    fclose(f);
    //EXPECT_EQ(mpu_calc.num_entries,2UL);
    //EXPECT_HEX_EQ(mpu_calc.mpu_table[0].RBAR,0x00000010);
    //EXPECT_HEX_EQ(mpu_calc.mpu_table[0].RASR,0x1306c333);
    //EXPECT_HEX_EQ(mpu_calc.mpu_table[1].RBAR,0x00f00011);
    //EXPECT_HEX_EQ(mpu_calc.mpu_table[1].RASR,0x13060027);

}

//UintOption option_num_random_display_iterations( 100000, "random_display_iterations", "number of iterations for the random test" );
UintOption option_num_random_display_iterations( 1000, "random_display_iterations", "number of iterations for the random test" );


/*
 * test mpu calculator and mpu_display for random start and end addresses (cache line aligned)
 */
TEST(MPU_CALCULATOR, random_displays)
{
    //mdx2_stat_t duration = {0,"test_time",""};
    //mdx2_stat_mark_start(&duration);
    uint32_t entry_overflows = 0;
    uint32_t num_tests = 0;
    FILE *f = fopen("/dev/null","w");
    for (uint32_t itr=0;itr<option_num_random_display_iterations.value;itr++)
    {
        mpu_calculator_t mpu_calc;
        uint32_t start_addr;
        uint32_t end_addr;
        // show progress every 2^n iterations
        if (__builtin_popcount(itr) == 1)
        {
            printf("random display test count=%d, entry_overflows=%d\n",itr,entry_overflows);
            //if (mdx2_stat_running_time(&duration) > option_test_timeout.value * 1000 * 1000)
            //{
            //    printf("exiting after %0.1f seconds\n",mdx2_stat_running_time(&duration)/1000.0/1000.0);
            //    break;
            //}
        }
        while(1)
        {
            uint32_t x = random();
            uint32_t y = random();
            if (x > y) {
                start_addr = y & ~31;
                end_addr = x | 31;
            } else {
                start_addr = x & ~31;
                end_addr = y | 31;
            }
            if (start_addr < end_addr)
            {
                break;
            }
        }
        mpu_calc.mpu_region_number = 0;
        if (!mpu_calc.build_best_mpu_entries(start_addr,end_addr,NEVER_EXECUTE,ARM_MPU_AP_FULL,NORMAL_WRITE_THROUGH_NO_WRITE_ALLOCATE))
        {
            entry_overflows++;
            //probably used more than 16 entries.
            EXPECT_EQ(mpu_calc.num_entries,16UL); 
            if (HasFailure())
            {
                printf("failed on iteration %d\n",itr);
                printf("start_addr = %x, end_addr = %x\n",start_addr,end_addr);
                break;
            }
        }
        else
        {
            num_tests++;
            mpu_display_t display;
            for (uint32_t i=0;i<mpu_calc.num_entries;i++)
            {
                display.mpu_table[i].RBAR = mpu_calc.mpu_table[i].RBAR;
                display.mpu_table[i].RASR = mpu_calc.mpu_table[i].RASR;
            }
            uint32_t first_addr, last_addr;
            display.get_first_and_last_address(&first_addr,&last_addr);
            EXPECT_HEX_EQ(first_addr,start_addr);
            EXPECT_HEX_EQ(last_addr,end_addr);
            display.display_memory_map(f,"");
            display.display_entries(f,"");
            if (HasFailure())
            {
                printf("failed on iteration %d\n",itr);
                printf("start_addr = %x, end_addr = %x\n",start_addr,end_addr);
                display.display_memory_map(stdout,"");
                display.display_entries(stdout,"");
                break;
            }
        }
    }
    fclose(f);
    //EXPECT_EQ(mpu_calc.num_entries,2UL);
    //EXPECT_HEX_EQ(mpu_calc.mpu_table[0].RBAR,0x00000010);
    //EXPECT_HEX_EQ(mpu_calc.mpu_table[0].RASR,0x1306c333);
    //EXPECT_HEX_EQ(mpu_calc.mpu_table[1].RBAR,0x00f00011);
    //EXPECT_HEX_EQ(mpu_calc.mpu_table[1].RASR,0x13060027);

}



TEST(MPU_CALCULATOR, no_access_for_region_0)
{
    mpu_calculator_t mpu_calc;
    mpu_calc.mpu_region_number = 0;
    mpu_calc.build_best_mpu_entries(0x0,0xffffffff,NEVER_EXECUTE,ARM_MPU_AP_NONE,NO_ACCESS);
    //EXPECT_EQ(mpu_calc.num_entries,2UL);
    //EXPECT_HEX_EQ(mpu_calc.mpu_table[0].RBAR,0x00000010);
    //EXPECT_HEX_EQ(mpu_calc.mpu_table[0].RASR,0x1306c333);
    //EXPECT_HEX_EQ(mpu_calc.mpu_table[1].RBAR,0x00f00011);
    //EXPECT_HEX_EQ(mpu_calc.mpu_table[1].RASR,0x13060027);

    mpu_display_t display;
    for (uint32_t i=0;i<mpu_calc.num_entries;i++)
    {
        display.mpu_table[i].RBAR = mpu_calc.mpu_table[i].RBAR;
        display.mpu_table[i].RASR = mpu_calc.mpu_table[i].RASR;
    }
    uint32_t first_addr, last_addr;
    display.get_first_and_last_address(&first_addr,&last_addr);
    EXPECT_HEX_EQ(first_addr,0x0);
    EXPECT_HEX_EQ(last_addr,0xffffffff);
    display.display_memory_map(stdout,"");
    display.display_entries(stdout,"");
}
TEST(MPU_CALCULATOR, close_to_end)
{
    mpu_calculator_t mpu_calc;
    mpu_calc.mpu_region_number = 0;
    mpu_calc.build_best_mpu_entries(0x7fff0000,0xffffffff,NEVER_EXECUTE,ARM_MPU_AP_NONE,NO_ACCESS);
    //EXPECT_EQ(mpu_calc.num_entries,2UL);
    //EXPECT_HEX_EQ(mpu_calc.mpu_table[0].RBAR,0x00000010);
    //EXPECT_HEX_EQ(mpu_calc.mpu_table[0].RASR,0x1306c333);
    //EXPECT_HEX_EQ(mpu_calc.mpu_table[1].RBAR,0x00f00011);
    //EXPECT_HEX_EQ(mpu_calc.mpu_table[1].RASR,0x13060027);

    mpu_display_t display;
    for (uint32_t i=0;i<mpu_calc.num_entries;i++)
    {
        display.mpu_table[i].RBAR = mpu_calc.mpu_table[i].RBAR;
        display.mpu_table[i].RASR = mpu_calc.mpu_table[i].RASR;
    }
    uint32_t first_addr, last_addr;
    display.get_first_and_last_address(&first_addr,&last_addr);
    EXPECT_HEX_EQ(first_addr,0x7fff0000);
    EXPECT_HEX_EQ(last_addr,0xffffffff);
    display.display_memory_map(stdout,"");
    display.display_entries(stdout,"");
}
/*
*/