#include <scouse/archx64/cpu/pmu/pmuamd.h>

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

#define AMD_PMC_EVT_CYCLES_NOT_HALTED      0x76u
#define AMD_PMC_UMASK_CYCLES_NOT_HALTED    0x00u
#define AMD_PMC_EVT_RETIRED_INSTRUCTIONS   0xC0u
#define AMD_PMC_UMASK_RETIRED_INSTRUCTIONS 0x00u

#define AMD_PMC_CTR_MASK	   ((1ull << 48) - 1ull)
#define AMD_PMC_CTL_ENABLE_BIT (1ull << 22)

VOID
MsrAmdPmcConfigure(
	ULONG32 PmcIndex,
	UINT8 EventSelect,
	UINT8 Unitmask,
	UINT8 OsUserMode
)
{
	ULONG32 MsrCtl = AMD_MSR_PERF_CTL(PmcIndex);
	ULONG32 MsrCtr = AMD_MSR_PERF_CTR(PmcIndex);

	ULONG64 Ctl = __readmsr(MsrCtl);
	Ctl &= ~(1ull << 22); // disable counter
	__writemsr(MsrCtl, Ctl);

	__writemsr(MsrCtr, 0); // reset counter

	ULONG64 NewCtl = 0;
	NewCtl |= (ULONG64)EventSelect;
	NewCtl |= (ULONG64)Unitmask << 8;
	NewCtl |= (ULONG64)(OsUserMode & 0x3u) << 16;

	NewCtl |= (1ull << 22); // enable counter

	__writemsr(MsrCtl, NewCtl);
}

VOID
AmdDtlbMissStartCounting(
	VOID
)
{
	MsrAmdPmcConfigure(
		0,
		AMD_PMC_EVT_L1_DTLB,
		AMD_PMC_UMASK_L1_DTLB_MISS,
		AMD_PMC_OSUSER_ALL
	);

	MsrAmdPmcConfigure(
		1,
		AMD_PMC_EVT_L1_DTLB,
		AMD_PMC_UMASK_L2_DTLB_MISS,
		AMD_PMC_OSUSER_ALL
	);
}

VOID
AmdDtlbMissStopCounting(
	ULONG64* L1DtlbMisses,
	ULONG64* L2DtlbMisses
)
{
	ULONG64 MsrCtr0 = __readmsr(AMD_MSR_PERF_CTR(0)) & AMD_PMC_CTR_MASK;
	ULONG64 MsrCtr1 = __readmsr(AMD_MSR_PERF_CTR(1)) & AMD_PMC_CTR_MASK;

	ULONG64 MsrCtl0 = __readmsr(AMD_MSR_PERF_CTL(0));
	ULONG64 MsrCtl1 = __readmsr(AMD_MSR_PERF_CTL(1));

	MsrCtl0 &= ~(1ull << 22); // disable counter
	MsrCtl1 &= ~(1ull << 22); // disable counter

	__writemsr(AMD_MSR_PERF_CTL(0), MsrCtl0);
	__writemsr(AMD_MSR_PERF_CTL(1), MsrCtl1);

	if (L1DtlbMisses)
	{
		*L1DtlbMisses = MsrCtr0;
	}
	if (L2DtlbMisses)
	{
		*L2DtlbMisses = MsrCtr1;
	}
}

VOID
AmdItlbMissStartCounting(
	VOID
)
{
	MsrAmdPmcConfigure(
		0,
		AMD_PMC_EVT_L1_ITLB_MISS_L2_HIT,
		AMD_PMC_UMASK_L1_ITLB_MISS_L2_HIT,
		AMD_PMC_OSUSER_ALL
	);

	MsrAmdPmcConfigure(
		1,
		AMD_PMC_EVT_L2_ITLB_MISS,
		AMD_PMC_UMASK_L2_ITLB_MISS,
		AMD_PMC_OSUSER_ALL
	);
}

VOID
AmdItlbMissStopCounting(
	ULONG64* L1ItlbMisses,
	ULONG64* L2ItlbMisses
)
{
	ULONG64 MsrCtr0 = __readmsr(AMD_MSR_PERF_CTR(0)) & AMD_PMC_CTR_MASK;
	ULONG64 MsrCtr1 = __readmsr(AMD_MSR_PERF_CTR(1)) & AMD_PMC_CTR_MASK;

	ULONG64 MsrCtl0 = __readmsr(AMD_MSR_PERF_CTL(0));
	ULONG64 MsrCtl1 = __readmsr(AMD_MSR_PERF_CTL(1));

	MsrCtl0 &= ~(1ull << 22);
	MsrCtl1 &= ~(1ull << 22);

	__writemsr(AMD_MSR_PERF_CTL(0), MsrCtl0);
	__writemsr(AMD_MSR_PERF_CTL(1), MsrCtl1);

	if (L1ItlbMisses)
	{
		*L1ItlbMisses = MsrCtr0;
	}
	if (L2ItlbMisses)
	{
		*L2ItlbMisses = MsrCtr1;
	}
}

VOID
MsrAmdPmcDisable(
	ULONG32 PmcIndex
)
{
	ULONG32 msrCtl = AMD_MSR_PERF_CTL(PmcIndex);
	ULONG64 ctl = __readmsr(msrCtl);
	ctl &= ~(1ull << 22);
	__writemsr(msrCtl, ctl);
}

ULONG64
MsrAmdPmcRead(
	ULONG32 PmcIndex
)
{
	return __readmsr(AMD_MSR_PERF_CTR(PmcIndex)) & AMD_PMC_CTR_MASK;
}

VOID
AmdCyclesStartCounting(
	VOID
)
{
	MsrAmdPmcConfigure(2, AMD_PMC_EVT_CYCLES_NOT_HALTED, AMD_PMC_UMASK_CYCLES_NOT_HALTED, AMD_PMC_OSUSER_ALL);
	MsrAmdPmcConfigure(3, AMD_PMC_EVT_RETIRED_INSTRUCTIONS, AMD_PMC_UMASK_RETIRED_INSTRUCTIONS, AMD_PMC_OSUSER_ALL);
}

VOID AmdCyclesStopCount(
	ULONG64* CyclesNotHalted,
	ULONG64* RetiredInstructions
)
{
	ULONG64 Cycles       = MsrAmdPmcRead(0x2);
	ULONG64 Instructions = MsrAmdPmcRead(0x3);

	MsrAmdPmcDisable(0x2);
	MsrAmdPmcDisable(0x3);

	if( CyclesNotHalted )
	{
        *CyclesNotHalted = Cycles;
	}

	if ( RetiredInstructions )
	{
		*RetiredInstructions = Instructions;
	}
}