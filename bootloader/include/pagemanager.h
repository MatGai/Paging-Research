#ifndef PAGEMANAGER_H
#define PAGEMANAGER_H

#include <Library/BaseMemoryLib.h>
#include <scouse/archx64/paging.h>
#include <scouse/shared/bootinfo.h>

#include "boot.h"
#include "pe.h"

EXTERN_C
ULONG64
scouse_transition_address_space(
    _In_ ULONG64 AddressSpace,
    _In_ ULONG64 StackSpace,
    _In_ ULONG64 EntryPoint,
    _In_ PBOOT_INFO BootInfo
);

typedef ULONG64(BLAPI* SCOUSE_TRANSITION_ADDRESS_SPACE)(ULONG64, ULONG64, ULONG64, PBOOT_INFO);

EXTERN_C CONST BYTE scouse_transition_address_space_start[];
EXTERN_C CONST BYTE scouse_transition_address_space_end[];

typedef enum _PFN_STATE
{
    Free, Allocated, Reserved
} PFN_STATE;

typedef struct _PFN_ENTRY
{
    PFN_STATE State;
    UINT32 Offset;
    UINT32 Ref;
} PFN_ENTRY, * PPFN_ENTRY;

PPFN_ENTRY SsPfn;
ULONG64 SsPfnCount;
ULONG64 SsPfnFreeHead;

#define PFN_LIST_END 0xFFFFFF
                     
#define PFN_TO_PHYSICAL_SIZE(pfn, size) ((pfn) << size)
#define PFN_TO_PHYSICAL(pfn) (PFN_TO_PHYSICAL_SIZE(pfn, MM_PAGE_SHIFT))

#define PHYSICAL_TO_PFN_SIZE(adr, size) ((adr) >> size)
#define PHYSICAL_TO_PFN(adr) (PHYSICAL_TO_PFN_SIZE(adr, MM_PAGE_SHIFT))

#define LOW_MEMORY_START 0x0000000000000000ULL
#define LOW_MEMORY_END   0x00007FFFFFFFFFFFULL

#define HIGH_MEMORY_START 0xFFFF800000000000ULL
#define HIGH_MEMORY_END   0xFFFFFFFFFFFFFFFFULL
#define PML4_HIGH_MEMORY_START 256 // PML4[ 256 ] = 0xFFFF_8000_0000_0000

#define DIRECT_MAP_BASE HIGH_MEMORY_START

#define KERNEL_VA_BASE 0xFFFFFFFF80000000ULL
#define KERNEL_VA_STACK_TOP ( KERNEL_VA_BASE - 0x1000 - 0x10 )
#define KERNEL_VA_STACK ( KERNEL_VA_BASE - 0x2000 )

/**
 * Converst a physical address to a virtual address
 * in the direct mapped region.
 * Only works after paging has been initialized.
 *
 * @param PhysicalAddress A physical address to convert.
 *
 * @return A virtual address in the direct mapped region.
 */
STATIC
__forceinline
VOID*
BLAPI
PhysicalToVirtual(
    _In_ ULONG64 PhysicalAddress
)
{
    return ( VOID* )( DIRECT_MAP_BASE + PhysicalAddress );
}

ULONG64
BLAPI
SsGetFreePhysicalPage(
    VOID
)
{
    if( SsPfnFreeHead == PFN_LIST_END )
    {
        DBG_INFO( L"No Free memory!" );
        getc( );
        return 0;
    }

    ULONG64 PageBase = PFN_TO_PHYSICAL( SsPfnFreeHead );
    PFN_ENTRY* Entry = &SsPfn[ SsPfnFreeHead ];

    // set next head
    SsPfnFreeHead = Entry->Offset;

    Entry->State = Allocated;
    Entry->Ref = 1;
    Entry->Offset = PFN_LIST_END;

    return PageBase;
}

static PMM_PTE_DESCRIPTOR gPML4;

VOID
BLAPI
SsFreePhysicalPage(
    _In_ ULONG64 Address
)
{
    ULONG64 IndexPFN = PHYSICAL_TO_PFN( Address );

    if( IndexPFN >= SsPfnCount )
    {
        DBG_INFO( L"Pfn too large\n" );
        getc( );
        return;
    }

    PFN_ENTRY* PFN = &SsPfn[ IndexPFN ];
    PFN->State = Free;
    PFN->Ref = 0;
    PFN->Offset = ( UINT32 )SsPfnFreeHead;
    SsPfnFreeHead = IndexPFN;
};

EFI_STATUS
BLAPI
AllocatePage(
    _Out_ ULONG64* Pa
)
{
    ULONG64 FreePage = SsGetFreePhysicalPage( );
    if( !FreePage )
    {
        DBG_INFO( L"No free memory!" );
        getc( );
        return EFI_OUT_OF_RESOURCES;
    }
    EFI_STATUS st =
        gBS->AllocatePages( AllocateAddress, EfiLoaderData, 1, &FreePage );

    if( EFI_ERROR( st ) )
    {
        DBG_INFO( L"Failed to allocate pages!" );
        getc( );
        return st;
    }

    *Pa = FreePage;
    return EFI_SUCCESS;
}

// DirectPagingInit initializes our paging structures for the direct map.
// It allocates one page for the PML4 (level-4 table) and zeros it.
ULONG64
BLAPI
SsPagingInit(
    VOID
)
{
    ULONG64 FreePage;

    AllocatePage( &FreePage );

    if( !FreePage )
    {
        DBG_INFO( L"No free memory!" );
        getc( );
        return ( ULONG64 )NULL;
    }

    gPML4 = ( PMM_PTE_DESCRIPTOR )( PVOID )FreePage;
    ZeroMem((PVOID)gPML4, MM_PAGE_SIZE );

    return FreePage;
}

EFI_STATUS
MmMapPageEx(
    _In_ PMM_PTE_DESCRIPTOR AddressSpace,
    _In_ ULONG64 VirtualAddress,
    _In_ ULONG64 PhysicalAddress,
    _In_ ULONG64 Flags
)
{
    if (!MM_PAGE_IS_ALIGNED(VirtualAddress) || !MM_PAGE_IS_ALIGNED(PhysicalAddress) || !MmIsAddressCanonical(VirtualAddress))
    {
        return EFI_INVALID_PARAMETER;
    }

    if (AddressSpace == NULL)
    {
        return EFI_NOT_READY;
    }

    EFI_STATUS Status = EFI_SUCCESS;

    ULONG64 TablePhysical = 0;

    MM_VIRTUAL_ADDRESS Virtual = { 0 };
    Virtual.Value = VirtualAddress;

    PMM_PTE_DESCRIPTOR Pml4 = (PMM_PTE_DESCRIPTOR)(AddressSpace);
    PMM_PTE_DESCRIPTOR Pdpt = NULL;
    PMM_PTE_DESCRIPTOR Pd = NULL;
    PMM_PTE_DESCRIPTOR Pt = NULL;

    if (!Pml4[Virtual.Pml4Index].Present)
    {
        Status = AllocatePage(&TablePhysical);
        if (EFI_ERROR(Status))
        {
            return Status;
        }

        ZeroMem((PVOID)(ULONG64)TablePhysical, MM_PAGE_SIZE);

        Pml4[Virtual.Pml4Index].Present = TRUE;
        Pml4[Virtual.Pml4Index].Writable = TRUE;
        Pml4[Virtual.Pml4Index].PageFrameNumber = TablePhysical >> MM_PAGE_SHIFT;
    }

    Pdpt = (PMM_PTE_DESCRIPTOR)(ULONG64)(Pml4[Virtual.Pml4Index].PageFrameNumber << MM_PAGE_SHIFT);

    if (!Pdpt[Virtual.PdptIndex].Present)
    {
        Status = AllocatePage(&TablePhysical);
        if (EFI_ERROR(Status))
        {
            return Status;
        }

        ZeroMem((PVOID)(ULONG64)TablePhysical, MM_PAGE_SIZE);

        Pdpt[Virtual.PdptIndex].Present = TRUE;
        Pdpt[Virtual.PdptIndex].Writable = TRUE;
        Pdpt[Virtual.PdptIndex].PageFrameNumber = TablePhysical >> MM_PAGE_SHIFT;
    }

    if (Pdpt[Virtual.PdptIndex].Huge.HugePage)
    {
        return EFI_ABORTED;
    }

    Pd = (PMM_PTE_DESCRIPTOR)(ULONG64)(Pdpt[Virtual.PdptIndex].PageFrameNumber << MM_PAGE_SHIFT);

    if (!Pd[Virtual.PdIndex].Present)
    {
        Status = AllocatePage(&TablePhysical);
        if (EFI_ERROR(Status))
        {
            return Status;
        }

        ZeroMem((PVOID)(ULONG64)TablePhysical, MM_PAGE_SIZE);

        Pd[Virtual.PdIndex].Present = TRUE;
        Pd[Virtual.PdIndex].Writable = TRUE;
        Pd[Virtual.PdIndex].PageFrameNumber = TablePhysical >> MM_PAGE_SHIFT;
    }

    if (Pd[Virtual.PdIndex].Large.LargePage)
    {
        return EFI_ABORTED;
    }

    Pt = (PMM_PTE_DESCRIPTOR)(ULONG64)(Pd[Virtual.PdIndex].PageFrameNumber << MM_PAGE_SHIFT);

    if (Pt[Virtual.PtIndex].Present)
    {
        return EFI_ALREADY_STARTED;
    }

    Pt[Virtual.PtIndex].Value = Flags;
    Pt[Virtual.PtIndex].Present = TRUE;
    Pt[Virtual.PtIndex].PageFrameNumber = PhysicalAddress >> MM_PAGE_SHIFT;

    return Status;
}

// MapPage maps a single 4KB page so that virtual address 'vaddr'
// maps to physical address 'paddr' with the specified 'flags' (for the PTE).
// This function walks the 4-level page table hierarchy, allocating lower-level
// tables on demand. It uses our global gPML4 (which is assumed to be already
// initialized).
EFI_STATUS
MmMapPage(
    _In_ ULONG64 VirtualAddress,
    _In_ ULONG64 PhysicalAddress,
    _In_ ULONG64 Flags
)
{
    return MmMapPageEx((PMM_PTE_DESCRIPTOR)gPML4, VirtualAddress, PhysicalAddress, Flags);
}

EFI_STATUS
MmMapLargePageEx(
    _In_ PMM_PTE_DESCRIPTOR AddressSpace,
    _In_ ULONG64 VirtualAddress,
    _In_ ULONG64 PhysicalAddress,
    _In_ ULONG64 Flags
)
{
    if (!MM_PAGE_LARGE_IS_ALIGNED(VirtualAddress) ||
        !MM_PAGE_LARGE_IS_ALIGNED(PhysicalAddress) ||
        !MmIsAddressCanonical(VirtualAddress)
        )
    {
        return EFI_INVALID_PARAMETER;
    }

    if (AddressSpace == NULL)
    {
        return EFI_NOT_READY;
    }

    EFI_STATUS Status = EFI_SUCCESS;

    ULONG64 TablePhysical = 0;

    MM_VIRTUAL_ADDRESS Virtual = { 0 };
    Virtual.Value = VirtualAddress;

    PMM_PTE_DESCRIPTOR Pml4 = (PMM_PTE_DESCRIPTOR)(AddressSpace);
    PMM_PTE_DESCRIPTOR Pdpt = NULL;
    PMM_PTE_DESCRIPTOR Pd = NULL;

    if (!Pml4[Virtual.Pml4Index].Present)
    {
        Status = AllocatePage(&TablePhysical);
        if (EFI_ERROR(Status))
        {
            return Status;
        }

        ZeroMem((PVOID)(ULONG64)TablePhysical, MM_PAGE_SIZE);

        Pml4[Virtual.Pml4Index].Present = TRUE;
        Pml4[Virtual.Pml4Index].Writable = TRUE;
        Pml4[Virtual.Pml4Index].PageFrameNumber = TablePhysical >> MM_PAGE_SHIFT;
    }

    Pdpt = (PMM_PTE_DESCRIPTOR)(ULONG64)(Pml4[Virtual.Pml4Index].PageFrameNumber << MM_PAGE_SHIFT);

    if (!Pdpt[Virtual.PdptIndex].Present)
    {
        Status = AllocatePage(&TablePhysical);
        if (EFI_ERROR(Status))
        {
            return Status;
        }

        ZeroMem((PVOID)(ULONG64)TablePhysical, MM_PAGE_SIZE);

        Pdpt[Virtual.PdptIndex].Present = TRUE;
        Pdpt[Virtual.PdptIndex].Writable = TRUE;
        Pdpt[Virtual.PdptIndex].PageFrameNumber = TablePhysical >> MM_PAGE_SHIFT;
    }

    if (Pdpt[Virtual.PdptIndex].Huge.HugePage)
    {
        return EFI_ABORTED;
    }

    Pd = (PMM_PTE_DESCRIPTOR)(ULONG64)(Pdpt[Virtual.PdptIndex].PageFrameNumber << MM_PAGE_SHIFT);

    if (Pd[Virtual.PdIndex].Present)
    {
        return EFI_ALREADY_STARTED;
    }

    Pd[Virtual.PdIndex].Value = Flags | MM_PAGE_FLAG_LARGE_PAGE;
    Pd[Virtual.PdIndex].Present = TRUE;
    Pd[Virtual.PdIndex].Large.PageFrameNumber = PhysicalAddress >> MM_PAGE_LARGE_SHIFT;

    return Status;
}

// MapLargePage maps a single 2MiB page
EFI_STATUS
MmMapLargePage(
    _In_ ULONG64 VirtualAddress,
    _In_ ULONG64 PhysicalAddress,
    _In_ ULONG64 Flags
)
{
    return MmMapLargePageEx((PMM_PTE_DESCRIPTOR)gPML4, VirtualAddress, PhysicalAddress, Flags);
}


EFI_STATUS
MmMapHugePageEx(
    _In_ PMM_PTE_DESCRIPTOR AddressSpace,
    _In_ ULONG64 VirtualAddress,
    _In_ ULONG64 PhysicalAddress,
    _In_ ULONG64 Flags
)
{
    if (!MM_PAGE_HUGE_IS_ALIGNED(VirtualAddress) || !MM_PAGE_HUGE_IS_ALIGNED(PhysicalAddress) || !MmIsAddressCanonical(VirtualAddress))
    {
        return EFI_INVALID_PARAMETER;
    }

    if (AddressSpace == NULL)
    {
        return EFI_NOT_READY;
    }

    EFI_STATUS Status = EFI_SUCCESS;

    ULONG64 TablePhysical = 0;

    MM_VIRTUAL_ADDRESS Virtual = { 0 };
    Virtual.Value = VirtualAddress;

    PMM_PTE_DESCRIPTOR Pml4 = (PMM_PTE_DESCRIPTOR)(AddressSpace);
    PMM_PTE_DESCRIPTOR Pdpt = NULL;

    if (!Pml4[Virtual.Pml4Index].Present)
    {
        Status = AllocatePage(&TablePhysical);
        if (EFI_ERROR(Status))
        {
            return Status;
        }

        ZeroMem((PVOID)(ULONG64)TablePhysical, MM_PAGE_SIZE);

        Pml4[Virtual.Pml4Index].Present = TRUE;
        Pml4[Virtual.Pml4Index].Writable = TRUE;
        Pml4[Virtual.Pml4Index].PageFrameNumber = TablePhysical >> MM_PAGE_SHIFT;
    }

    Pdpt = (PMM_PTE_DESCRIPTOR)(ULONG64)(Pml4[Virtual.Pml4Index].PageFrameNumber << MM_PAGE_SHIFT);

    if (Pdpt[Virtual.PdptIndex].Present)
    {
        return EFI_ABORTED;
    }

    Pdpt[Virtual.PdptIndex].Value = Flags | MM_PAGE_FLAG_LARGE_PAGE;
    Pdpt[Virtual.PdptIndex].Present = TRUE;
    Pdpt[Virtual.PdptIndex].Huge.PageFrameNumber = PhysicalAddress >> MM_PAGE_HUGE_SHIFT;

    return Status;
}


EFI_STATUS
MmMapHugePage(
    _In_ ULONG64 VirtualAddress,
    _In_ ULONG64 PhysicalAddress,
    _In_ ULONG64 Flags
)
{
    return MmMapHugePageEx((PMM_PTE_DESCRIPTOR)gPML4, VirtualAddress, PhysicalAddress, Flags);
}

/*
* Checks if a mapping exists for the given virtual address in the specified address space.
* 
* @param AddressSpace The page table to check (e.g. PML4), must be given virtual PML4 pointer.
* @param VirtualAddress The virtual address to check for a mapping.
* 
* @return TRUE if a mapping exists, FALSE otherwise.
*/
BOOLEAN
MmMappingExists(
   _In_ ULONG64 AddressSpace,
   _In_ ULONG64 VirtualAddress
)
{
    PMM_PTE_DESCRIPTOR Pml4    = (PMM_PTE_DESCRIPTOR)AddressSpace;
    MM_VIRTUAL_ADDRESS Virtual = { .Value = VirtualAddress };
    MM_PTE_DESCRIPTOR  PageTableEntry;

    PageTableEntry = Pml4[Virtual.Pml4Index];
    if ( !PageTableEntry.Present )
    {
        return FALSE;
    }

    PMM_PTE_DESCRIPTOR Pdpt = (PMM_PTE_DESCRIPTOR)(PageTableEntry.PageFrameNumber << MM_PAGE_SHIFT);
    PageTableEntry = Pdpt[Virtual.PdptIndex];

    if ( !PageTableEntry.Present )
    {
        return FALSE;
    }

    if (PageTableEntry.Huge.HugePage)
    {
        return TRUE;   // 1GiB mapping
    }

    PMM_PTE_DESCRIPTOR Pd = (PMM_PTE_DESCRIPTOR)(ULONG64)(PageTableEntry.PageFrameNumber << MM_PAGE_SHIFT);
    
    PageTableEntry = Pd[Virtual.PdIndex];
    if (!PageTableEntry.Present)
    {
        return FALSE;
    }

    if (PageTableEntry.Large.LargePage)
    {
        return TRUE;   // 2MiB mapping
    }

    PMM_PTE_DESCRIPTOR Pt = (PMM_PTE_DESCRIPTOR)(PageTableEntry.PageFrameNumber << MM_PAGE_SHIFT);

    PageTableEntry = Pt[Virtual.PtIndex];
    if (!PageTableEntry.Present)
    {
        return FALSE;
    }

    return TRUE;       // 4KiB mapping
};

/*
* A greedy mapping algorithm, maps largest possible pages first, then smaller pages for any remaining range.
* 
* @param AddressSpace The page table to modify (e.g. PML4).
* @param VirtualAddress The starting virtual address to map.
* @param PhysicalAddress The starting physical address to map to.
* @param Size The size of the range to map, in bytes. Must be > 0.
* @param Flags The flags to set on the page table entries (e.g. present, rw, etc).
* 
* @return EFI_SUCCESS if the mapping was successful, or an appropriate error code on failure.
*/
EFI_STATUS
MmMapRangeEx(
    _In_ PMM_PTE_DESCRIPTOR AddressSpace,
    _In_ ULONG64 VirtualAddress,
    _In_ ULONG64 PhysicalAddress,
    _In_ ULONG64 Size,
    _In_ ULONG64 Flags
)
{ // this code has a major issue. When an error is encounted it immediatly returns. realistically this error should be handled properly, 
  // and mapped pages/tables needed to be invalidated. 
    if (!Size)
    {
        return EFI_INVALID_PARAMETER;
    }

    ULONG64 CurrentVirtualAddress  = VirtualAddress;
    ULONG64 CurrentPhysicalAddress = PhysicalAddress;
    ULONG64 VirtualEnd             = VirtualAddress + Size;

    if (VirtualEnd < VirtualAddress)
    {
        return EFI_INVALID_PARAMETER;
    }

    while (CurrentVirtualAddress < VirtualEnd)
    {
        ULONG64 Remaining = VirtualEnd - CurrentVirtualAddress;
        if ( Remaining >= MM_PAGE_HUGE_SIZE          && 
             MM_PAGE_HUGE_IS_ALIGNED(CurrentVirtualAddress) &&
             MM_PAGE_HUGE_IS_ALIGNED(CurrentPhysicalAddress)
           )
        {
            CHKERR(MmMapHugePageEx(AddressSpace, CurrentVirtualAddress, CurrentPhysicalAddress, Flags));
            CurrentVirtualAddress  += MM_PAGE_HUGE_SIZE;
            CurrentPhysicalAddress += MM_PAGE_HUGE_SIZE;
            continue;
        }
        else if ( Remaining >= MM_PAGE_LARGE_SIZE          && 
                  MM_PAGE_LARGE_IS_ALIGNED(CurrentVirtualAddress) &&
                  MM_PAGE_LARGE_IS_ALIGNED(CurrentPhysicalAddress)
                )
        {
            CHKERR(MmMapLargePageEx(AddressSpace, CurrentVirtualAddress, CurrentPhysicalAddress, Flags));
            CurrentVirtualAddress  += MM_PAGE_LARGE_SIZE;
            CurrentPhysicalAddress += MM_PAGE_LARGE_SIZE;
            continue;
        }
        else
        {
            CHKERR(MmMapPageEx(AddressSpace, CurrentVirtualAddress, CurrentPhysicalAddress, Flags));
            CurrentVirtualAddress += MM_PAGE_SIZE;
            CurrentPhysicalAddress += MM_PAGE_SIZE;
        }
    }

    return EFI_SUCCESS;
}

/*
* A wrapper around MmMapRangeEx that uses our global gPML4 as the page table base, for convenience. 
* 
* @param VirtualAddress The starting virtual address to map.
* @param PhysicalAddress The starting physical address to map to.
* @param Size The size of the range to map, in bytes. Must be > 0.
* @param Flags The flags to set on the page table entries (e.g. present, rw, etc).
* 
* @return EFI_SUCCESS if the mapping was successful, or an appropriate error code on failure.
*/
EFI_STATUS
MmMapRange(
   _In_ ULONG64 VirtualAddress,
   _In_ ULONG64 PhysicalAddress,
   _In_ ULONG64 Size,
   _In_ ULONG64 Flags
)
{
    return MmMapRangeEx(gPML4, VirtualAddress, PhysicalAddress, Size, Flags);
}

EFI_STATUS
MmMapHugeRangeEx(
    _In_ PMM_PTE_DESCRIPTOR AddressSpace,
    _In_ ULONG64 Virtual,
    _In_ ULONG64 Physical,
    _In_ ULONG64 Pages,
    _In_ ULONG64 Flags
)
{
    if( !Pages )
    {
        return EFI_INVALID_PARAMETER;
    }

    ULONG64 CurrentVirtual  = Virtual;
    ULONG64 CurrentPhysical = Physical;

    while ( Pages > 0 )
    {
        CHKERR( MmMapHugePageEx(AddressSpace, CurrentVirtual, CurrentPhysical, Flags) );
        CurrentVirtual  += MM_PAGE_HUGE_SIZE;
        CurrentPhysical += MM_PAGE_HUGE_SIZE;
        Pages--;
    }

    return EFI_SUCCESS;
}

EFI_STATUS
MmMapLargeRangeEx(
    _In_ PMM_PTE_DESCRIPTOR AddressSpace,
    _In_ ULONG64 Virtual,
    _In_ ULONG64 Physical,
    _In_ ULONG64 Pages,
    _In_ ULONG64 Flags
)
{
    if ( !Pages )
    {
        return EFI_INVALID_PARAMETER;
    }

    ULONG64 CurrentVirtual = Virtual;
    ULONG64 CurrentPhysical = Physical;

    while (Pages > 0)
    {
        CHKERR(MmMapLargePageEx(AddressSpace, CurrentVirtual, CurrentPhysical, Flags));
        CurrentVirtual  += MM_PAGE_LARGE_SIZE;
        CurrentPhysical += MM_PAGE_LARGE_SIZE;
        Pages--;
    }

    return EFI_SUCCESS;
}

EFI_STATUS
MmMapSmallRangeEx(
    _In_ PMM_PTE_DESCRIPTOR AddressSpace,
    _In_ ULONG64 Virtual,
    _In_ ULONG64 Physical,
    _In_ ULONG64 Pages,
    _In_ ULONG64 Flags
)
{
    if ( !Pages )
    {
        return EFI_INVALID_PARAMETER;
    }

    ULONG64 CurrentVirtual  = Virtual;
    ULONG64 CurrentPhysical = Physical;

    while (Pages > 0)
    {
        CHKERR(MmMapPageEx(AddressSpace, CurrentVirtual, CurrentPhysical, Flags));
        CurrentVirtual  += MM_PAGE_SIZE;
        CurrentPhysical += MM_PAGE_SIZE;
        Pages--;
    }

    return EFI_SUCCESS;
}

// DirectMapRange maps all physical memory from physStart to physEnd
// into the direct mapped region so that VA = DIRECT_MAP_BASE + PA.
// The caller must ensure that physStart and physEnd are page-aligned.
EFI_STATUS
DirectMapRange(
    ULONG64 PhysicalStart,
    ULONG64 PhysicalEnd
)
{
    return MmMapRange(
        /*DIRECT_MAP_BASE +*/ PhysicalStart,
        PhysicalStart,
        PhysicalEnd - PhysicalStart,
        MmReadWriteExecuteProtection
    );
}

EFI_STATUS
BLAPI
UnmapPage(
    ULONG64 vaddr
)
{
    return EFI_SUCCESS;
}

EFI_STATUS
BLAPI
MapKernelLargePage(
    ULONG64 VirtualImageBase,
    ULONG64 NewVirtualImageBase
)
{
    PEFI_IMAGE_NT_HEADERS NtHeaders = EFI_IMAGE_NTHEADERS(VirtualImageBase);
    if (!NtHeaders)
    {
        return EFI_INVALID_PARAMETER;
    }

    // Total image span (headers + all sections), rounded up to 2 MiB
    ULONG64 SizeOfImage = NtHeaders->OptionalHeader.SizeOfImage;
    ULONG64 SizeAligned = MM_PAGE_LARGE_ALIGN_DOWN(SizeOfImage + MM_PAGE_LARGE_SIZE - 1);

    if (!MM_PAGE_LARGE_IS_ALIGNED(VirtualImageBase) ||
        !MM_PAGE_LARGE_IS_ALIGNED(NewVirtualImageBase)
       )
    {
        return EFI_INVALID_PARAMETER;
    }
    
    ULONG64 Flags = MM_PAGE_FLAG_PRESENT | MmReadWriteExecuteProtection;

    for ( ULONG64 Offset = 0; Offset < SizeAligned; Offset += MM_PAGE_LARGE_SIZE )
    {
        EFI_STATUS St = MmMapLargePage(
            NewVirtualImageBase + Offset,
            VirtualImageBase + Offset,
            Flags
        );

        if (EFI_ERROR(St))
        {
            DBG_ERROR(St, L"Kernel map 2MiB failed at offset %p\n", Offset);
            return St;
        }
    }

    return EFI_SUCCESS;
}

EFI_STATUS
BLAPI
MapKernelPage(
    ULONG64 VirtualImageBase,
    ULONG64 NewVirtualImageBase
)
{
    PEFI_IMAGE_NT_HEADERS NtHeaders = EFI_IMAGE_NTHEADERS( VirtualImageBase );

    ULONG64 SizeOfHeaders = MM_PAGE_ALIGN_UP( NtHeaders->OptionalHeader.SizeOfHeaders );
    for( ULONG64 Offset = 0; Offset < SizeOfHeaders; Offset += MM_PAGE_SIZE )
    {
        EFI_STATUS St = MmMapPage( NewVirtualImageBase + Offset, VirtualImageBase + Offset, MM_PAGE_FLAG_PRESENT | MM_PAGE_FLAG_READ_WRITE );
        if( EFI_ERROR( St ) )
        {
            DBG_ERROR(St, L"Failed to re-map kernel headers\n");
            return St;
        }
    }

    EFI_IMAGE_SECTION_HEADER* CurrentSection = EFI_IMAGE_FIRST_SECTION( NtHeaders );
    for( UINT32 Index = 0; Index < NtHeaders->FileHeader.NumberOfSections; ++Index )
    {
        ULONG64 SectionNewVirtualAddress = NewVirtualImageBase + CurrentSection[ Index ].VirtualAddress;
        ULONG64 SectionPhysicalAddress = VirtualImageBase + CurrentSection[ Index ].VirtualAddress;

        ULONG64 PageFlags = MM_PAGE_FLAG_PRESENT;

        // only care about 
        if( ( CurrentSection[ Index ].Characteristics & EFI_IMAGE_SCN_MEM_WRITE ) )
        {
            PageFlags |= MM_PAGE_FLAG_READ_WRITE;
        }

        for (ULONG64 Offset = 0; Offset < MM_PAGE_ALIGN_DOWN(CurrentSection[Index].Misc.VirtualSize + MM_PAGE_SIZE - 1); Offset += MM_PAGE_SIZE )
        {
            EFI_STATUS St = MmMapPage( SectionNewVirtualAddress + Offset, SectionPhysicalAddress + Offset, PageFlags );
            if (EFI_ERROR(St))
            {
                DBG_ERROR(St, L"Failed to re-map kernel section\n");
                return St;
            }
        }
    }
    return EFI_SUCCESS;
}


// 
// MMU STRESSING
//
STATIC ULONG64 gMmuStressArenaPhysicalBase = 0;
STATIC ULONG64 gMmuStressArenaLength = 0;
STATIC ULONG64 gMmuStressDescriptorCapacity = 0;

#define MMU_TEST_VIRTUAL_BASE 0xFFFF900000000000ULL

#define MMU_TEST_PAGE_FLAGS MmReadWriteProtection

STATIC CONST MMU_STRESS_SPACE_DESCRIPTOR Set0Small = { MMU_TEST_VIRTUAL_BASE, MM_PAGE_SMALL_SIZE, MmuStressModePages, 30, MMU_TEST_PAGE_FLAGS };
STATIC CONST MMU_STRESS_SPACE_DESCRIPTOR Set0Large = { MMU_TEST_VIRTUAL_BASE, MM_PAGE_LARGE_SIZE, MmuStressModePages, 30, MMU_TEST_PAGE_FLAGS };

STATIC CONST MMU_STRESS_SPACE_DESCRIPTOR Set1Small = { MMU_TEST_VIRTUAL_BASE, MM_PAGE_SMALL_SIZE, MmuStressModePages, 250, MMU_TEST_PAGE_FLAGS };
STATIC CONST MMU_STRESS_SPACE_DESCRIPTOR Set1Large = { MMU_TEST_VIRTUAL_BASE, MM_PAGE_LARGE_SIZE, MmuStressModePages, 250, MMU_TEST_PAGE_FLAGS };

STATIC CONST MMU_STRESS_SPACE_DESCRIPTOR Set2Small = { MMU_TEST_VIRTUAL_BASE, MM_PAGE_SMALL_SIZE, MmuStressModePages, 1000, MMU_TEST_PAGE_FLAGS };
STATIC CONST MMU_STRESS_SPACE_DESCRIPTOR Set2Large = { MMU_TEST_VIRTUAL_BASE, MM_PAGE_LARGE_SIZE, MmuStressModePages, 1000, MMU_TEST_PAGE_FLAGS };

STATIC CONST MMU_STRESS_SPACE_DESCRIPTOR Set3Small = { MMU_TEST_VIRTUAL_BASE, MM_PAGE_SMALL_SIZE, MmuStressModePages, 2500, MMU_TEST_PAGE_FLAGS };
STATIC CONST MMU_STRESS_SPACE_DESCRIPTOR Set3Large = { MMU_TEST_VIRTUAL_BASE, MM_PAGE_LARGE_SIZE, MmuStressModePages, 2500, MMU_TEST_PAGE_FLAGS };

STATIC CONST MMU_STRESS_SPACE_DESCRIPTOR Set4Small = { MMU_TEST_VIRTUAL_BASE, MM_PAGE_SMALL_SIZE, MmuStressModeBytes, MM_MB_TO_BYTES(200), MMU_TEST_PAGE_FLAGS };
STATIC CONST MMU_STRESS_SPACE_DESCRIPTOR Set4Large = { MMU_TEST_VIRTUAL_BASE, MM_PAGE_LARGE_SIZE, MmuStressModeBytes, MM_MB_TO_BYTES(200), MMU_TEST_PAGE_FLAGS };

STATIC CONST MMU_STRESS_SPACE_DESCRIPTOR Set5Small = { MMU_TEST_VIRTUAL_BASE, MM_PAGE_SMALL_SIZE, MmuStressModeBytes, MM_GB_TO_BYTES(1), MMU_TEST_PAGE_FLAGS };
STATIC CONST MMU_STRESS_SPACE_DESCRIPTOR Set5Large = { MMU_TEST_VIRTUAL_BASE, MM_PAGE_LARGE_SIZE, MmuStressModeBytes, MM_GB_TO_BYTES(1), MMU_TEST_PAGE_FLAGS };

STATIC CONST MMU_STRESS_SPACE_SETS gMmuTestingDescriptors[] =
{
    {
        0,
        &Set0Small,
        &Set0Large,
        NULL
    },
    {
        1,
        &Set1Small,
        &Set1Large,
        NULL
    },
    {
        2,
        &Set2Small,
        &Set2Large,
        NULL
    },
    {
        3,
        &Set3Small,
        &Set3Large,
        NULL
    },
    {
        4,
        &Set4Small,
        &Set4Large,
        NULL
    },
    {
        5,
        &Set5Small,
        &Set5Large,
        NULL
    }
};

BOOLEAN
BlIsRangeFree(
    _In_ ULONG64 StartPfn,
    _In_ ULONG64 PageCount
)
{
    ULONG64 Pfn;

    if (StartPfn + PageCount > SsPfnCount)
    {
        return FALSE;
    }

    for (Pfn = StartPfn; Pfn < StartPfn + PageCount; ++Pfn)
    {
        if (SsPfn[Pfn].State != Free)
        {
            return FALSE;
        }
    }

    return TRUE;
}

VOID
BlMarkAllocatedRange(
    _In_ ULONG64 StartPfn,
    _In_ ULONG64 PageCount
)
{
    ULONG64 Pfn;

    for (Pfn = StartPfn; Pfn < StartPfn + PageCount; ++Pfn)
    {
        SsPfn[Pfn].State = Allocated;
        SsPfn[Pfn].Ref = 1;
        SsPfn[Pfn].Offset = PFN_LIST_END;
    }
}

EFI_STATUS
BlAllocateAlignedPages(
    _Out_ PULONG64 PhysicalBase,
    _In_ ULONG64 PageCount,
    _In_ ULONG64 Alignment
)
{
    ULONG64 StartPfn;
    EFI_STATUS Status;

    if (!PhysicalBase || !PageCount || !Alignment)
    {
        return EFI_INVALID_PARAMETER;
    }

    if (Alignment & (Alignment - 1))
    {
        return EFI_INVALID_PARAMETER;
    }

    if (Alignment < MM_PAGE_SIZE)
    {
        return EFI_INVALID_PARAMETER;
    }

    //
    // Never hand out PFN 0 / physical 0.
    //
    for (StartPfn = 1; StartPfn + PageCount <= SsPfnCount; ++StartPfn)
    {
        ULONG64 Physical = PFN_TO_PHYSICAL(StartPfn);
        EFI_PHYSICAL_ADDRESS Address;

        if (Physical == 0)
        {
            continue;
        }

        if ((Physical & (Alignment - 1)) != 0)
        {
            continue;
        }

        if (!BlIsRangeFree(StartPfn, PageCount))
        {
            continue;
        }

        Address = (EFI_PHYSICAL_ADDRESS)Physical;
        Status = gBS->AllocatePages(
            AllocateAddress,
            EfiLoaderData,
            (UINTN)PageCount,
            &Address
        );

        if (EFI_ERROR(Status))
        {
            continue;
        }

        BlMarkAllocatedRange(StartPfn, PageCount);
        *PhysicalBase = (ULONG64)Address;
        return EFI_SUCCESS;
    }

    return EFI_OUT_OF_RESOURCES;
}

EFI_STATUS
BlAllocateArenaBytes(
    _Out_ PULONG64 PhysicalBase,
    _In_ ULONG64 LengthBytes,
    _In_ ULONG64 Alignment
)
{
    if (!PhysicalBase || !LengthBytes || !Alignment)
    {
        return EFI_INVALID_PARAMETER;
    }

    return BlAllocateAlignedPages(
        PhysicalBase,
        EFI_SIZE_TO_PAGES(LengthBytes),
        Alignment
    );
}

EFI_STATUS
BlMmuPlanLengthBytes(
    _In_ PMMU_STRESS_SPACE_DESCRIPTOR Descriptor,
    _Out_ PULONG64 LengthBytes
)
{
    ULONG64 Length;

    if (!Descriptor || !LengthBytes)
    {
        return EFI_INVALID_PARAMETER;
    }

    if (!Descriptor->SizeValue)
    {
        return EFI_INVALID_PARAMETER;
    }

    if (Descriptor->PageSize != MM_PAGE_SIZE &&
        Descriptor->PageSize != MM_PAGE_LARGE_SIZE &&
        Descriptor->PageSize != MM_PAGE_HUGE_SIZE)
    {
        return EFI_INVALID_PARAMETER;
    }

    if (Descriptor->SizingMode == MmuStressModePages)
    {
        if (Descriptor->SizeValue > (MAXULONG64 / Descriptor->PageSize))
        {
            return EFI_INVALID_PARAMETER;
        }

        Length = Descriptor->SizeValue * Descriptor->PageSize;
    }
    else if (Descriptor->SizingMode == MmuStressModeBytes)
    {
        Length = Descriptor->SizeValue;
    }
    else
    {
        return EFI_INVALID_PARAMETER;
    }

    if (Descriptor->PageSize == MM_PAGE_SIZE)
    {
        Length = MM_PAGE_ALIGN_UP(Length);
    }
    else if (Descriptor->PageSize == MM_PAGE_LARGE_SIZE)
    {
        Length = MM_PAGE_LARGE_ALIGN_UP(Length);
    }
    else
    {
        Length = MM_PAGE_HUGE_ALIGN_UP(Length);
    }

    *LengthBytes = Length;
    return EFI_SUCCESS;
}

ULONG64
BlGetMmuTestingSetCount(
    VOID
)
{
    return sizeof(gMmuTestingDescriptors) / sizeof(gMmuTestingDescriptors[0]);
}

ULONG64
BlCountSpacesInSet(
    _In_ CONST MMU_STRESS_SPACE_SETS* Sets
)
{
    ULONG64 Count = 0;

    if (!Sets)
    {
        return 0;
    }

    if (Sets->SpacePage)
    {
        Count++;
    }

    if (Sets->SpaceLargePage)
    {
        Count++;
    }

    if (Sets->SpaceHugePage)
    {
        Count++;
    }

    return Count;
}

ULONG64
BlPageSizeToAlignment(
    _In_ ULONG64 PageSize
)
{
    if (PageSize == MM_PAGE_HUGE_SIZE)
    {
        return MM_PAGE_HUGE_SIZE;
    }

    if (PageSize == MM_PAGE_LARGE_SIZE)
    {
        return MM_PAGE_LARGE_SIZE;
    }

    return MM_PAGE_SIZE;
}

EFI_STATUS
BlAllocateMetadataBuffer(
    _Out_ PVOID* Buffer,
    _In_ ULONG64 LengthBytes
)
{
    EFI_STATUS Status;
    ULONG64 Physical = 0;
    ULONG64 Pages;

    if (!Buffer || !LengthBytes)
    {
        return EFI_INVALID_PARAMETER;
    }

    Pages = EFI_SIZE_TO_PAGES(LengthBytes);

    Status = BlAllocateAlignedPages(&Physical, Pages, MM_PAGE_SIZE);
    if (EFI_ERROR(Status))
    {
        return Status;
    }

    if (Physical == 0)
    {
        return EFI_NOT_READY;
    }

    //
    // Since you do not have a direct map, explicitly identity-map it.
    //
    Status = MmMapRange(
        Physical,
        Physical,
        Pages * MM_PAGE_SIZE,
        MmReadWriteExecuteProtection
    );
    if (EFI_ERROR(Status))
    {
        return Status;
    }

    ZeroMem((PVOID)(UINTN)Physical, Pages * MM_PAGE_SIZE);
    *Buffer = (PVOID)(UINTN)Physical;

    return EFI_SUCCESS;
}

EFI_STATUS
BlCloneCurrentAddressSpace(
    _Out_ PMM_PTE_DESCRIPTOR* NewAddressSpace,
    _Out_ PULONG64 NewAddressSpacePhysical
)
{
    EFI_STATUS Status;
    ULONG64 NewPml4Physical = 0;

    if (!NewAddressSpace || !NewAddressSpacePhysical)
    {
        return EFI_INVALID_PARAMETER;
    }

    if (!gPML4)
    {
        return EFI_NOT_READY;
    }

    Status = AllocatePage(&NewPml4Physical);
    if (EFI_ERROR(Status))
    {
        return Status;
    }

    CopyMem((PVOID)(UINTN)NewPml4Physical, (CONST VOID*)gPML4, MM_PAGE_SIZE);

    *NewAddressSpace = (PMM_PTE_DESCRIPTOR)(UINTN)NewPml4Physical;
    *NewAddressSpacePhysical = NewPml4Physical;

    return EFI_SUCCESS;
}

STATIC
EFI_STATUS
BlMapStressSpaceWithPageSize(
    _In_ PMM_PTE_DESCRIPTOR AddressSpace,
    _In_ PMMU_STRESS_SPACE_DESCRIPTOR Descriptor,
    _In_ ULONG64 PhysicalBase,
    _In_ ULONG64 LengthBytes
)
{
    ULONG64 PageCount;

    if (!AddressSpace || !Descriptor || !PhysicalBase || !LengthBytes)
    {
        return EFI_INVALID_PARAMETER;
    }

    if (Descriptor->PageSize == MM_PAGE_SIZE)
    {
        PageCount = LengthBytes / MM_PAGE_SIZE;
        return MmMapSmallRangeEx(
            AddressSpace,
            Descriptor->VirtualBase,
            PhysicalBase,
            PageCount,
            Descriptor->PageFlags
        );
    }

    if (Descriptor->PageSize == MM_PAGE_LARGE_SIZE)
    {
        PageCount = LengthBytes / MM_PAGE_LARGE_SIZE;
        return MmMapLargeRangeEx(
            AddressSpace,
            Descriptor->VirtualBase,
            PhysicalBase,
            PageCount,
            Descriptor->PageFlags
        );
    }

    if (Descriptor->PageSize == MM_PAGE_HUGE_SIZE)
    {
        PageCount = LengthBytes / MM_PAGE_HUGE_SIZE;
        return MmMapHugeRangeEx(
            AddressSpace,
            Descriptor->VirtualBase,
            PhysicalBase,
            PageCount,
            Descriptor->PageFlags
        );
    }

    return EFI_INVALID_PARAMETER;
}

EFI_STATUS
BlAppendSingleStressAddressSpace(
    _Inout_ PBOOT_INFO BootInfo,
    _In_ PMMU_STRESS_SPACE_DESCRIPTOR Descriptor,
    _In_ ULONG64 SetIdentifier
)
{
    EFI_STATUS Status;
    ULONG64 LengthBytes = 0;
    ULONG64 Pml4Physical = 0;
    PMM_PTE_DESCRIPTOR NewAddressSpace = NULL;
    PADDRESS_SPACE_DESCRIPTORS Out;

    if (!BootInfo || !Descriptor)
    {
        return EFI_INVALID_PARAMETER;
    }

    if (!BootInfo->MmuStresserDescriptors.AddressSpaces)
    {
        Print(L"BootInfo MmuStresserDescriptors not initialised\n");
        return EFI_NOT_READY;
    }

    if (BootInfo->MmuStresserDescriptors.AddressSpaceCount >= gMmuStressDescriptorCapacity)
    {
        Print(L"BootInfo MmuStresserDescriptors AddressSpaceCount buffer too small\n");
        return EFI_BUFFER_TOO_SMALL;
    }

    Status = BlMmuPlanLengthBytes(Descriptor, &LengthBytes);
    if (EFI_ERROR(Status))
    {
        Print(L"Failed to plan length bytes for set %d\n", SetIdentifier);
        return Status;
    }

    Status = BlCloneCurrentAddressSpace(&NewAddressSpace, &Pml4Physical);
    if (EFI_ERROR(Status))
    {
        Print(L"Failed to clone current address space for set %d\n", SetIdentifier);
        return Status;
    }

    Status = BlMapStressSpaceWithPageSize(
        NewAddressSpace,
        Descriptor,
        gMmuStressArenaPhysicalBase,
        LengthBytes
    );
    if (EFI_ERROR(Status))
    {
        Print(L"Failed to map stress space for set %d\n", SetIdentifier);
        return Status;
    }

    Out = &BootInfo->MmuStresserDescriptors.AddressSpaces[
        BootInfo->MmuStresserDescriptors.AddressSpaceCount
    ];

    ZeroMem(Out, sizeof(*Out));

    Out->AddressSpace.Value = 0;
    Out->AddressSpace.PageFrameNumber = Pml4Physical >> MM_PAGE_SHIFT;

    Out->HeapVirtualBase = Descriptor->VirtualBase;
    Out->HeapPhysicalBase = gMmuStressArenaPhysicalBase;
    Out->HeapLength = LengthBytes;
    Out->HeapPages = LengthBytes / Descriptor->PageSize;
    Out->PageSize = Descriptor->PageSize;
    Out->SetIdentifier = SetIdentifier;

    //
    // Optional later: store per-leaf descriptors if you want a flat array
    // describing each mapping unit. For now, the CR3 + base/length/page size
    // is enough to reconstruct/use the arena.
    //
    Out->HeapPageDescriptors = NULL;

    BootInfo->MmuStresserDescriptors.AddressSpaceCount++;

    return EFI_SUCCESS;
}

EFI_STATUS
BlFindArenaRequirements(
    _In_ ULONG64 SetCount,
    _Out_ PULONG64 MaxLength,
    _Out_ PULONG64 MaxAlignment,
    _Out_ PULONG64 DescriptorCount
)
{
    ULONG64 Index;
    ULONG64 BestLength = 0;
    ULONG64 BestAlignment = MM_PAGE_SIZE;
    ULONG64 TotalDescriptors = 0;

    if (!MaxLength || !MaxAlignment || !DescriptorCount)
    {
        return EFI_INVALID_PARAMETER;
    }

    if (!SetCount || SetCount > BlGetMmuTestingSetCount())
    {
        return EFI_INVALID_PARAMETER;
    }

    for (Index = 0; Index < SetCount; ++Index)
    {
        CONST MMU_STRESS_SPACE_SETS* Sets = &gMmuTestingDescriptors[Index];
        CONST PMMU_STRESS_SPACE_DESCRIPTOR Candidates[3] =
        {
            Sets->SpacePage,
            Sets->SpaceLargePage,
            Sets->SpaceHugePage
        };

        ULONG64 CandidateIndex;

        TotalDescriptors += BlCountSpacesInSet(Sets);

        for (CandidateIndex = 0; CandidateIndex < 3; ++CandidateIndex)
        {
            ULONG64 LengthBytes;
            ULONG64 Alignment;
            EFI_STATUS Status;

            if (!Candidates[CandidateIndex])
            {
                continue;
            }

            Status = BlMmuPlanLengthBytes(Candidates[CandidateIndex], &LengthBytes);
            if (EFI_ERROR(Status))
            {
                return Status;
            }

            Alignment = BlPageSizeToAlignment(Candidates[CandidateIndex]->PageSize);

            if (LengthBytes > BestLength)
            {
                BestLength = LengthBytes;
            }

            if (Alignment > BestAlignment)
            {
                BestAlignment = Alignment;
            }
        }
    }

    if (!BestLength || !TotalDescriptors)
    {
        return EFI_NOT_FOUND;
    }

    if (BestAlignment == MM_PAGE_HUGE_SIZE)
    {
        BestLength = MM_PAGE_HUGE_ALIGN_UP(BestLength);
    }
    else if (BestAlignment == MM_PAGE_LARGE_SIZE)
    {
        BestLength = MM_PAGE_LARGE_ALIGN_UP(BestLength);
    }
    else
    {
        BestLength = MM_PAGE_ALIGN_UP(BestLength);
    }

    *MaxLength = BestLength;
    *MaxAlignment = BestAlignment;
    *DescriptorCount = TotalDescriptors;

    return EFI_SUCCESS;
}

EFI_STATUS
BlBuildMmuStressDescriptor(
    _Inout_ PBOOT_INFO BootInfo,
    _In_ PMMU_STRESS_SPACE_SETS Sets
)
{
    EFI_STATUS Status;

    if (!BootInfo || !Sets)
    {
        return EFI_INVALID_PARAMETER;
    }

    if (!gMmuStressArenaPhysicalBase || !gMmuStressArenaLength)
    {
        return EFI_NOT_READY;
    }

    if (Sets->SpacePage)
    {
        Status = BlAppendSingleStressAddressSpace(
            BootInfo,
            Sets->SpacePage,
            Sets->SetIdentifier
        );
        if (EFI_ERROR(Status))
        {
            Print(L"Failed to append page stress space for set %d\n", Sets->SetIdentifier);
            return Status;
        }
    }

    if (Sets->SpaceLargePage)
    {
        Status = BlAppendSingleStressAddressSpace(
            BootInfo,
            Sets->SpaceLargePage,
            Sets->SetIdentifier
        );
        if (EFI_ERROR(Status))
        {
            Print(L"Failed to append large page stress space for set %d\n", Sets->SetIdentifier);
            return Status;
        }
    }

    if (Sets->SpaceHugePage)
    {
        Status = BlAppendSingleStressAddressSpace(
            BootInfo,
            Sets->SpaceHugePage,
            Sets->SetIdentifier
        );
        if (EFI_ERROR(Status))
        {
            Print(L"Failed to append huge page stress space for set %d\n", Sets->SetIdentifier);
            return Status;
        }
    }

    return EFI_SUCCESS;
};

VOID
BlDumpLargestConventionalRegion(
    _In_ PBOOT_INFO BootInfo
)
{
    ULONG64 LargestBytes = 0;
    ULONG64 LargestStart = 0;
    ULONG64 Offset;

    for (Offset = 0; Offset < BootInfo->MemoryMap.MapSize; Offset += BootInfo->MemoryMap.DescriptorSize)
    {
        PBL_EFI_MEMORY_DESCRIPTOR Desc =
            (PBL_EFI_MEMORY_DESCRIPTOR)((UINT8*)BootInfo->MemoryMap.Descriptor + Offset);

        if (Desc->Type != EfiConventionalMemory)
        {
            continue;
        }

        ULONG64 Bytes = Desc->NumberOfPages << MM_PAGE_SHIFT;
        if (Bytes > LargestBytes)
        {
            LargestBytes = Bytes;
            LargestStart = Desc->PhysicalStart;
        }
    }

    DBG_INFO(L"Largest conventional region: base=%llx size=%llu MiB\n",
        LargestStart,
        LargestBytes >> 20);
}

EFI_STATUS
BlAllocateStresserPhysical(
    _Inout_ PBOOT_INFO BootInfo,
    _In_ ULONG64 PairCount
)
{
    EFI_STATUS Status;
    ULONG64 MaxLength = 0;
    ULONG64 MaxAlignment = 0;
    ULONG64 DescriptorCount = 0;
    ULONG64 DescriptorBytes;
    ULONG64 Index;
    PADDRESS_SPACE_DESCRIPTORS DescriptorArray = NULL;

    if (!BootInfo)
    {
        return EFI_INVALID_PARAMETER;
    }

    if (PairCount == 0 || PairCount > BlGetMmuTestingSetCount())
    {
        return EFI_INVALID_PARAMETER;
    }

    if (BootInfo->MmuStresserDescriptors.AddressSpaces != NULL ||
        BootInfo->MmuStresserDescriptors.AddressSpaceCount != 0)
    {
        return EFI_ALREADY_STARTED;
    }

    Status = BlFindArenaRequirements(
        PairCount,
        &MaxLength,
        &MaxAlignment,
        &DescriptorCount
    );
    if (EFI_ERROR(Status))
    {
        return Status;
    }

    Status = BlAllocateArenaBytes(
        &gMmuStressArenaPhysicalBase,
        MaxLength,
        MaxAlignment
    );
    if (EFI_ERROR(Status))
    {
        return Status;
    }

    gMmuStressArenaLength = MaxLength;
    gMmuStressDescriptorCapacity = DescriptorCount;

    DescriptorBytes = DescriptorCount * sizeof(ADDRESS_SPACE_DESCRIPTORS);

    Status = BlAllocateMetadataBuffer((PVOID*)&DescriptorArray, DescriptorBytes);
    if (EFI_ERROR(Status))
    {
        return Status;
    }

    ZeroMem(DescriptorArray, DescriptorBytes);

    BootInfo->MmuStresserDescriptors.AddressSpaces = DescriptorArray;
    BootInfo->MmuStresserDescriptors.AddressSpaceCount = 0;

    for (Index = 0; Index < PairCount; ++Index)
    {
        Status = BlBuildMmuStressDescriptor(BootInfo, (PMMU_STRESS_SPACE_SETS)&gMmuTestingDescriptors[Index]);
        if (EFI_ERROR(Status))
        {
            Print(L"Index %d\n", Index);
            return Status;
        }
    }

    return EFI_SUCCESS;
}

#endif  // !PAGEMANAGER_H
