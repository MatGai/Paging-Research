#ifndef KRNL_MSR_H
#define KRNL_MSR_H

#include <scouse/shared/typedefs.h>

#define AMD_MSR_PERF_CTL( Value ) (0xC0010200u + ((ULONG32)( Value ) * 2u))
#define AMD_MSR_PERF_CTR( Value ) (0xC0010201u + ((ULONG32)( Value ) * 2u))


#define AMD_PMC_OSUSER_NONE       0u    
#define AMD_PMC_OSUSER_USER       1u    
#define AMD_PMC_OSUSER_OS         2u    
#define AMD_PMC_OSUSER_ALL        3u    

#define AMD_PMC_EVT_L1_DTLB        0x45u
#define AMD_PMC_UMASK_L1_DTLB_MISS 0xFFu   
#define AMD_PMC_UMASK_L2_DTLB_MISS 0xF0u   

#define AMD_PMC_EVT_L1_ITLB_MISS_L2_HIT   0x84u
#define AMD_PMC_UMASK_L1_ITLB_MISS_L2_HIT 0x00u

#define AMD_PMC_EVT_L2_ITLB_MISS          0x85u
#define AMD_PMC_UMASK_L2_ITLB_MISS        0x07u

#define AMD_PMC_EVT_ALL_TLB_FLUSH   0x78u
#define AMD_PMC_UMASK_ALL_TLB_FLUSH 0xFFu

#define AMD_PMC_CTR_MASK  ((1ull << 48) - 1ull)

VOID
MsrAmdPmcConfigure(
	ULONG32 PmcIndex,
	UINT8 EventSelect,
	UINT8 Unitmask,
	UINT8 OsUserMode
);

VOID
AmdDtlbMissStartCounting(
	VOID
);

VOID
AmdDtlbMissStopCounting(
	ULONG64* L1DtlbMisses,
	ULONG64* L2DtlbMisses
);

VOID
AmdItlbMissStartCounting(
	VOID
);

VOID
AmdItlbMissStopCounting(
	ULONG64* L1ItlbMisses,
	ULONG64* L2ItlbMisses
);

VOID
MsrAmdPmcDisable(
	ULONG32 PmcIndex
);

ULONG64
MsrAmdPmcRead(
	ULONG32 PmcIndex
);

VOID
AmdCyclesStartCounting(
	VOID
);

VOID AmdCyclesStopCount(
	ULONG64* CyclesNotHalted,
	ULONG64* RetiredInstructions
);

#endif // !KRNL_MSR_H