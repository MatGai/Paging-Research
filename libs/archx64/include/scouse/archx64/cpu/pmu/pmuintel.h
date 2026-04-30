#ifndef MSR_INTEL_H
#define MSR_INTEL_H

//
// Information taken from Intel Volume 3 Software Development Manual Chapter 21
//

#include <scouse/shared/cpuinfo.h>
#include <scouse/shared/status.h>
#include <scouse/archx64/intrinsics.h>

#define PERFORMANCE_MONITOR_EVENT_COUNT 8

EXTERN PCSTR CONST IntelPerformanceMonitorEvents[];

typedef enum _INTEL_PERFORMANCE_MONITOR_EVENT
{
    CORE_CYCLES                  = 0x0,
    INSTRUCTION_RETIRED          = 0x1,
    REFERNECE_CYCLES             = 0x2,
    LAST_LEVEL_CACHE_REFERENCE   = 0x3,
    LAST_LEVEL_CACHE_MISSES      = 0x4,
    BRANCH_INSTRUCTION_RETIRED   = 0x5,
    BRANCH_MISPREDICT_RETIRED    = 0x6,
    TOP_DOWN_SLOT                = 0x7,
    EVENT_UNSUPPORTED            = 0x8
} INTEL_PERFORMANCE_MONITOR_EVENT;

// Each version also supports the previous versions. 
typedef enum _INTEL_PMU_VERSION
{
    NO_SUPPORT, // CPUID.0Ah:EAX[7:0] = 0
    VERSION_1,  // CPUID.0Ah:EAX[7:0] = 1
    VERSION_2,  // CPUID.0Ah:EAX[7:0] = 2
    VERSION_3,  // CPUID.0Ah:EAX[7:0] = 3
    VERSION_4,  // CPUID.0Ah:EAX[7:0] = 4 -> Skylake and later
    VERSION_5,  // CPUID.0Ah:EAX[7:0] = 5
    VERSION_6   // CPUID.0Ah:EAX[7:0] = 6
} INTEL_PMU_VERSION;

typedef struct _INTEL_MSR_PER_CORE_INFO
{
    INTEL_PMU_VERSION Version;
    ULONG32           MsrCount;
    ULONG32           PmcBitWidth;
    ULONG32           FixedFunctionCount;
    ULONG32           FixedFunctionBitWidth;

    INTEL_PERFORMANCE_MONITOR_EVENT SupportedEvents[PERFORMANCE_MONITOR_EVENT_COUNT];

} INTEL_MSR_PER_CORE_INFO, *PINTEL_MSR_PER_CORE_INFO;

//
// This is more aimed towards the newer skylake generation of intel processors.
// Using earlier generations may cause undefined behaviour.
//

#define INTEL_CPUID_PMU_LEAF         0xA
#define INTEL_CPUID_PMU_VERSION_MASK 0x7F

#define INTEL_CPUID_PERFANCE_MONITOR_COUNT( Eax )        ( ( Eax >> 8 ) & 0x7F ) // [15:8]
#define INTEL_CPUID_PERFORMANCE_MONITOR_BIT_WIDTH( Eax ) ( ( Eax >> 16 ) & 0xFF )// [23:16]
#define INTEL_CPUID_FIXED_FUNCTION_COUNT( Edx )          ( Edx & 0x1F )          // [4:0]
#define INTEL_CPUID_FIXED_FUNCTION_BIT_WIDTH( Edx )      ( ( Edx >> 5 ) & 0xFF ) // [12:5]

#define INTEL_CPUID_PERFORMANCE_MONITOR_EVENT_SUPPORT( Event, Ebx, Eax ) ( !((Ebx >> Event) & 0x1) && ( ( ( Eax >> 24 ) & 0x7F ) > Event ) )
#define INTEL_CPUID_FIXCOUNTER_SUPPORT( Counter, Ecx, Edx )              ( ( ( Ecx >> Counter ) & 0x1 ) || ( ( Edx & 0x1F ) > Counter ) ) 

#define INTEL_MSR_IA32_PMC(Index)             (0x0C1u + (ULONG32)(Index))
#define INTEL_MSR_IA32_PERFEVTSEL(Index)      (0x186u + (ULONG32)(Index)) 

#define INTEL_MSR_IA32_FIXED_CTR0           0x309u 
#define INTEL_MSR_IA32_FIXED_CTR1           0x30Au 
#define INTEL_MSR_IA32_FIXED_CTR2           0x30Bu 

#define INTEL_MSR_IA32_FIXED_CTR_CTRL       0x38Du
#define INTEL_MSR_IA32_PERF_GLOBAL_STATUS   0x38Eu
#define INTEL_MSR_IA32_PERF_GLOBAL_CTRL     0x38Fu
#define INTEL_MSR_IA32_PERF_GLOBAL_OVF_CTRL 0x390u

#define INTEL_MSR_PERF_CTR( Ctr, Index ) ( ( Ctr ) & ( ~( 0xFull << ( Index * 4 ) ) ) )

//#define INTEL_EVTSEL_USR   (1ull << 16) // specifies counter is operating at ring level > 0
//#define INTEL_EVTSEL_OS    (1ull << 17) // specifies counter is operating at ring level 0
//#define INTEL_EVTSEL_EN    (1ull << 22) // used to enable counter

#define PMU_INTEL_OSUSER_NONE 0x0
#define PMU_INTEL_OSUSER_USER 0x1
#define PMU_INTEL_OSUSER_OS   0x2
#define PMU_INTEL_OSUSER_ALL  0x3

extern INTEL_MSR_PER_CORE_INFO IntelPmuInfo;

/**
* @brief Checks if the processor supports the PMU and returns the version number if it does.
* 
* @param PmuInfo A pointer to variable that will receive the PMU information.
* 
* @return SC_OK if the PMU is supported, with version number returned in PmuVersionNumber. SC_PMU_UNSUPPORTED if the PMU is unsupported.  
*/
SCSTATUS
IntelPmuSupport(
    _Out_ PINTEL_MSR_PER_CORE_INFO PmuInfo
);

#define SKL_EVT_DTLB_LOAD_MISSES   0x08u
#define SKL_EVT_DTLB_STORE_MISSES  0x49u
#define SKL_EVT_ITLB_MISSES        0x85u
#define SKL_UMASK_WALK_COMPLETED   0x0Eu

FORCEINLINE
VOID
IntelDisableAllCounters(
    VOID
)
{
    __writemsr(INTEL_MSR_IA32_PERF_GLOBAL_CTRL, 0);

    // Disable all GP event selects and clear counters
    for (ULONG32 Index = 0; Index < IntelPmuInfo.MsrCount; ++Index)
    {
        __writemsr(INTEL_MSR_IA32_PERFEVTSEL(Index), 0);
        __writemsr(INTEL_MSR_IA32_PMC(Index), 0);
    }

    // Disable fixed counters and clear values
    __writemsr(INTEL_MSR_IA32_FIXED_CTR_CTRL, 0);
    __writemsr(INTEL_MSR_IA32_FIXED_CTR0, 0);
    __writemsr(INTEL_MSR_IA32_FIXED_CTR1, 0);
    __writemsr(INTEL_MSR_IA32_FIXED_CTR2, 0);

    // Clear overflow flags
    __writemsr(INTEL_MSR_IA32_PERF_GLOBAL_OVF_CTRL, ~0ull);
}

FORCEINLINE
VOID
PmuIntelDisableFixedCounter(
    ULONG32 CounterIndex
)
{
    if (CounterIndex >= IntelPmuInfo.FixedFunctionCount)
    {
        return;
    }
    __writemsr(INTEL_MSR_IA32_PERF_GLOBAL_CTRL, (__readmsr(INTEL_MSR_IA32_PERF_GLOBAL_CTRL)) & ~(1ull << (32 + CounterIndex)));
    __writemsr(INTEL_MSR_IA32_FIXED_CTR_CTRL, INTEL_MSR_PERF_CTR(__readmsr(INTEL_MSR_IA32_FIXED_CTR_CTRL), CounterIndex));
}

FORCEINLINE
VOID
PmuIntelDisableGpCounter(
    ULONG32 CounterIndex
)
{
    if (CounterIndex >= IntelPmuInfo.MsrCount)
    {
        return;
    }
    __writemsr(INTEL_MSR_IA32_PERF_GLOBAL_CTRL, (__readmsr(INTEL_MSR_IA32_PERF_GLOBAL_CTRL)) & ~(1ull << CounterIndex));
    __writemsr(INTEL_MSR_IA32_PERFEVTSEL(CounterIndex), 0);
}

FORCEINLINE
VOID
PmuIntelConfigureFixedCounter(
    ULONG32 CounterIndex,
    UINT8   OsUserMode
)
{
    if (CounterIndex >= IntelPmuInfo.FixedFunctionCount)
    {
        return;
    }

    __writemsr(INTEL_MSR_IA32_FIXED_CTR0 + CounterIndex, 0);

    ULONG64 Ctr = INTEL_MSR_PERF_CTR(__readmsr(INTEL_MSR_IA32_FIXED_CTR_CTRL), CounterIndex);
    if (OsUserMode & PMU_INTEL_OSUSER_OS)
    {
        Ctr |= 0x1ull << (CounterIndex * 4);
    }
    if (OsUserMode & PMU_INTEL_OSUSER_USER)
    {
        Ctr |= 0x2ull << (CounterIndex * 4);
    }
    __writemsr(INTEL_MSR_IA32_FIXED_CTR_CTRL, Ctr);
    __writemsr(INTEL_MSR_IA32_PERF_GLOBAL_CTRL, (__readmsr(INTEL_MSR_IA32_PERF_GLOBAL_CTRL)) | (1ull << (32 + CounterIndex)));
}

FORCEINLINE
VOID
PmuIntelConfigureGpCounter(
    ULONG32 CounterIndex,
    UINT8   EventSelector,
    UINT8   Umask,
    UINT8   OsUserMode
)
{
    __writemsr(INTEL_MSR_IA32_PERFEVTSEL(CounterIndex), 0);
    __writemsr(INTEL_MSR_IA32_PMC(CounterIndex), 0); // 0 out counter as cannot write to enabled counter

    ULONG64 Value = 0;
    Value |= (ULONG64)EventSelector;
    Value |= (ULONG64)Umask << 8;
    Value |= (ULONG64)(OsUserMode & 0x3) << 16;
    Value |= (1ull << 22); // Enable counter

    __writemsr(INTEL_MSR_IA32_PERFEVTSEL(CounterIndex), Value);
    __writemsr(INTEL_MSR_IA32_PERF_GLOBAL_CTRL, (__readmsr(INTEL_MSR_IA32_PERF_GLOBAL_CTRL)) | (1ull << CounterIndex));
}

FORCEINLINE
ULONG64
PmuIntelReadFixedCounter(
    _In_  ULONG32  CounterIndex
)
{
    if (CounterIndex < IntelPmuInfo.FixedFunctionCount)
    {
        return __readmsr(INTEL_MSR_IA32_FIXED_CTR0 + CounterIndex) & ((1ull << IntelPmuInfo.FixedFunctionBitWidth) - 1ull);
    }
    return 0;
}

FORCEINLINE
ULONG64
PmuIntelReadGpCounter(
    _In_  ULONG32  CounterIndex
)
{
    if (CounterIndex < IntelPmuInfo.MsrCount)
    {
        return __readmsr(INTEL_MSR_IA32_PMC(CounterIndex)) & ((1ull << IntelPmuInfo.PmcBitWidth) - 1ull);
    }
    return 0;
}

FORCEINLINE
VOID
PmuIntelRestartFixedCounters(
    VOID
)
{
    __writemsr(INTEL_MSR_IA32_PERF_GLOBAL_CTRL, 0);
    __writemsr(INTEL_MSR_IA32_FIXED_CTR_CTRL, (1ull << 0) | (1ull << 1) | (1ull << 4) | (1ull << 5) | (1ull << 8) | (1ull << 9));
    __writemsr(INTEL_MSR_IA32_FIXED_CTR0, 0);
    __writemsr(INTEL_MSR_IA32_FIXED_CTR1, 0);
    __writemsr(INTEL_MSR_IA32_FIXED_CTR2, 0);
    __writemsr(INTEL_MSR_IA32_PERF_GLOBAL_OVF_CTRL, ~0ull);
}

FORCEINLINE
VOID
PmuIntelStartFixedCounters(
    VOID
)
{
    ULONG64 g = __readmsr(INTEL_MSR_IA32_PERF_GLOBAL_CTRL);
    g |= (1ull << 32) | (1ull << 33) | (1ull << 34);                 // preserve existing GP enables
    __writemsr(INTEL_MSR_IA32_PERF_GLOBAL_CTRL, g);
}

FORCEINLINE
VOID
PmuIntelStopAllCounters(
    VOID
)
{
    __writemsr(INTEL_MSR_IA32_PERF_GLOBAL_CTRL, 0);
}

FORCEINLINE
VOID
PmuIntelInstructionsRetiredStartCounting(
    VOID
)
{
    PmuIntelConfigureFixedCounter(
        0,
        PMU_INTEL_OSUSER_ALL
    );
}

FORCEINLINE
VOID
PmuIntelInstructionsRetiredStopCounting(
    _Out_ PULONG64 InstructionsRetired
)
{
    ULONG64 Value = PmuIntelReadFixedCounter(0);
    PmuIntelDisableFixedCounter(0);
    if (InstructionsRetired)
    {
        *InstructionsRetired = Value;
    }
}

FORCEINLINE
VOID
PmuIntelCpuClockUnhaltedStartCounting(
    VOID
)
{
    PmuIntelConfigureFixedCounter(
        1,
        PMU_INTEL_OSUSER_ALL
    );
}

FORCEINLINE
VOID
PmuIntelCpuClockUnhaltedStopCounting(
    _Out_ PULONG64 ClockUnhalted
)
{
    ULONG64 Value = PmuIntelReadFixedCounter(1);
    PmuIntelDisableFixedCounter(1);
    if (ClockUnhalted)
    {
        *ClockUnhalted = Value;
    }
}

FORCEINLINE
VOID
PmuIntelCpuClockReferencedStartCounting(
    VOID
)
{
    PmuIntelConfigureFixedCounter(
        2,
        PMU_INTEL_OSUSER_ALL
    );
}

FORCEINLINE
VOID
PmuIntelCpuClockReferencedStopCounting(
    _Out_ PULONG64 Referenced
)
{
    ULONG64 Value = PmuIntelReadFixedCounter(2);
    PmuIntelDisableFixedCounter(2);
    if (Referenced)
    {
        *Referenced = Value;
    }
}

FORCEINLINE
VOID
PmuIntelDtlbLoadWalkCompletedStartCounting(
    VOID
)
{
    PmuIntelConfigureGpCounter(
        0,
        0x08,
        0x0E,
        PMU_INTEL_OSUSER_ALL
    );
}

FORCEINLINE
VOID
PmuIntelDtlbLoadWalkCompletedStopCounting(
    _Out_ PULONG64 Misses
)
{
    ULONG64 Value = PmuIntelReadGpCounter(0);
    PmuIntelDisableGpCounter(0);
    if (Misses)
    {
        *Misses = Value;
    }
}

//[PMU_EVT_DTLB_STORE_WALK_COMPLETED] = PMU_DB_GP(0x13, 0x0E),
//    [PMU_EVT_ITLB_WALK_COMPLETED]   = PMU_DB_GP(0x11, 0x0E),
FORCEINLINE
VOID
PmuIntelDtlbStoreWalkCompletedStartCounting(
    VOID
)
{
    PmuIntelConfigureGpCounter(
        1,
        0x49,
        0x0E,
        PMU_INTEL_OSUSER_ALL
    );
}

FORCEINLINE
VOID
PmuIntelDtlbStoreWalkCompletedStopCounting(
    _Out_ PULONG64 Misses
)
{
    ULONG64 Value = PmuIntelReadGpCounter(1);
    PmuIntelDisableGpCounter(1);
    if (Misses)
    {
        *Misses = Value;
    }
}

FORCEINLINE
VOID
PmuIntelItlbWalkCompletedStartCounting(
    VOID
)
{
    PmuIntelConfigureGpCounter(
        2,
        0x85,
        0x0E,
        PMU_INTEL_OSUSER_ALL
    );
}

FORCEINLINE
VOID
PmuIntelItlbWalkCompletedStopCounting(
    _Out_ PULONG64 Misses
)
{
    ULONG64 Value = PmuIntelReadGpCounter(2);
    PmuIntelDisableGpCounter(2);
    if (Misses)
    {
        *Misses = Value;
    }
}



#endif // !MSR_INTEL_H
