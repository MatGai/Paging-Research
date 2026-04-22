#include <scouse/archx64/cpu/pmu/pmuintel.h>
#include <intrin.h>


PCSTR IntelPerformanceMonitorEvents[] =
{
    "CORE_CYCLES",
    "INSTRUCTION_RETIRED",
    "REFERNECE_CYCLES",
    "LAST_LEVEL_CACHE_REFERENCE",
    "LAST_LEVEL_CACHE_MISSES",
    "BRANCH_INSTRUCTION_RETIRED",
    "BRANCH_MISPREDICT_RETIRED",
    "TOP_DOWN_SLOT",
    "EVENT_UNSUPPORTED"
};

INTEL_MSR_PER_CORE_INFO IntelPmuInfo = { 0 };

SCSTATUS
IntelPmuSupport(
    _Out_ PINTEL_MSR_PER_CORE_INFO PmuInfo
)
{
    if( !PmuInfo )
    {
        return SC_INVALID_PARAMETER;
    }
    
    REGISTER_SET Reg0A;
    _scouse_cpuid(INTEL_CPUID_PMU_LEAF, Reg0A.Registers);

    if ((Reg0A.Eax & INTEL_CPUID_PMU_VERSION_MASK) == 0)
    {
        return SC_PMU_UNSUPPORTED;
    }

    IntelPmuInfo.Version = Reg0A.Eax & INTEL_CPUID_PMU_VERSION_MASK;
    PmuInfo->Version     = Reg0A.Eax & INTEL_CPUID_PMU_VERSION_MASK;
    if ( PmuInfo->Version < VERSION_4 )
    {
        return SC_UNSUPPORTED;
    }

    IntelPmuInfo.MsrCount              = INTEL_CPUID_PERFANCE_MONITOR_COUNT(Reg0A.Eax);
    IntelPmuInfo.PmcBitWidth           = INTEL_CPUID_PERFORMANCE_MONITOR_BIT_WIDTH(Reg0A.Eax);
    IntelPmuInfo.FixedFunctionCount    = INTEL_CPUID_FIXED_FUNCTION_COUNT(Reg0A.Edx);
    IntelPmuInfo.FixedFunctionBitWidth = INTEL_CPUID_FIXED_FUNCTION_BIT_WIDTH(Reg0A.Edx);

    PmuInfo->MsrCount              = INTEL_CPUID_PERFANCE_MONITOR_COUNT( Reg0A.Eax );
    PmuInfo->PmcBitWidth           = INTEL_CPUID_PERFORMANCE_MONITOR_BIT_WIDTH( Reg0A.Eax );
    PmuInfo->FixedFunctionCount    = INTEL_CPUID_FIXED_FUNCTION_COUNT( Reg0A.Edx );
    PmuInfo->FixedFunctionBitWidth = INTEL_CPUID_FIXED_FUNCTION_BIT_WIDTH( Reg0A.Edx );

    for (ULONG32 Index = 0; Index < PmuInfo->MsrCount; ++Index)
    {
        IntelPmuInfo.SupportedEvents[Index] = EVENT_UNSUPPORTED;
        PmuInfo->SupportedEvents[Index]     = EVENT_UNSUPPORTED;
        if( INTEL_CPUID_PERFORMANCE_MONITOR_EVENT_SUPPORT(Index, Reg0A.Ebx, Reg0A.Eax ) )
        {
            IntelPmuInfo.SupportedEvents[Index] = (INTEL_PERFORMANCE_MONITOR_EVENT)Index;
            PmuInfo->SupportedEvents[Index]     = (INTEL_PERFORMANCE_MONITOR_EVENT)Index;
        }
    }

    return SC_OK;
}



