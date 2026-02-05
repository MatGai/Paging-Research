#ifndef KRNL_PAGEMANAGER_H
#define KRNL_PAGEMANAGER_H

#include <scouse/shared/typedefs.h>

typedef enum _PFN_STATE
{
    Free = 0,
    Allocated = 1,
    Reserved = 2
} PFN_STATE;

typedef struct _PFN_ENTRY
{
    PFN_STATE State;
    ULONG32   Offset;  
    ULONG32   Ref;
} PFN_ENTRY, * PPFN_ENTRY;

extern PPFN_ENTRY SsPfn;
extern ULONG64    SsPfnCount;
extern ULONG64    SsPfnFreeHead;

#define PML4_INDEX(Value) (((ULONG64)(Value) >> 39) & 0x1FFull)
#define PDPT_INDEX(Value) (((ULONG64)(Value) >> 30) & 0x1FFull)
#define PD_INDEX(Value)   (((ULONG64)(Value) >> 21) & 0x1FFull)
#define PT_INDEX(Value)   (((ULONG64)(Value) >> 12) & 0x1FFull)

#define DEFAULT_PAGE_SIZE 0x1000ull
#define LARGE_PAGE_SIZE   0x200000ull
#define HUGE_PAGE_SIZE    0x40000000ull 

#define PAGE_SHIFT 12ull
#define PFN_LIST_END 0xFFFFFFull 

#define PFN_TO_PHYSICAL_SIZE(pfn, size) ((ULONG64)(pfn) << (size))
#define PFN_TO_PHYSICAL(pfn)            (PFN_TO_PHYSICAL_SIZE((pfn), PAGE_SHIFT))

#define PHYSICAL_TO_PFN_SIZE(adr, size) ((ULONG64)(adr) >> (size))
#define PHYSICAL_TO_PFN(adr)            (PHYSICAL_TO_PFN_SIZE((adr), PAGE_SHIFT))

#define ALIGN_NEXT_PAGE(Value) (((ULONG64)(Value) + DEFAULT_PAGE_SIZE - 1) & ~0xFFFull)
#define ALIGN_PAGE(Value)      ((ULONG64)(Value) & ~0xFFFull)

#define PAGE_FLAG_PRESENT (1ull << 0)
#define PAGE_FLAG_RW      (1ull << 1)
#define PAGE_FLAG_USER    (1ull << 2)
#define PAGE_FLAG_PS      (1ull << 7)
#define PAGE_FLAG_NX      (1ull << 63)

#define PAGE_PHYSICAL_MASK 0x000FFFFFFFFFF000ull

#define MAKE_PAGE_ENTRY(Entry, NextTablePhysical) \
    (Entry) = (((NextTablePhysical) & PAGE_PHYSICAL_MASK) | (PAGE_FLAG_PRESENT | PAGE_FLAG_RW))

#define MAKE_LEAF_ENTRY(Entry, PagePhysical, Flags) \
    (Entry) = (((PagePhysical) & PAGE_PHYSICAL_MASK) | ((Flags) & ~PAGE_PHYSICAL_MASK))

extern ULONG64  gDirectMapBase;
extern ULONG64* gPML4;         


VOID 
KrnlPagingInit(
    ULONG64 DirectMapBase,
    ULONG64 Pml4Physical,
    ULONG64 PfnArrayPhysical,
    ULONG64 PfnCount,
    ULONG64 PfnFreeHead
);


PVOID 
KrnlPhysicalToVirtual(
    ULONG64 PhysicalAddress
);


ULONG64 
SsGetFreePhysicalPage(
    VOID
);
VOID    
SsFreePhysicalPage(
    ULONG64 PhysicalAddress
);

LONG32     
KrnlAllocPage(
    ULONG64* OutPhysical, 
    void** OutVirtual /* can be NULL */
);

LONG32 
KrnlMapPage(
    ULONG64 VirtualAddress, 
    ULONG64 PhysicalAddress, 
    ULONG64 Flags
);

LONG32 
KrnlMapLargePage(
    ULONG64 VirtualAddress, 
    ULONG64 PhysicalAddress, 
    ULONG64 Flags
);

LONG32 
KrnlMapHugePage(
    ULONG64 VirtualAddress, 
    ULONG64 PhysicalAddress, 
    ULONG64 Flags
);

#endif //!KRNL_PAGEMANAGER_H
