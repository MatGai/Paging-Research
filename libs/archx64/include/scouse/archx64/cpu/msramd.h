#ifndef KRNL_MSR_H
#define KRNL_MSR_H

#include <stdint.h>
#include <intrin.h>

#define AMD_MSR_PERF_CTL( Value ) (0xC0010200u + ((unsigned __int32)( Value ) * 2u))
#define AMD_MSR_PERF_CTR( Value ) (0xC0010201u + ((unsigned __int32)( Value ) * 2u))


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

void
MsrAmdPmcConfigure(
	unsigned __int32 PmcIndex,
	unsigned __int8 EventSelect,
	unsigned __int8 Unitmask,
	unsigned __int8 OsUserMode
);

void
AmdDtlbMissStartCounting(
	void
);

void
AmdDtlbMissStopCounting(
	unsigned __int64* L1DtlbMisses,
	unsigned __int64* L2DtlbMisses
);

void
AmdItlbMissStartCounting(
	void
);

void
AmdItlbMissStopCounting(
	unsigned __int64* L1ItlbMisses,
	unsigned __int64* L2ItlbMisses
);

void
MsrAmdPmcDisable(
	unsigned __int32 PmcIndex
);

unsigned __int64
MsrAmdPmcRead(
	unsigned __int32 PmcIndex
);

void
AmdCyclesStartCounting(
	void
);

void AmdCyclesStopCount(
	unsigned __int64* CyclesNotHalted,
	unsigned __int64* RetiredInstructions
);

#endif // !KRNL_MSR_H