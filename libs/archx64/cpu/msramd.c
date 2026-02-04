#include <scouse/archx64/cpu/msramd.h>

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

#define AMD_PMC_EVT_CYCLES_NOT_HALTED      0x76u
#define AMD_PMC_UMASK_CYCLES_NOT_HALTED    0x00u
#define AMD_PMC_EVT_RETIRED_INSTRUCTIONS   0xC0u
#define AMD_PMC_UMASK_RETIRED_INSTRUCTIONS 0x00u

#define AMD_PMC_CTR_MASK	   ((1ull << 48) - 1ull)
#define AMD_PMC_CTL_ENABLE_BIT (1ull << 22)

void
MsrAmdPmcConfigure(
	unsigned __int32 PmcIndex,
	unsigned __int8 EventSelect,
	unsigned __int8 Unitmask,
	unsigned __int8 OsUserMode
)
{
	unsigned __int32 MsrCtl = AMD_MSR_PERF_CTL(PmcIndex);
	unsigned __int32 MsrCtr = AMD_MSR_PERF_CTR(PmcIndex);

	unsigned __int64 Ctl = __readmsr(MsrCtl);
	Ctl &= ~(1ull << 22); // disable counter
	__writemsr(MsrCtl, Ctl);

	__writemsr(MsrCtr, 0); // reset counter

	unsigned __int64 NewCtl = 0;
	NewCtl |= (unsigned __int64)EventSelect;
	NewCtl |= (unsigned __int64)Unitmask << 8;
	NewCtl |= (unsigned __int64)(OsUserMode & 0x3u) << 16;

	NewCtl |= (1ull << 22); // enable counter

	__writemsr(MsrCtl, NewCtl);
}

void
AmdDtlbMissStartCounting(
	void
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

void
AmdDtlbMissStopCounting(
	unsigned __int64* L1DtlbMisses,
	unsigned __int64* L2DtlbMisses
)
{
	unsigned __int64 MsrCtr0 = __readmsr(AMD_MSR_PERF_CTR(0)) & AMD_PMC_CTR_MASK;
	unsigned __int64 MsrCtr1 = __readmsr(AMD_MSR_PERF_CTR(1)) & AMD_PMC_CTR_MASK;

	unsigned __int64 MsrCtl0 = __readmsr(AMD_MSR_PERF_CTL(0));
	unsigned __int64 MsrCtl1 = __readmsr(AMD_MSR_PERF_CTL(1));

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

void
AmdItlbMissStartCounting(
	void
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

void
AmdItlbMissStopCounting(
	unsigned __int64* L1ItlbMisses,
	unsigned __int64* L2ItlbMisses
)
{
	unsigned __int64 MsrCtr0 = __readmsr(AMD_MSR_PERF_CTR(0)) & AMD_PMC_CTR_MASK;
	unsigned __int64 MsrCtr1 = __readmsr(AMD_MSR_PERF_CTR(1)) & AMD_PMC_CTR_MASK;

	unsigned __int64 MsrCtl0 = __readmsr(AMD_MSR_PERF_CTL(0));
	unsigned __int64 MsrCtl1 = __readmsr(AMD_MSR_PERF_CTL(1));

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

void
MsrAmdPmcDisable(
	unsigned __int32 PmcIndex
)
{
	unsigned __int32 msrCtl = AMD_MSR_PERF_CTL(PmcIndex);
	unsigned __int64 ctl = __readmsr(msrCtl);
	ctl &= ~(1ull << 22);
	__writemsr(msrCtl, ctl);
}

unsigned __int64
MsrAmdPmcRead(
	unsigned __int32 PmcIndex
)
{
	return __readmsr(AMD_MSR_PERF_CTR(PmcIndex)) & AMD_PMC_CTR_MASK;
}

void
AmdCyclesStartCounting(
	void
)
{
	MsrAmdPmcConfigure(2, AMD_PMC_EVT_CYCLES_NOT_HALTED, AMD_PMC_UMASK_CYCLES_NOT_HALTED, AMD_PMC_OSUSER_ALL);
	MsrAmdPmcConfigure(3, AMD_PMC_EVT_RETIRED_INSTRUCTIONS, AMD_PMC_UMASK_RETIRED_INSTRUCTIONS, AMD_PMC_OSUSER_ALL);
}

void AmdCyclesStopCount(
	unsigned __int64* CyclesNotHalted,
	unsigned __int64* RetiredInstructions
)
{
	unsigned __int64 Cycles       = MsrAmdPmcRead(0x2);
	unsigned __int64 Instructions = MsrAmdPmcRead(0x3);

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