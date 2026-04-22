#include <scouse/archx64/cpu/pmu/pmuintel.h>
#include <scouse/archx64/cpu/cpudata.h>
#include <scouse/archx64/intrinsics.h>
#include <scouse/runtime/gfx/render.h>
#include <scouse/runtime/print.h>
#include <strser.h>
#include <timer.h>

STATIC 
PCSTR 
CacheTypeStr(
    CPU_CACHE_TYPE CacheType
)
{
    switch (CacheType)
    {
        case CPU_FEATURE_CACHE_DATA:        return "Cache Data";
        case CPU_FEATURE_CACHE_INSTRUCTION: return "Cache Inst";
        case CPU_FEATURE_CACHE_UNIFIED:     return "Cache Unfied";
        case CPU_FEATURE_CACHE_TLB:         return "TLB";
        case CPU_FEATURE_CACHE_DTLB:        return "DTLB";
        case CPU_FEATURE_CACHE_STLB:        return "STLB";
        case CPU_FEATURE_CACHE_PREFETCH:    return "PREF";
        default:                            return "NULL";
    }
}

STATIC 
VOID 
DumpPageMask(
    LONG32 Mask
)
{
    // bit0=4K, bit1=2M, bit2=4M, bit3=1G
    if (Mask < 0)
    {
        return;
    }

    printf("[");
    if (Mask & 0x1) printf("4K ");
    if (Mask & 0x2) printf("2M ");
    if (Mask & 0x4) printf("4M ");
    if (Mask & 0x8) printf("1G ");
    printf("]");
}

LONG32 
KernelMain(
    PBOOT_INFO BootInfo
)
{
    SCSTATUS St;

    St = RtGfxInitContext(&BootInfo->FrameBufferDescriptor);
    if (SC_FAILED(St))
    {
        return St;
    }

    RtGfxInitConsole();

    printf(
        "Hello Kernel\nResolution: %u x %u\n",
        BootInfo->FrameBufferDescriptor.ResolutionWidth,
        BootInfo->FrameBufferDescriptor.ResolutionHeight
    );

    INTEL_MSR_PER_CORE_INFO PmuInfo = { 0 };
    St = IntelPmuSupport(&PmuInfo);
    if( SC_FAILED( St ) )
    {
        switch (St)
        {
            case SC_PMU_UNSUPPORTED:
            {
                printf("[ERROR] Pmu Unsupported %d\n", PmuInfo.Version);
                break;
            }
            case SC_INVALID_PARAMETER:
            {
                printf("[ERROR] Invalid Paramter\n");
                break;
            }
            case SC_UNSUPPORTED:
            {
                printf("[ERROR] Unsupported Pmu version %d\n", PmuInfo.Version);
                break;
            }
        }
        //return St;
    }
    else
    {
        printf(
            "Pmu Info ->\n\tVersion %d, MsrCount %d, PmcBitWidth %d, FixedCount %d, FixedWith %d\n",
            PmuInfo.Version, PmuInfo.MsrCount, PmuInfo.PmcBitWidth,
            PmuInfo.FixedFunctionCount, PmuInfo.FixedFunctionBitWidth
        );

        ULONG64 ClockUnhalted = 0;
        ULONG64 InstructionsRetired = 0;
        PmuIntelCpuClockUnhaltedStartCounting();
        PmuIntelInstructionsRetiredStartCounting();
        
        for (ULONG32 Index = 0; Index < PmuInfo.MsrCount; ++Index)
        {
            printf(
                "\tGP Counter %s",
               IntelPerformanceMonitorEvents[ Index ]
            );
        
            if (PmuInfo.SupportedEvents[Index] == EVENT_UNSUPPORTED)
            {
                printf(" (Unsupported)\n");
            }
            else
            {
                printf(" (Supported)\n");
            }
        }
        
        PmuIntelCpuClockUnhaltedStopCounting(
            &ClockUnhalted
        );

        PmuIntelInstructionsRetiredStopCounting(
            &InstructionsRetired
        );
        
        printf("Unhalted cycles: %llx , Instr Retired: %llx\n", ClockUnhalted, InstructionsRetired);
    }

    CPU_CACHE_INFO Info = { 0 };
    GetCacheInfo(&Info, 0);

    printf("CacheInfo: %d records\n", Info.Size);

    for (LONG32 Index = 0; Index < Info.Size; ++Index)
    {
        CACHE_INFO* L = &Info.Levels[Index];

        printf("#%d  L%d  %s  Size=%d  Ways=%d  Part=%d  Line=%d  Entries/Sets=%d ",
            Index,
            (LONG32)L->Level,
            CacheTypeStr(L->CacheType),
            (LONG32)L->CacheSize,
            (LONG32)L->Ways,
            (LONG32)L->Partitioning,
            (LONG32)L->LineSize,
            (LONG32)L->TlbEntries);

        if (L->CacheSize == -1 &&
            ( L->CacheType == CPU_FEATURE_CACHE_TLB  ||
              L->CacheType == CPU_FEATURE_CACHE_DTLB ||
              L->CacheType == CPU_FEATURE_CACHE_STLB
            )
           ) 
        {
            printf(" PageSizes=");
            DumpPageMask(L->LineSize);
        }

        printf("\n");
    }
    
    printf(
        "Address spaces %p Count %llu\n",
        BootInfo->MmuStresserDescriptors.AddressSpaces,
        BootInfo->MmuStresserDescriptors.AddressSpaceCount
    );

    MmuRunAllBenchmarks(BootInfo);

    return 0;
}