#include <stdint.h>
#include <stdbool.h>
#include <scouse/archx64/cpu/msrintel.h>
#include <intrin.h>

//
// This is more aimed towards the newer skylake generation of intel processors.
// Using earlier generations may cause undefined behaviour.
//


#define INTEL_MSR_IA32_PMC(i)             (0x0C1u + (unsigned __int32)(i))
#define INTEL_MSR_IA32_PERFEVTSEL(i)      (0x186u + (unsigned __int32)(i))

#define INTEL_MSR_IA32_FIXED_CTR0         0x309u 
#define INTEL_MSR_IA32_FIXED_CTR1         0x30Au 
#define INTEL_MSR_IA32_FIXED_CTR2         0x30Bu 

#define INTEL_MSR_IA32_FIXED_CTR_CTRL     0x38Du
#define INTEL_MSR_IA32_PERF_GLOBAL_STATUS 0x38Eu
#define INTEL_MSR_IA32_PERF_GLOBAL_CTRL   0x38Fu
#define INTEL_MSR_IA32_PERF_GLOBAL_OVF_CTRL 0x390u

#define INTEL_EVTSEL_USR   (1ull << 16)
#define INTEL_EVTSEL_OS    (1ull << 17)
#define INTEL_EVTSEL_EN    (1ull << 22)

#define INTEL_FIXED_OS   0x1u
#define INTEL_FIXED_USR  0x2u

static
__forceinline
unsigned __int64 IntelFixedCtrlField(
    unsigned __int32 FixedIndex,
    bool Os,
    bool Usr
)
{
    unsigned __int64 Result = 0;
    unsigned __int32 Field  = 0;
    if (Os)
    {
        Field |= INTEL_FIXED_OS;
    }
    if (Usr)
    {
        Field |= INTEL_FIXED_USR;
    }

    Result |= ((unsigned __int64)(Field & 0xF) << (FixedIndex * 4));
    return Result;
}

#define SKL_EVT_DTLB_LOAD_MISSES   0x08u
#define SKL_EVT_DTLB_STORE_MISSES  0x49u
#define SKL_EVT_ITLB_MISSES        0x85u
#define SKL_UMASK_WALK_COMPLETED   0x0Eu

static
void
IntelDisableAllCounters(
    void
)
{
    __writemsr(INTEL_MSR_IA32_PERF_GLOBAL_CTRL, 0);

    // Disable all GP event selects and clear counters
    for (uint32_t i = 0; i < 8; ++i) 
    {
        __writemsr(INTEL_MSR_IA32_PERFEVTSEL(i), 0);
        __writemsr(INTEL_MSR_IA32_PMC(i), 0);
    }

    // Disable fixed counters and clear values
    __writemsr(INTEL_MSR_IA32_FIXED_CTR_CTRL, 0);
    __writemsr(INTEL_MSR_IA32_FIXED_CTR0, 0);
    __writemsr(INTEL_MSR_IA32_FIXED_CTR1, 0);
    __writemsr(INTEL_MSR_IA32_FIXED_CTR2, 0);

    // Clear overflow flags
    __writemsr(INTEL_MSR_IA32_PERF_GLOBAL_OVF_CTRL, ~0ull);
}

static 
void
IntelProgramGpEvent(
    unsigned __int32 PmcIndex,
    unsigned __int8  EventSelector,
    unsigned __int8 Umask
)
{
    __writemsr(INTEL_MSR_IA32_PMC(PmcIndex), 0);

    unsigned __int64 EventSelectValue = 0;
    EventSelectValue |= (unsigned __int64)EventSelector;
    EventSelectValue |= ((unsigned __int64)Umask << 8);
    EventSelectValue |= INTEL_EVTSEL_USR | INTEL_EVTSEL_OS | INTEL_EVTSEL_EN;

    __writemsr(INTEL_MSR_IA32_PERFEVTSEL(PmcIndex), EventSelectValue);
}

bool 
IntelPmuInitSkyLake(
    void
)
{



}