#include <scouse/shared/bootinfo.h>
#include <scouse/shared/cpuinfo.h>
#include <scouse/archx64/intrinsics.h>
#include <scouse/archx64/cpu/pmu/pmuintel.h>
#include <scouse/runtime/string.h>
#include <scouse/runtime/print.h>
#include <scouse/archx64/cpu/cpudata.h>
#include "include/strser.h"
#include <intrin.h>


#pragma intrinsic(_disable)
#pragma intrinsic(_enable)
#pragma intrinsic(__readcr0)
#pragma intrinsic(__writecr0)
#pragma intrinsic(__readcr3)
#pragma intrinsic(__writecr3)
#pragma intrinsic(__wbinvd)
#pragma intrinsic(__rdtsc)
#pragma intrinsic(__rdtscp)
#pragma intrinsic(__readeflags)
#pragma intrinsic(__readcr4)
#pragma intrinsic(__writecr4)

STATIC VOLATILE ULONG64 gPointerSink = 0;

#define X86_CR0_CD (1ull << 30)
#define X86_CR0_NW (1ull << 29)
#define X86_EFLAGS_IF 0x200ULL
#define X86_CR4_PGE (1ull << 7)


STATIC
FORCEINLINE
ULONG64
MmuDisableInterruptsSave(
    VOID
)
{
    ULONG64 Flags = __readeflags();
    _disable();
    return Flags;
}

STATIC
FORCEINLINE
VOID
MmuRestoreInterrupts(
    _In_ ULONG64 Flags
)
{
    if (Flags & X86_EFLAGS_IF)
    {
        _enable();
    }
}

STATIC
FORCEINLINE
ULONG64
MmuPageCount(
    _In_ PADDRESS_SPACE_DESCRIPTORS Space
)
{
    return Space->HeapLength / Space->PageSize;
}

STATIC
FORCEINLINE
BOOLEAN
MmuIsAmd(
    VOID
)
{
    CPUINFO Info;
    GetCpuInfo(&Info);
    return memcmp(Info.Vendor, CPU_AUTHENTIC_AMD, 12) == 0;
}

STATIC
FORCEINLINE
ULONG64
MmuStrideQwords(
    _In_ PADDRESS_SPACE_DESCRIPTORS Space
)
{
    return Space->PageSize / sizeof(ULONG64);
}

STATIC
FORCEINLINE
ULONG64
MmuReadSpaceCr3(
    _In_ PADDRESS_SPACE_DESCRIPTORS Space
)
{
    return Space->AddressSpace.Value;
}

STATIC
VOID
MmuEnterAddressSpace(
    _In_  PADDRESS_SPACE_DESCRIPTORS Space,
    _Out_ PULONG64 OldCr3
)
{
    *OldCr3 = __readcr3();
    __writecr3(MmuReadSpaceCr3(Space));
}

STATIC
VOID
MmuLeaveAddressSpace(
    _In_ ULONG64 OldCr3
)
{
    __writecr3(OldCr3);
}

STATIC
VOID
MmuFlushCurrentTlb(
    VOID
)
{
    ULONG64 Cr4 = __readcr4();

    // also try to flush global entries
    if (Cr4 & X86_CR4_PGE)
    {
        __writecr4(Cr4 & ~X86_CR4_PGE);
        __writecr4(Cr4);
    }
    else
    {
        ULONG64 Cr3 = __readcr3();
        __writecr3(Cr3);
    }
}

STATIC
ULONG64*
MmuHeapBuffer(
    _In_ PADDRESS_SPACE_DESCRIPTORS Space
)
{
    return (ULONG64*)(ULONG64)Space->HeapVirtualBase;
}

VOID
MmuFillLinearPointerChain(
    _In_ PADDRESS_SPACE_DESCRIPTORS Space
)
{
    ULONG64 OldCr3;
    ULONG64 Count;
    ULONG64 Stride;
    ULONG64* Buffer;
    ULONG64 Index;

    MmuEnterAddressSpace(Space, &OldCr3);

    Count = MmuPageCount(Space);
    Stride = MmuStrideQwords(Space);
    Buffer = MmuHeapBuffer(Space);

    if (Count < 2 || Stride == 0)
    {
        MmuLeaveAddressSpace(OldCr3);
        return;
    }

    for (Index = 0; Index < Count - 1; ++Index)
    {
        Buffer[Index * Stride] = (ULONG64)(ULONG64)&Buffer[(Index + 1) * Stride];
    }

    Buffer[(Count - 1) * Stride] = (ULONG64)(ULONG64)&Buffer[0];

    MmuLeaveAddressSpace(OldCr3);
}

STATIC
__declspec(noinline)
VOID
MmuWalkLinearPointerChainInner(
    _In_ ULONG64* Head,
    _In_ ULONG64 Iterations
)
{
    volatile ULONG64* P = Head;
    ULONG64 Index;

    for (Index = 0; Index < Iterations; ++Index)
    {
        P = (ULONG64*)(ULONG64)(*P);
    }

    gPointerSink = (ULONG64)(ULONG64)P;
}

STATIC
VOID
MmuZeroResult(
    _Out_ PMMU_STRESS_RESULT Result
)
{
    Result->TscDelta = 0;
    Result->DtlbLoadWalkCompleted = 0;
    Result->InstructionsRetired = 0;
    Result->CpuClockUnhalted = 0;
    Result->CpuClockReferenced = 0;
}

STATIC
VOID
MmuAddResult(
    _Inout_ PMMU_STRESS_RESULT Sum,
    _In_    CONST MMU_STRESS_RESULT* Value
)
{
    Sum->TscDelta += Value->TscDelta;
    Sum->DtlbLoadWalkCompleted += Value->DtlbLoadWalkCompleted;
    Sum->InstructionsRetired += Value->InstructionsRetired;
    Sum->CpuClockUnhalted += Value->CpuClockUnhalted;
    Sum->CpuClockReferenced += Value->CpuClockReferenced;
}

STATIC
VOID
MmuAverageResult(
    _Out_ PMMU_STRESS_RESULT Average,
    _In_  CONST MMU_STRESS_RESULT* Sum,
    _In_  ULONG64 Count
)
{
    if (Count == 0)
    {
        MmuZeroResult(Average);
        return;
    }

    Average->TscDelta = Sum->TscDelta / Count;
    Average->DtlbLoadWalkCompleted = Sum->DtlbLoadWalkCompleted / Count;
    Average->InstructionsRetired = Sum->InstructionsRetired / Count;
    Average->CpuClockUnhalted = Sum->CpuClockUnhalted / Count;
    Average->CpuClockReferenced = Sum->CpuClockReferenced / Count;
}

STATIC
ULONG64
MmuDisableCachesGlobal(
    VOID
)
{
    ULONG64 OldCr0 = __readcr0();

    __wbinvd();
    __writecr0((OldCr0 | X86_CR0_CD) & ~X86_CR0_NW);
    __wbinvd();

    return OldCr0;
}

STATIC
VOID
MmuRestoreCachesGlobal(
    _In_ ULONG64 OldCr0
)
{
    __wbinvd();
    __writecr0(OldCr0);
    __wbinvd();
}

//
// Shared log buffer helpers
//

STATIC
VOID
MmuLogReset(
    _In_ PBOOT_INFO BootInfo
)
{
    if (!BootInfo)
    {
        return;
    }

    BootInfo->MmuStressLog.Length = 0;
    BootInfo->MmuStressLog.Capacity = MMU_STRESS_LOG_TEXT_SIZE;
    BootInfo->MmuStressLog.Text[0] = '\0';
}

STATIC
VOID
MmuLogVPrintf(
    _In_ PBOOT_INFO BootInfo,
    _In_ PCSTR Format,
    _In_ va_list Args
)
{
    LONG32 Written;
    ULONG64 Remaining;
    PSTR Destination;

    if (!BootInfo || !Format)
    {
        return;
    }

    if (BootInfo->MmuStressLog.Capacity == 0)
    {
        return;
    }

    if (BootInfo->MmuStressLog.Length >= BootInfo->MmuStressLog.Capacity - 1)
    {
        return;
    }

    Remaining = BootInfo->MmuStressLog.Capacity - BootInfo->MmuStressLog.Length;
    Destination = &BootInfo->MmuStressLog.Text[BootInfo->MmuStressLog.Length];

    Written = vsnprintf(
        Destination,
        (LONG32)Remaining,
        Format,
        Args
    );

    if (Written <= 0)
    {
        return;
    }

    //
    // Clamp if truncated. Keep the final byte as NUL.
    //
    if ((ULONG64)Written >= Remaining)
    {
        BootInfo->MmuStressLog.Length = BootInfo->MmuStressLog.Capacity - 1;
        BootInfo->MmuStressLog.Text[BootInfo->MmuStressLog.Length] = '\0';
    }
    else
    {
        BootInfo->MmuStressLog.Length += (ULONG64)Written;
    }
}

STATIC
VOID
MmuLogPrintf(
    _In_ PBOOT_INFO BootInfo,
    _In_ PCSTR Format,
    ...
)
{
    va_list Args;
    va_start(Args, Format);
    MmuLogVPrintf(BootInfo, Format, Args);
    va_end(Args);
}

STATIC
VOID
MmuRunPointerPass(
    _In_  PADDRESS_SPACE_DESCRIPTORS Space,
    _In_  ULONG64 Iterations,
    _Out_ PMMU_STRESS_RESULT Result
)
{
    ULONG64 Start;
    ULONG64 End;
    ULONG64* Head;

    MmuZeroResult(Result);

    //
    // We are already inside the target CR3 when called.
    //
    Head = (ULONG64*)(ULONG64)Space->HeapVirtualBase;

    if (MmuIsAmd())
    {
        Start = TscStart();
        MmuWalkLinearPointerChainInner(Head, Iterations);
        End = TscEnd();
    }
    else
    {
        PmuIntelDtlbLoadWalkCompletedStartCounting();
        PmuIntelInstructionsRetiredStartCounting();
        PmuIntelCpuClockUnhaltedStartCounting();
        PmuIntelCpuClockReferencedStartCounting();

        Start = TscStart();
        MmuWalkLinearPointerChainInner(Head, Iterations);
        End = TscEnd();

        PmuIntelCpuClockReferencedStopCounting(&Result->CpuClockReferenced);
        PmuIntelCpuClockUnhaltedStopCounting(&Result->CpuClockUnhalted);
        PmuIntelInstructionsRetiredStopCounting(&Result->InstructionsRetired);
        PmuIntelDtlbLoadWalkCompletedStopCounting(&Result->DtlbLoadWalkCompleted);
    }

    Result->TscDelta = End - Start;
}

STATIC
VOID
MmuRunColdWarmPair(
    _In_  PADDRESS_SPACE_DESCRIPTORS Space,
    _In_  ULONG64 Iterations,
    _In_  BOOLEAN DisableCachesGlobal,
    _Out_ PMMU_STRESS_RESULT Cold,
    _Out_ PMMU_STRESS_RESULT Warm
)
{
    ULONG64 OldCr0 = 0;

    MmuZeroResult(Cold);
    MmuZeroResult(Warm);

    //
    // We are already in the test CR3 when this is called.
    //
    if (DisableCachesGlobal)
    {
        OldCr0 = MmuDisableCachesGlobal();
    }

    //
    // Force the first pass cold in translation terms.
    //
    MmuFlushCurrentTlb();

    MmuRunPointerPass(Space, Iterations, Cold);
    MmuRunPointerPass(Space, Iterations, Warm);

    if (DisableCachesGlobal)
    {
        MmuRestoreCachesGlobal(OldCr0);
    }
}


STATIC
LONG32
MmuRunProfileForSpace(
    _In_ PBOOT_INFO BootInfo,
    _In_ PADDRESS_SPACE_DESCRIPTORS Space,
    _In_ CONST MMU_TEST_PROFILE* Profile
)
{
    ULONG64 Iterations;
    MMU_STRESS_RESULT ColdSum;
    MMU_STRESS_RESULT WarmSum;
    MMU_STRESS_RESULT ColdAvg;
    MMU_STRESS_RESULT WarmAvg;
    ULONG32 Rep;

    if (!BootInfo || !Space || !Profile)
    {
        return -1;
    }

    MmuZeroResult(&ColdSum);
    MmuZeroResult(&WarmSum);
    MmuZeroResult(&ColdAvg);
    MmuZeroResult(&WarmAvg);

    Iterations = MmuPageCount(Space);
    if (Iterations == 0)
    {
        return -1;
    }

    //
    // Build the pointer chain once before the measurement loop.
    //
    ULONG64 OldIrqlFlags = MmuDisableInterruptsSave();
    MmuFillLinearPointerChain(Space);
    MmuRestoreInterrupts(OldIrqlFlags);

    MmuLogPrintf(
        BootInfo,
        "PROFILE=%s Set=%llu CR3=0x%llx HeapVA=0x%llx HeapPA=0x%llx HeapLen=0x%llx PageSize=0x%llx Repeats=%u\r\n",
        Profile->Name,
        Space->SetIdentifier,
        Space->AddressSpace.Value,
        Space->HeapVirtualBase,
        Space->HeapPhysicalBase,
        Space->HeapLength,
        Space->PageSize,
        Profile->RepeatCount
    );

    for (Rep = 0; Rep < Profile->RepeatCount; ++Rep)
    {
        ULONG64 OldCr3;
        ULONG64 InterruptFlags;
        MMU_STRESS_RESULT Cold;
        MMU_STRESS_RESULT Warm;

        MmuZeroResult(&Cold);
        MmuZeroResult(&Warm);

        InterruptFlags = MmuDisableInterruptsSave();

        MmuEnterAddressSpace(Space, &OldCr3);

        MmuRunColdWarmPair(
            Space,
            Iterations,
            Profile->DisableCachesGlobal,
            &Cold,
            &Warm
        );

        MmuLeaveAddressSpace(OldCr3);

        MmuRestoreInterrupts(InterruptFlags);

        MmuAddResult(&ColdSum, &Cold);
        MmuAddResult(&WarmSum, &Warm);

        MmuLogPrintf(
            BootInfo,
            "  REP=%u COLD TSC=%llu DTLBWalk=%llu Instr=%llu Unhalted=%llu Ref=%llu\r\n",
            Rep,
            Cold.TscDelta,
            Cold.DtlbLoadWalkCompleted,
            Cold.InstructionsRetired,
            Cold.CpuClockUnhalted,
            Cold.CpuClockReferenced
        );

        MmuLogPrintf(
            BootInfo,
            "  REP=%u WARM TSC=%llu DTLBWalk=%llu Instr=%llu Unhalted=%llu Ref=%llu\r\n",
            Rep,
            Warm.TscDelta,
            Warm.DtlbLoadWalkCompleted,
            Warm.InstructionsRetired,
            Warm.CpuClockUnhalted,
            Warm.CpuClockReferenced
        );
    }

    MmuAverageResult(&ColdAvg, &ColdSum, Profile->RepeatCount);
    MmuAverageResult(&WarmAvg, &WarmSum, Profile->RepeatCount);

    MmuLogPrintf(
        BootInfo,
        "  AVG COLD TSC=%llu DTLBWalk=%llu Instr=%llu Unhalted=%llu Ref=%llu\r\n",
        ColdAvg.TscDelta,
        ColdAvg.DtlbLoadWalkCompleted,
        ColdAvg.InstructionsRetired,
        ColdAvg.CpuClockUnhalted,
        ColdAvg.CpuClockReferenced
    );

    MmuLogPrintf(
        BootInfo,
        "  AVG WARM TSC=%llu DTLBWalk=%llu Instr=%llu Unhalted=%llu Ref=%llu\r\n\r\n",
        WarmAvg.TscDelta,
        WarmAvg.DtlbLoadWalkCompleted,
        WarmAvg.InstructionsRetired,
        WarmAvg.CpuClockUnhalted,
        WarmAvg.CpuClockReferenced
    );

    return 0;
}

LONG32
MmuRunAllBenchmarks(
    _In_ PBOOT_INFO BootInfo
)
{
    ULONG64 Index;

    STATIC CONST MMU_TEST_PROFILE Profiles[] =
    {
        { "baseline",           FALSE, 5 },
        { "cr0_cache_disabled", TRUE,  5 },
    };

    if (!BootInfo ||
        !BootInfo->MmuStresserDescriptors.AddressSpaces ||
        !BootInfo->MmuStresserDescriptors.AddressSpaceCount)
    {
        return -1;
    }

    MmuLogReset(BootInfo);

    MmuLogPrintf(BootInfo, "MMU STRESS BEGIN\r\n");
    MmuLogPrintf(
        BootInfo,
        "AddressSpaceCount=%llu\r\n\r\n",
        BootInfo->MmuStresserDescriptors.AddressSpaceCount
    );

    for (ULONG64 ProfileIndex = 0; ProfileIndex < sizeof(Profiles) / sizeof(Profiles[0]); ++ProfileIndex)
    {
        MmuLogPrintf(
            BootInfo,
            "----- BEGIN PROFILE: %s -----\r\n",
            Profiles[ProfileIndex].Name
        );

        for (Index = 0; Index < BootInfo->MmuStresserDescriptors.AddressSpaceCount; ++Index)
        {
            PADDRESS_SPACE_DESCRIPTORS Space =
                &BootInfo->MmuStresserDescriptors.AddressSpaces[Index];

            if (MmuRunProfileForSpace(BootInfo, Space, &Profiles[ProfileIndex]) != 0)
            {
                MmuLogPrintf(
                    BootInfo,
                    "FAILED AddressSpaceIndex=%llu\r\n",
                    Index
                );
            }
        }

        MmuLogPrintf(
            BootInfo,
            "----- END PROFILE: %s -----\r\n\r\n",
            Profiles[ProfileIndex].Name
        );
    }

    MmuLogPrintf(BootInfo, "MMU STRESS END\r\n");

    return 0;
}