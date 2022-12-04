/* copyright Microchip 2022, MIT License */
/**
* @file
* @brief
*   This file configures the m7 MPU
*/

// Include Files

// Local Enumerated Types

// Local Constants

// Namespace

// Local Macro Definitions

// Local Classes

// Local Structures and Unions

// Local Variables

// Local Function Prototypes

// Extern Variables

// Private Functions

// Module Functions

// Public Functions

#include "cpu_m7.h"
#include "mpu_armv7.h"
#include "top.h"
#include "mdx2_freertos.h"
#include "configure_mpu.h"
#include "dbg_log.h"
#include "device_ocr_addr.h"
#include "mdx2_shared_memory.h"
#include "mpu_calculator.h"
#include "mpu_display.h"
#include "multitask.h" // for STACK_OVERFLOW_MPU_GUARD_SIZE_IN_BYTES


#include "mpu_table.h"

extern uint32_t _end_of_rodata;
extern uint32_t __data_start__;

extern uint32_t __logging_start__;
extern uint32_t __logging_end__;

#if 0
// byron:
//    https://developer.arm.com/documentation/dui0646/c/cortex-m7-peripherals/optional-memory-protection-unit/mpu-access-permission-attributes?lang=en
//
//    https://community.nxp.com/t5/LPC-Microcontrollers/Memory-protection-unit-MPU-example-for-NXP-LPC4078/m-p/1001075
//
// also some nice tables here,... but doesn't show TEX 101:
//   https://www.st.com/resource/en/application_note/dm00272912-managing-memory-protection-unit-in-stm32-mcus-stmicroelectronics.pdf
//
#endif

void Configure_MPU()
{
    // set SBC->CACR.SIWT=1 because Jason Thomson says so.
    // or if one was to read the documentation it would make sense.
    // see also a variety of performance issues with various cortex-m ports
    // pointing to "woops,... needs to set SIWT=1"
    SCB->CACR = SCB->CACR | SCB_CACR_SIWT_Msk;

    __DMB();

    ARM_MPU_Disable();
    for (uint32_t i=0;i<16;i++)
    {
        ARM_MPU_ClrRegion(i);
    }

    const ARM_MPU_Region_t *mpu_table = mpuTable;

    ARM_MPU_Load(mpu_table, MPU_TABLE_SIZE);

    __DSB();
    __ISB();

    //  --> MPU_CTRL_HFNMIENA_Msk=1   Enable MPU during hard fault, NMI, and FAULTMASK handlers execution    
    //      MPU_CTRL_HFNMIENA_Msk=0   Disable MPU during hard fault, NMI, and FAULTMASK handler execution
    //      MPU_CTRL_PRIVDEFENA_Msk=1 Enable default memory map as a background region for privileged access    
    //  --> MPU_CTRL_PRIVDEFENA_Msk=0 Use only MPU region settings

    // note: DX2SW-485 tried calling ARM_MPU_Enable(0) e.g. MPU_CTRL_HFNMIENA_Msk=0 but this didn't help.
    ARM_MPU_Enable(MPU_CTRL_HFNMIENA_Msk);

    __DSB();
    __ISB();

}



extern "C" void mpu_dump()
{
    LOG_STRING("read_only_start",0x00400000);
    LOG_STRING("read_only_end",(uint32_t)&__data_start__);
    LOG_STRING("write_through_START",(uint32_t)&__logging_start__);
    LOG_STRING("write_through_END",(uint32_t)&__logging_end__);
    uint32_t mpu_TYPE = MPU->TYPE;
    uint32_t d_regions = (mpu_TYPE & 0xff00) >> 8;
    //uint32_t i_regions = (mpu_TYPE & 0xff);
    //LOG_STRING( "mpu_TYPE", mpu_TYPE );
    //LOG_STRING( "d_regions", d_regions );
    //LOG_STRING( "i_regions", i_regions );
    //uint32_t mpu_CTRL = MPU->CTRL;
    //uint32_t mpu_Enable = mpu_CTRL & 0x4;
    //uint32_t mpu_HFNMIENA = mpu_CTRL & 0x2;
    //uint32_t mpu_PRIVDEFENA = mpu_CTRL & 0x1;
    //LOG_STRING( "mpu_CTRL", mpu_CTRL );
    //LOG_STRING( "mpu_Enable", mpu_Enable );
    //LOG_STRING( "mpu_HFNMIENA", mpu_HFNMIENA );
    //LOG_STRING( "mpu_PRIVDEFENA", mpu_PRIVDEFENA );
    // TODO: add log codes for mpu_ctrl and mpu_type
    mpu_display_t mpu_display;
    for (uint32_t i = 0; i<d_regions;i++)
    {
        MPU->RNR = i;
        uint32_t mpu_RBAR = MPU->RBAR;
        uint32_t mpu_RASR = MPU->RASR;
        mpu_display.set(i,mpu_RBAR,mpu_RASR);
        MDX2_LOG4_INFO( MDX2_DIGIHAL_MPU_REGION_RBAR_RASR, i, mpu_RBAR, mpu_RASR, (uint32_t)(size_t)"dump" );
    }
    mpu_display.display_memory_map(NULL,"");
}


#if( configUSE_MPU_THREAD_STACK_GUARD == 1 )

    // for debugging generate a debug log when calling MPUThreadGuard_apply inside vTaskSwitchContext
    void MPUThreadGuard_log( MPUThreadStackGuard_t *xThreadGuard )
    {
        MDX2_LOG4_INFO( MDX2_DIGIHAL_MPU_REGION_RBAR_RASR, xThreadGuard->MPU_thread_guard[0].RBAR & MPU_RBAR_REGION_Msk, xThreadGuard->MPU_thread_guard[0].RBAR, xThreadGuard->MPU_thread_guard[0].RASR, (uint32_t)(size_t)xThreadGuard->task_name );
    }

    // calculate the mpu region that covers a region at the end of the thread's task
    void MPUThreadGuard_calculate( MPUThreadStackGuard_t *xThreadGuard, StackType_t *pxStack, const char *task_name )
    {
        xThreadGuard->task_name = task_name;
        mpu_calculator_t mpu_calc;
        mpu_calc.mpu_region_number = 15;
        mpu_calc.start_addr = (uint32_t)pxStack;
        mpu_calc.end_addr = mpu_calc.start_addr+STACK_OVERFLOW_MPU_GUARD_SIZE_IN_BYTES-1;

// 
// Table B3-15 Access permissions field encoding
// AP[2:0] Privileged access Unprivileged access Notes
// 000     No access         No access           Any access generates a permission fault
// 001     Read/write        No access           Privileged access only
// 010     Read/write        Read-only           Any unprivileged write generates a permission fault
// 011     Read/write        Read/write          Full access
// 100     UNPREDICTABLE     UNPREDICTABLE       Reserved
// 101     Read-only         No                  access Privileged read-only
// 110     Read-only         Read-only           Privileged and unprivileged read-only
// 111     Read-only         Read-only           Privileged and unprivileged read-only

// DX2SW-485: tried using a 'red zone' with with ARM_MPU_AP_PRO or ARM_MPU_AP_PRIV 
//  but those do not cause a memory protection violation. (are threads privileged?)
#define ARM_MPU_AP_NONE 0U ///!< MPU Access Permission no access
#define ARM_MPU_AP_PRIV 1U ///!< MPU Access Permission privileged access only
#define ARM_MPU_AP_URO  2U ///!< MPU Access Permission unprivileged access read-only
#define ARM_MPU_AP_FULL 3U ///!< MPU Access Permission full access
#define ARM_MPU_AP_PRO  5U ///!< MPU Access Permission privileged access read-only
#define ARM_MPU_AP_RO   6U ///!< MPU Access Permission read-only access

        mpu_calc.build_best_mpu_entries(mpu_calc.start_addr,mpu_calc.end_addr,NEVER_EXECUTE,ARM_MPU_AP_RO,NORMAL_WRITE_BACK_READ_AND_WRITE_ALLOCATE);
        // mpu_display_t mpu_display;
        // for (uint32_t i = 0; i<15;i++)
        // {
        //     MPU->RNR = i;
        //     uint32_t mpu_RBAR = MPU->RBAR;
        //     uint32_t mpu_RASR = MPU->RASR;
        //     mpu_display.set(i,mpu_RBAR,mpu_RASR);
        // }
        //     uint32_t mpu_RBAR = mpu_calc.mpu_table[0].RBAR;
        //     uint32_t mpu_RASR = mpu_calc.mpu_table[0].RASR;
        // mpu_display.set(15,mpu_RBAR,mpu_RASR);
        // MDX2_LOG3_CRITICAL( MDX2_DIGIHAL_MPU_REGION_RBAR_RASR, 15, mpu_RBAR, mpu_RASR );
        // mpu_display.display_memory_map(NULL,"");
        xThreadGuard->MPU_thread_guard[0] = mpu_calc.mpu_table[0];
        SCB_CleanDCache_by_Addr(pxStack,STACK_OVERFLOW_MPU_GUARD_SIZE_IN_BYTES);
    }
#endif

#define MPU_ENTRY_15_FOR_TESTING_PMON_CACHED_READS 15


/**
 * when googling for what's the M7 L1 cache prefetch instruction (it's PLD) found a nice read about the M7 cache:
 *
 * https://blog.feabhas.com/2020/11/introduction-to-the-arm-cortex-m7-cache-part-3-optimising-software-to-use-cache/
 *
 * use MPU entry 15 to configure base_addres + size as cacheable.
 *
 * programs mpu entry 15 to be cached for addresses in [base_address,base_address+size)
 *
 * e.g.
 *
 * @param[in] base_address - base_address
 * @param[in] size_in_bytes - size in bytes
 * @param[in] DisableExec - NEVER_EXECUTE
 * @param[in] AccessPermission - ARM_MPU_AP_RO
 * @param[in] AccessAttributes - NORMAL_WRITE_BACK_READ_AND_WRITE_ALLOCATE or NORMAL_UNCACHED
 */
void mpu_configure_region( uint32_t base_address, uint32_t size_in_bytes, uint32_t DisableExec, uint32_t AccessPermission, uint32_t AccessAttributes )
{
    mpu_calculator_t mpu_calc;
    mpu_calc.mpu_region_number = MPU_ENTRY_15_FOR_TESTING_PMON_CACHED_READS;
    mpu_calc.start_addr = base_address;
    mpu_calc.end_addr = base_address+size_in_bytes;

    #define MAX_ENTRIES 1
    LOG_STRING("start_addr",mpu_calc.start_addr);
    LOG_STRING("end_addr",mpu_calc.end_addr);
    do {
        mpu_calc.find_size_and_subregions();
        mpu_calc.build_entry(DisableExec,AccessPermission,AccessAttributes);
        mpu_calc.start_addr = mpu_calc.next_addr;
        LOG_STRING("next_addr",mpu_calc.next_addr);
    } while (mpu_calc.num_entries < MAX_ENTRIES && (mpu_calc.end_addr - mpu_calc.start_addr >= 32));


    // need to ensure no memory operations are in flight when we disable an MPU region and reprogram it.
    // so we disable interrupts and wait for all memory operations to complete.
    {
        cpu_interrupt_disable_guard disable_interrupts;

        __DSB();
        __ISB();
        __DMB();

        // assuming just 1 entry:
        ARM_MPU_ClrRegion(MPU_ENTRY_15_FOR_TESTING_PMON_CACHED_READS);
        ARM_MPU_SetRegion(mpu_calc.mpu_table[0].RBAR,mpu_calc.mpu_table[0].RASR);
    }

    MDX2_LOG4_ERROR( MDX2_DIGIHAL_MPU_REGION_RBAR_RASR, mpu_calc.mpu_region_number, mpu_calc.mpu_table[0].RBAR, mpu_calc.mpu_table[0].RASR, (uint32_t)(size_t)"config" );
    // note: as an optimization we could create a const_expr expression for the MPU entry that corresponds to a particular base_address and size
    // which would allow us to know the RBAR and RASR for a fixed range of memory.
}

/**
 * clear mpu table entry 15 (undo mpu_configure_region_as_cached)
 */
void mpu_clear_region()
{
    cpu_interrupt_disable_guard disable_interrupts;

    __DSB();
    __ISB();
    __DMB();

    // assuming just 1 entry:
    ARM_MPU_ClrRegion(MPU_ENTRY_15_FOR_TESTING_PMON_CACHED_READS);
}

