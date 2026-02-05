#include "pagemanager.h"

/* You can replace this with your runtime memset/ZeroMem if you have one */
static 
__forceinline 
VOID 
KnrlZeroPage(
    PVOID p
)
{
    volatile UINT8* b = (volatile UINT8*)p;
    for (ULONG64 i = 0; i < DEFAULT_PAGE_SIZE; ++i) b[i] = 0;
}

/* Globals (single definitions) */
PPFN_ENTRY SsPfn = (PPFN_ENTRY)0;
ULONG64    SsPfnCount = 0;
ULONG64    SsPfnFreeHead = PFN_LIST_END;

ULONG64    gDirectMapBase = 0;
ULONG64* gPML4 = (ULONG64*)0;

PVOID
KrnlPhysicalToVirtual(
    ULONG64 PhysicalAddress
)
{
    return (PVOID)(ULONG64)(gDirectMapBase + PhysicalAddress);
}

VOID
KrnlPagingInit(
    ULONG64 DirectMapBase,
    ULONG64 Pml4Physical,
    ULONG64 PfnArrayPhysical,
    ULONG64 PfnCount,
    ULONG64 PfnFreeHead
)
{
    gDirectMapBase = DirectMapBase;

    gPML4 = (ULONG64*)KrnlPhysicalToVirtual(Pml4Physical);
    SsPfn = (PPFN_ENTRY)KrnlPhysicalToVirtual(PfnArrayPhysical);

    SsPfnCount = PfnCount;
    SsPfnFreeHead = PfnFreeHead;
}

ULONG64 
SsGetFreePhysicalPage(
    VOID
)
{
    if (SsPfnFreeHead == PFN_LIST_END)
        return 0;

    ULONG64 pfn = SsPfnFreeHead;
    ULONG64 page_pa = PFN_TO_PHYSICAL(pfn);

    PFN_ENTRY* e = &SsPfn[pfn];
    SsPfnFreeHead = (ULONG64)e->Offset;

    e->State = Allocated;
    e->Ref = 1;
    e->Offset = (ULONG32)PFN_LIST_END;

    return page_pa;
}

VOID 
SsFreePhysicalPage(
    ULONG64 PhysicalAddress
)
{
    ULONG64 pfn = PHYSICAL_TO_PFN(PhysicalAddress);
    if (pfn >= SsPfnCount)
        return;

    PFN_ENTRY* e = &SsPfn[pfn];
    e->State = Free;
    e->Ref = 0;
    e->Offset = (ULONG32)SsPfnFreeHead;
    SsPfnFreeHead = pfn;
}

int KrnlAllocPage(ULONG64* OutPhysical, void** OutVirtual)
{
    if (!OutPhysical) return 0;

    ULONG64 pa = SsGetFreePhysicalPage();
    if (!pa) return 0;

    *OutPhysical = pa;

    if (OutVirtual)
        *OutVirtual = KrnlPhysicalToVirtual(pa);

    return 1;
}


static 
LONG32
KrnlMapTables(
    ULONG64* Pml4Va,
    ULONG64 VirtualAddress,
    ULONG64 PhysicalAddress,
    ULONG64 TableEnd, 
    ULONG64 Flags
)
{
    if (!Pml4Va)
    {
        return 0;
    }

    if (TableEnd < 1 || TableEnd > 3)
    {
        return 0;
    }

    if (TableEnd == 1) 
    { 
        if ((VirtualAddress & (HUGE_PAGE_SIZE - 1)) != 0) return 0;
        if ((PhysicalAddress & (HUGE_PAGE_SIZE - 1)) != 0) return 0;
        Flags |= PAGE_FLAG_PS;
    }
    else if (TableEnd == 2) 
    { 
        if ((VirtualAddress & (LARGE_PAGE_SIZE - 1)) != 0) return 0;
        if ((PhysicalAddress & (LARGE_PAGE_SIZE - 1)) != 0) return 0;
        Flags |= PAGE_FLAG_PS;
    }
    else 
    {
        if ((VirtualAddress & (DEFAULT_PAGE_SIZE - 1)) != 0) return 0;
        if ((PhysicalAddress & (DEFAULT_PAGE_SIZE - 1)) != 0) return 0;
    }

    ULONG64 idx[4] = 
    {
        PML4_INDEX(VirtualAddress),
        PDPT_INDEX(VirtualAddress),
        PD_INDEX(VirtualAddress),
        PT_INDEX(VirtualAddress)
    };

    ULONG64* table = Pml4Va;

    for (ULONG32 level = 0; level < (ULONG32)TableEnd; ++level)
    {
        ULONG64 e = table[idx[level]];

        if (!(e & PAGE_FLAG_PRESENT))
        {
            ULONG64 next_pa = 0;
            void* next_va = 0;

            if (!KrnlAllocPage(&next_pa, &next_va))
                return 0;

            KnrlZeroPage(next_va);

            MAKE_PAGE_ENTRY(table[idx[level]], next_pa);
            e = table[idx[level]];
        }

        table = (ULONG64*)KrnlPhysicalToVirtual(e & PAGE_PHYSICAL_MASK);
    }

    MAKE_LEAF_ENTRY(table[idx[TableEnd]], PhysicalAddress, Flags);
    return 1;
}

LONG32 
KrnlMapPage(
    ULONG64 VirtualAddress, 
    ULONG64 PhysicalAddress, 
    ULONG64 Flags
)
{
    return KrnlMapTables(gPML4, VirtualAddress, PhysicalAddress, 3, Flags);
}

LONG32 
KrnlMapLargePage(
    ULONG64 VirtualAddress, 
    ULONG64 PhysicalAddress, 
    ULONG64 Flags
)
{
    return KrnlMapTables(gPML4, VirtualAddress, PhysicalAddress, 2, Flags);
}

LONG32
KrnlMapHugePage(
    ULONG64 VirtualAddress, 
    ULONG64 PhysicalAddress, 
    ULONG64 Flags
)
{
    return KrnlMapTables(gPML4, VirtualAddress, PhysicalAddress, 1, Flags);
}
