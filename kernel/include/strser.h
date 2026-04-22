#ifndef STRSER_H
#define STRSER_H

#include <scouse/archx64/paging.h>
#include <scouse/shared/bootinfo.h>
#include <scouse/shared/typedefs.h>
#include <timer.h>

typedef struct _MMU_STRESS_RESULT
{
    ULONG64 TscDelta;
    ULONG64 DtlbLoadWalkCompleted;
    ULONG64 InstructionsRetired;
    ULONG64 CpuClockUnhalted;
    ULONG64 CpuClockReferenced;
} MMU_STRESS_RESULT, * PMMU_STRESS_RESULT;

typedef struct _MMU_TEST_PROFILE
{
    PCSTR   Name;
    BOOLEAN DisableCachesGlobal;
    ULONG32 RepeatCount;
} MMU_TEST_PROFILE;

VOID
MmuFillLinearPointerChain(
    _In_ PADDRESS_SPACE_DESCRIPTORS Space
);

VOID
MmuWalkLinearPointerChain(
    _In_ PADDRESS_SPACE_DESCRIPTORS Space,
    _In_ ULONG64 Iterations
);

VOID
MmuFillRandomPointerChain(
    _In_ PADDRESS_SPACE_DESCRIPTORS Space,
    _In_ ULONG64 Seed
);

VOID
MmuWalkRandomPointerChain(
    _In_ PADDRESS_SPACE_DESCRIPTORS Space,
    _In_ ULONG64 Iterations
);

VOID
MmuFillLinearJmpChain(
    _In_ PADDRESS_SPACE_DESCRIPTORS Space
);

VOID
MmuRunLinearJmpChain(
    _In_ PADDRESS_SPACE_DESCRIPTORS Space
);

LONG32
MmuRunPointerChainBenchmark(
    _In_  PADDRESS_SPACE_DESCRIPTORS Space,
    _Out_ PMMU_STRESS_RESULT Cold,
    _Out_ PMMU_STRESS_RESULT Warm
);

LONG32
MmuRunAllBenchmarks(
    _In_ PBOOT_INFO BootInfo
);

#endif