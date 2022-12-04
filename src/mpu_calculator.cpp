/* copyright Microchip 2022, MIT License */
/**
* @file
* @brief
*   class for calculating mpu regions
*/

#include "mpu_calculator.h"
#include "dbg_log.h"
#include "mdx2_sys.h"

#define DEBUG_LOG_STRING(x) do { } while (0)
#define DEBUG_PRINTF(...) do {} while (0)
/**
#define DEBUG_LOG_STRING(x) do { \
printf("%s = 0x%08x\n",#x,x); \
} while(0)
#define DEBUG_PRINTF() printf
*/

/*
 * use one more more regions to configure memory as read only
 * note: you could mix positive and negative regions but let's just use positive regions to build up the range of memory,
 * because the math is simpler.
 *
 * this code uses 1 region per 3 bits in the address.
 *
 * note: assumes start_addr will be naturally aligned with whatever size is picked.
 *
 * note: this function is not used anymore since build_best_mpu_entries() works with arbitrary
 *  alignment of start_addr and end_addr.
 *
 * note: this function is faster than build_best_mpu_entries()
 */
void mpu_calculator_t::find_size_and_subregions()
{
    uint32_t actual_size = end_addr - start_addr;

    // assumes start_addr is naturally aligned.
    region_start_addr = start_addr;
    DEBUG_LOG_STRING(actual_size);
    // e.g. 
    //  start_addr        = 00400000
    //  end_addr          = 00437b88
    //  size              =    37b88
    //  size_pwr_2        =    40000 
    //  subregion_size    =     8000
    //  num_subregions    =        6

    uint32_t size_pwr_2 = 1<<(32-__builtin_clz(actual_size-1));
    // never use a region size of less than 256 since that makes the
    // subregion smaller than 32 bytes
    if (size_pwr_2 < 256)
    {
        size_pwr_2 = 256;
    }
    DEBUG_LOG_STRING(size_pwr_2);

    uint32_t subregion_size = size_pwr_2 / 8;
    DEBUG_LOG_STRING(subregion_size);
    num_subregions = (actual_size) / subregion_size;
    DEBUG_LOG_STRING(num_subregions);
    actual_mpu_size = num_subregions * subregion_size;
    DEBUG_LOG_STRING(actual_mpu_size);
    next_addr   = start_addr + actual_mpu_size;

    // set last num_subregions bits in the subregions_mask byte.
    // e.g. if subregions = 6 we want subregions_mask to be 0b1100_0000
    subregions_mask = 0xff << (num_subregions);
    subregions_mask &= 0xff;
    DEBUG_LOG_STRING(subregions_mask);
}


/*
example of running Configure_ReadOnlyMemory_for_text_and_rodata() with logging:

e.g. if you need to configure some memory of size <256K then you need 4 pages to get up to 64 bytes resolution.

first page covers  256K +/-  32k
second page covers  32k +/-   4k
third page covers    4k +/- 512b
fourth page covers 512b +/-  64b

start_addr       0x400000
end_addr         0x437c85
actual_size      0x37c85
size_pwr_2       0x40000
subregion_size   0x8000
num_subregions   0x6
actual_mpu_size  0x30000
subregions_mask  0xfc
next_addr        0x430000   <-- first page covers from 0x400000 to 0x430000

actual_size      0x7c85
size_pwr_2       0x8000
subregion_size   0x1000
num_subregions   0x7
actual_mpu_size  0x7000
subregions_mask  0xfe
next_addr        0x437000   <-- second page covers from 0x430000 to 0x437000

actual_size      0xc85
size_pwr_2       0x1000
subregion_size   0x200
num_subregions   0x6
actual_mpu_size  0xc00
subregions_mask  0xfc
next_addr        0x437c00   <-- third page covers from 0x437000 to 0x437c00

actual_size      0x85
size_pwr_2       0x100
subregion_size   0x20
num_subregions   0x4
actual_mpu_size  0x80
subregions_mask  0xf0
next_addr        0x437c80   <-- fourth page covers from 0x437c00 to 0x437c80

to reduce the number of entries required to get a perfect fit, the linker file device.ld
could be changed to increase the alignment in the .bss section.

note: .data is "initialized" data and starts with the symbol __data_start__, __data_load or _sdata

    / * initialized data section * /
    .data :
    {
        . = ALIGN(64);
        __data_load = LOADADDR(.data);
        _sdata = .;
        __data_start__ = .;

to change .data to start at a 64 byte boundary or 128

*/



/*
 * use one more more regions to configure memory
 *
 * algorithm where start_addr is not naturally aligned.

  byron's thoughts:

  figure out the alignment of the start address and therefore the largest subregion size usable.

  e.g. if first address is 0x0041180, then the subregion size needs to be 128 bytes
  that means the region size is 8 * 128 = 1024 bytes e.g. 0x400
  that means the start address needs to be 0x0041000
  figure out the best subregion mask e.g. in this case skip first 3 regions and keep the rest.
  e.g. probably 
     0x0041000..0x004117f skip
     0x0041180..0x00413ff select

  (although have to be careful not go past the last address)

  repeat with a smaller subregion size to see if that gives more coverage.
  e.g. try 64b
  that means the region size is 8 * 64 = 512 bytes e.g. 0x200
  that means the start address needs to be 0x0041000
  figure out the best subregion mask e.g. in this case skip first 6 regions and keep the rest. 
     0x0041000..0x004117f skip
     0x0041180..0x00411ff select
  (although have to be careful not go past the last address)

  repeat with a smaller subregion size to see if that gives more coverage.
  e.g. try 32b
  that means the region size is 8 * 32 = 256 bytes e.g. 0x100
  that means the start address needs to be 0x0041100
  figure out the best subregion mask e.g. in this case skip first 4 regions and keep the rest. 
     0x0041100..0x004117f skip
     0x0041180..0x00411ff select
  (although have to be careful not go past the last address)

  i don't think going below 256 is useful since subregions aren't available.

  so in the above example, we'd pick the first one unless our last address was 0x00411bf

maybe for simplicity we can just select the the first usable subregion size.
 */
void mpu_calculator_t::try_subregion_size(uint32_t subregion_size, uint32_t region_start_addr_ )
{
    region_start_addr = region_start_addr_;
    region_size = subregion_size * 8;
    DEBUG_LOG_STRING(region_start_addr);
    // e.g. if start addr = 0x0041180
    // and subregion_size = 0x80
    // region_size = 0x400
    // so region_start_addr = 0x0041000
    uint32_t first_subregion;
    if (start_addr > region_start_addr)
    {
        // e.g.
        // if start_addr         = 0x00412345
        // and region_start_addr = 0x00410000
        // and subregion_size    = 0x00001000
        // then first subregion would be at:
        //                         0x00413000
        // e.g. (0x00412345 - 0x00410000 + 0xffff) / subregion_size;
        first_subregion = (start_addr + (subregion_size - 1) - region_start_addr) / subregion_size;
        first_addr = region_start_addr + first_subregion * subregion_size;
    }
    else
    {
        first_subregion = 0;
        first_addr = region_start_addr;
    }
    // e.g. if      end_addr = 0x00437c85
    // and start_region_addr = 0x00400000
    // and subregion_size is = 0x00020000
    // last_subregion should be: 0   0x00437c85 - 0x00400000 = 0x17c85 - 
    // 
    uint32_t last_subregion = (end_addr+1 - (subregion_size-1) - region_start_addr) / subregion_size;
    if (last_subregion > 7)
    {
        last_subregion = 7;
    }
    if (first_subregion > 7 || last_subregion < first_subregion)
    {
        num_subregions = 0;
    }
    else
    {
        num_subregions = last_subregion - first_subregion + 1;
    }
    // if the first subregion finishes after the start address,
    // then nothing works.
    if (region_start_addr + subregion_size-1 > end_addr)
    {
        num_subregions = 0;
    }
    if (num_subregions == 0)
    {
        subregions_mask = 0xff;
    }
    else
    {
        // e.g. if num_subregions = 4, and first_subregion = 1
        // valid_regions   = 0b0011110
        // subregions_mask = 0b1100001
        uint32_t valid_regions = ((1<<num_subregions) - 1) << first_subregion;
        subregions_mask = 0xff & ~valid_regions;
    }
    DEBUG_LOG_STRING(subregions_mask);
    actual_mpu_size = num_subregions * subregion_size;
    //MDX2_ASSERT(actual_mpu_size <= region_size);
    //printf("if using a subregion size of 0x%x and a region_start_addr of 0x%08x, %d subregions (%x) would overlap with 0x%08x .. 0x%08x covering 0x%08x\n",subregion_size,region_start_addr,num_subregions,subregions_mask,start_addr,end_addr,actual_mpu_size);
    // DEBUG_LOG_STRING(actual_mpu_size);
}

// select best mpu region to cover as much of start_addr .. end_addr as possible.
// where neither start nor end are nicely aligned.
void mpu_calculator_t::select_best_mpu_size()
{
    if (start_addr == 0 && end_addr == 0xffffffff)
    {
        try_subregion_size(0x20000000,0x0);
        next_addr = end_addr;
        return;
    }
    uint32_t actual_size = end_addr - start_addr + 1;

    // max subregionsize to test: actual_size;
    // min subregionsize to test: stop if all 8 subregions used. e.g. subregion_mask == 0
    // max subregionsize is also natural alignment of start_addr
    // min subregionsize = 32
    // e.g. if start addr = 0x0041180,... the 0x80 is max subregionsize.
    // or stop if current_coverage > subregionsize * 8
    DEBUG_LOG_STRING(start_addr);
    DEBUG_LOG_STRING(end_addr);
    DEBUG_LOG_STRING(actual_size);
    // e.g. 
    //  start_addr        = 00400000
    //  end_addr          = 00437b88
    //  size              =    37b88
    //  size_pwr_2        =    40000 
    //  subregion_size    =     8000
    //  num_subregions    =        6

    uint32_t size_pwr_2 = 1<<(32-__builtin_clz(actual_size-1));
    // never use a region size of less than 256 since that makes the sub region size 32
    if (size_pwr_2 < 256)
    {
        size_pwr_2 = 256;
    }
    DEBUG_LOG_STRING(size_pwr_2);

    // maybe better with size_pwr_2 / 8,... not sure
    uint32_t max_subregion_size = size_pwr_2 / 2;

    // this code breaks trying to use a region size of 2^32
    if (actual_size > 0x80000000)
    {
        max_subregion_size = 0x80000000;
    }
    uint32_t min_subregion_size = 32;
    uint32_t best_mpu_size = 0;
    uint32_t best_mpu_subregion_size = 0;
    uint32_t best_mpu_region_start_addr = 0;
    DEBUG_LOG_STRING(max_subregion_size);
    DEBUG_LOG_STRING(min_subregion_size);
    for (uint32_t subregion_size = max_subregion_size;
         subregion_size >= min_subregion_size;
         subregion_size /= 2)
    {
        uint32_t try_region_size = subregion_size*8;
        // try with region starting before start_addr
        region_start_addr = start_addr & ~(try_region_size-1);
        try_subregion_size(subregion_size,region_start_addr);
        if (actual_mpu_size >= best_mpu_size)
        {
            best_mpu_size = actual_mpu_size;
            best_mpu_subregion_size = subregion_size;
            best_mpu_region_start_addr = region_start_addr;
            //printf("subregion_size = 0x%08x (%d), region_start_addr=0x%08x, actual_mpu_size = 0x%08x\n",subregion_size,subregion_size,region_start_addr,actual_mpu_size);
        }
        if (subregions_mask == 0)
        {
            //printf("all mpu regions used,... found best\n");
            break;
        }
        if (region_start_addr != start_addr)
        {
            // try with region starting after start_addr
            region_start_addr = (start_addr + try_region_size-1) & ~(try_region_size-1);
            try_subregion_size(subregion_size,region_start_addr);
            if (actual_mpu_size >= best_mpu_size)
            {
                best_mpu_size = actual_mpu_size;
                best_mpu_subregion_size = subregion_size;
                best_mpu_region_start_addr = region_start_addr;
                //printf("subregion_size = 0x%08x (%d), region_start_addr=0x%08x, actual_mpu_size = 0x%08x\n",subregion_size,subregion_size,region_start_addr,actual_mpu_size);
            }
            if (subregions_mask == 0)
            {
                //printf("all mpu regions used,... found best\n");
                break;
            }
        }
    }

    try_subregion_size(best_mpu_subregion_size,best_mpu_region_start_addr);
    next_addr = first_addr + actual_mpu_size;
    DEBUG_PRINTF("BYRON: was looking for a region to cover: 0x%08x .. 0x%08x\n",start_addr,end_addr);
    DEBUG_PRINTF("BYRON: found region:                      0x%08x .. 0x%08x\n",first_addr,next_addr);
    DEBUG_PRINTF("BYRON: full size was: 0x%08x .. 0x%08x\n",region_start_addr,region_start_addr+best_mpu_subregion_size*8);
    DEBUG_PRINTF("BYRON: with subregion mask  0x%02x\n",subregions_mask);
    DEBUG_PRINTF("BYRON: e.g. covers 0x%08x of 0x%08x\n",actual_mpu_size,actual_size);
#if 0
            uint32_t subregions = (~e->SubRegionDisable) & 0xff;
            while (subregions != 0)
            {
                uint32_t first_subregion = __builtin_ctz(subregions);
                uint32_t next_subregion = first_subregion;
                while (subregions & (1<<(next_subregion)))
                {
                    subregions &= ~(1<<next_subregion);
                    next_subregion++;
                }
                //uint32_t last_subregion = next_subregion-1;
                uint32_t start_addr = e->BaseAddress + e->subregion_size*first_subregion;
                uint32_t end_addr = e->BaseAddress + e->subregion_size*next_subregion-1;
                if (start_addr < first_addr)
                {
                    first_addr = start_addr;
                }
                if (end_addr > last_addr)
                {
                    last_addr = end_addr;
                }
            }
#endif
    DEBUG_LOG_STRING(best_mpu_subregion_size);
    DEBUG_LOG_STRING(region_start_addr);
    DEBUG_LOG_STRING(subregions_mask);
    DEBUG_LOG_STRING(actual_mpu_size);
    DEBUG_LOG_STRING(next_addr);
}

bool mpu_calculator_t::copy_child_entries( mpu_calculator_t &child )
{
    for (uint32_t i=0;i<child.num_entries;i++)
    {
        if (num_entries >= MAX_ENTRIES)
        {
            return false;
        }
        uint32_t RBAR = child.mpu_table[i].RBAR;
        uint32_t RASR = child.mpu_table[i].RASR;
        RBAR = RBAR & ~MPU_RBAR_REGION_Msk;
        RBAR = RBAR | mpu_region_number;
        mpu_table[num_entries].RBAR = RBAR;
        mpu_table[num_entries].RASR = RASR;
        num_entries++;
        mpu_region_number++;
    }
    return true;
}

/**
 * @brief
 *    build a set of MPU entries to cover the region
 * 
 * this finds the largest mpu entry to cover the majority of the region, 
 * then fills in the left-overs on the left and right.
 * hopefully either start_addr_ or end_addr_ is nicely aligned, otherwise a boatload of regions would be required.
 * e.g. to/from arbitrary 32 byte locations would take 10 entries,... 
 * and of course we only have a maximum of 16 for everything !
 * 
 * @note: end_addr IS NOW part of the region,... e.g. the region is start_addr .. end_addr, and end_addr+1 is outside the region.
 */
bool mpu_calculator_t::build_best_mpu_entries( uint32_t start_addr_, uint32_t end_addr_, uint32_t DisableExec, uint32_t AccessPermission, uint32_t AccessAttributes)
{
    if (num_entries >= MAX_ENTRIES)
    {
        return false;
    }
    if (mpu_region_number >= MAX_ENTRIES)
    {
        return false;
    }
    start_addr = start_addr_;
    end_addr = end_addr_;
    if (end_addr - start_addr < 31)
    {
        return false;
    }

    select_best_mpu_size();
    build_entry(DisableExec,AccessPermission,AccessAttributes);
    // save first_addr and next_addr
    // since we recursively call build_best_mpu_entries and overwrite those class variables
    uint32_t left_addr = first_addr;
    uint32_t right_addr = next_addr;
    // add coverage to the left.
    if (left_addr > start_addr_)
    {
        mpu_calculator_t left_calculator;
        left_calculator.build_best_mpu_entries(start_addr_,left_addr-1,DisableExec,AccessPermission,AccessAttributes);
        if (!copy_child_entries(left_calculator))
        {
            return false;
        }
    }
    // add coverage to the right. (next_addr == 0 means overflow)
    // as Mame Maria M'baye points out,
    // (right_addr < end_addr_) skips the final byte
    // e.g. if right_addr == 0x200 and end_addr = 0x200
    // then we don't build an entry for 0x200..0x200
    // which has the nice accidental feature that 0x200 behaves like 0x1ff
    // which is probably what was intended.
    // and we are relying on this for our coverage of .text and .logging
    // because currently the 'end' of one region is currently actually the first byte
    // of the next region,... however we could update the scripts to subtract one,... 
    // but subtracting one from a hexadecimal number in a shell script is a little
    // fragile too !
    if ((right_addr < end_addr_) && (next_addr != 0))
    {
        mpu_calculator_t right_calculator;
        right_calculator.build_best_mpu_entries(right_addr,end_addr_,DisableExec,AccessPermission,AccessAttributes);
        if (!copy_child_entries(right_calculator))
        {
            return false;
        }
    }
    return true;
}




/**
 *
 * @param[in] DisableExec - EXECUTABLE or NEVER_EXECUTE
 * @param[in] AccessPermission - ARM_MPU_AP_RO or ARM_MPU_AP_FULL
 * @param[in] AccessAttributes - NORMAL_WRITE_BACK_READ_AND_WRITE_ALLOCATE or NORMAL_UNCACHED
 */
void mpu_calculator_t::build_entry( uint32_t DisableExec, uint32_t AccessPermission, uint32_t AccessAttributes)
{
    uint32_t mpu_RBAR = ARM_MPU_RBAR(mpu_region_number, region_start_addr);

    uint32_t region_bits = region_size_to_region_bits(region_size);

    uint32_t mpu_RASR = ARM_MPU_RASR_EX(DisableExec, 
                                        AccessPermission,
                                        AccessAttributes,
                                        subregions_mask,
                                        region_bits);


    mpu_table[num_entries].RBAR = mpu_RBAR;
    mpu_table[num_entries].RASR = mpu_RASR;
    num_entries++;
    mpu_region_number++;
}

