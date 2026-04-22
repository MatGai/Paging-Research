#ifndef ARCHX64_PAGING_H
#define ARCHX64_PAGING_H

#include <scouse/shared/typedefs.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4201)
#endif

#define MM_PAGE_SMALL_SIZE	4096
#define MM_PAGE_LARGE_SIZE  ( MM_PAGE_SMALL_SIZE * 512 )
#define MM_PAGE_HUGE_SIZE	( MM_PAGE_LARGE_SIZE * 512 )

#define MM_PAGE_SMALL_MASK  ( MM_PAGE_SMALL_SIZE - 1 )
#define MM_PAGE_LARGE_MASK  ( MM_PAGE_LARGE_SIZE - 1 )
#define MM_PAGE_HUGE_MASK   ( MM_PAGE_HUGE_SIZE  - 1 )

#define MM_PAGE_SMALL_SHIFT 12
#define MM_PAGE_LARGE_SHIFT 21
#define MM_PAGE_HUGE_SHIFT	30

#define MM_PAGE_SIZE  MM_PAGE_SMALL_SIZE
#define MM_PAGE_MASK  MM_PAGE_SMALL_MASK
#define MM_PAGE_SHIFT MM_PAGE_SMALL_SHIFT

#define MM_SIZE_TO_PAGE_SMALL( Size ) ( ( ( Size ) + MM_PAGE_SMALL_MASK ) >> MM_PAGE_SMALL_SHIFT )
#define MM_SIZE_TO_PAGE_LARGE( Size ) ( ( ( Size ) + MM_PAGE_LARGE_MASK ) >> MM_PAGE_LARGE_SHIFT )
#define MM_SIZE_TO_PAGE_HUGE(  Size ) ( ( ( Size ) + MM_PAGE_HUGE_MASK )  >> MM_PAGE_HUGE_SHIFT  )

#define MM_PAGES_SMALL_TO_SIZE( Pages ) ( ( Pages ) << MM_PAGE_SMALL_SHIFT )
#define MM_PAGES_LARGE_TO_SIZE( Pages ) ( ( Pages ) << MM_PAGE_LARGE_SHIFT )
#define MM_PAGES_HUGE_TO_SIZE(  Pages ) ( ( Pages ) << MM_PAGE_HUGE_SHIFT  ) 

#define MM_SIZE_TO_PAGE MM_SIZE_TO_PAGE_SMALL
#define MM_PAGES_TO_SIZE MM_PAGES_SMALL_TO_SIZE

#define MM_PAGE_SMALL_ALIGN_DOWN( Address ) ( ( ( Address ) >> MM_PAGE_SMALL_SHIFT ) << MM_PAGE_SMALL_SHIFT )
#define MM_PAGE_SMALL_ALIGN_UP( Address ) ( ( ( ( Address ) + MM_PAGE_SMALL_MASK ) >> MM_PAGE_SMALL_SHIFT ) << MM_PAGE_SMALL_SHIFT )

#define MM_PAGE_LARGE_ALIGN_DOWN( Address ) ( ( ( Address ) >> MM_PAGE_LARGE_SHIFT ) << MM_PAGE_LARGE_SHIFT )
#define MM_PAGE_LARGE_ALIGN_UP( Address ) ( ( ( ( Address ) + MM_PAGE_LARGE_MASK ) >> MM_PAGE_LARGE_SHIFT ) << MM_PAGE_LARGE_SHIFT )

#define MM_PAGE_HUGE_ALIGN_DOWN( Address ) ( ( ( Address ) >> MM_PAGE_HUGE_SHIFT ) << MM_PAGE_HUGE_SHIFT )
#define MM_PAGE_HUGE_ALIGN_UP( Address ) ( ( ( ( Address ) + MM_PAGE_HUGE_MASK ) >> MM_PAGE_HUGE_SHIFT ) << MM_PAGE_HUGE_SHIFT )

#define MM_PAGE_ALIGN_DOWN MM_PAGE_SMALL_ALIGN_DOWN
#define MM_PAGE_ALIGN_UP   MM_PAGE_SMALL_ALIGN_UP

#define MM_PAGE_SMALL_IS_ALIGNED( Address ) ( MM_PAGE_SMALL_ALIGN_DOWN( Address ) == Address )
#define MM_PAGE_LARGE_IS_ALIGNED( Address ) ( MM_PAGE_LARGE_ALIGN_DOWN( Address ) == Address )
#define MM_PAGE_HUGE_IS_ALIGNED(  Address ) ( MM_PAGE_HUGE_ALIGN_DOWN(  Address ) == Address )

#define MM_PAGE_IS_ALIGNED MM_PAGE_SMALL_IS_ALIGNED

#define MM_KERNEL_PML4_BASE 256

#define MM_CANONICAL_LOW_MASK  0x0000FFFFFFFFFFFFull
#define MM_CANONICAL_HIGH_MASK 0xFFFF000000000000ull
#define MM_CANONICAL_SIGN_BIT  (1ull << 47)

#define MM_PAGE_ENTRY_COUNT ( MM_PAGE_SIZE / sizeof( PVOID ) )

#define MM_PAGE_FLAG_PRESENT    ( 1ull << 0 )
#define MM_PAGE_FLAG_READ_WRITE ( 1ull << 1 )
#define MM_PAGE_FLAG_USER       ( 1ull << 2 )
#define MM_PAGE_FLAG_LARGE_PAGE ( 1ull << 7 )
#define MM_PAGE_FLAG_NO_EXECUTE ( 1ull << 63 )

#define MM_BYTES_TO_GB( Bytes ) (Bytes / ( 1024 * 1024 * 1024))
#define MM_BYTES_TO_MB( Bytes ) (Bytes / ( 1024 * 1024))

#define MM_GB_TO_BYTES( Gb ) (Gb * (1024 * 1024 * 1024))
#define MM_MB_TO_BYTES( Mb ) (Mb * (1024 * 1024))

typedef union _MM_VIRTUAL_ADDRESS
{
	ULONG64 Value;

	struct
	{
		ULONG64 Offset	  : 12;
		ULONG64 PtIndex   : 9;
		ULONG64 PdIndex   : 9;
		ULONG64 PdptIndex : 9;
		ULONG64 Pml4Index : 9;
		ULONG64 Reserved  : 16;
	};

	struct
	{
		ULONG64 Offset	  : 21;
		ULONG64 PdIndex   : 9;
		ULONG64 PdptIndex : 9;
		ULONG64 Pml4Index : 9;
		ULONG64 Reserved  : 16;

	} Large;

	struct
	{
		ULONG64 Offset    : 30;
		ULONG64 PdptIndex : 9;
		ULONG64 Pml4Index : 9;
		ULONG64 Reserved  : 16;

	} Huge;

} MM_VIRTUAL_ADDRESS, * PMM_VIRTUAL_ADDRESS;

static_assert(sizeof(MM_VIRTUAL_ADDRESS) == sizeof(ULONG64), "MM_VIRTUAL_ADDRESS Must be 8 bytes in size");

typedef union _MM_PHYSICAL_ADDRESS
{
	ULONG64 Value;

	struct
	{
		ULONG64 Offset : 12;
	};

	struct
	{
		ULONG64 Offset : 21;

	} Large;

	struct
	{
		ULONG64 Offset : 30;

	} Huge;

} MM_PHYSICAL_ADDRESS, * PMM_PHYSICAL_ADDRESS;

static_assert(sizeof(MM_PHYSICAL_ADDRESS) == sizeof(ULONG64), "MM_PHYSICAL_ADDRESS Must be 8 bytes in size");

typedef union _MM_PTE_DESCRIPTOR
{
    ULONG64 Value;

    //
    // Intel SDM Volume 3
    // Table 5-20. Format of a Page-Table Entry that Maps a 4-KByte Page
    //
    struct
    {
        ULONG64 Present          : 1;   // [0]
        ULONG64 Writable         : 1;   // [1]
        ULONG64 UserSupervisor   : 1;   // [2]
        ULONG64 PageWriteThrough : 1;   // [3]
        ULONG64 PageCacheDisable : 1;   // [4]
        ULONG64 Accessed         : 1;   // [5]
        ULONG64 Dirty            : 1;   // [6]
        ULONG64 PAT              : 1;   // [7]
        ULONG64 Global           : 1;   // [8]
        ULONG64 Ignored1         : 2;   // [10:9]
        ULONG64 HLATRestart      : 1;   // [11]
        ULONG64 PageFrameNumber  : 36;  // [47:12]
        ULONG64 Reserved0        : 4;   // [51:48]
        ULONG64 Ignored2         : 7;   // [58:52]
        ULONG64 ProtectionKey    : 4;   // [62:59]
        ULONG64 NoExecute        : 1;   // [63]
    };

    //
    // Intel SDM Volume 3
    // Table 5-18. Format of a Page-Directory Entry that Maps a 2-MByte Page
    //
    struct
    {
        ULONG64 Present          : 1;   // [0]
        ULONG64 Writable         : 1;   // [1]
        ULONG64 UserSupervisor   : 1;   // [2]
        ULONG64 PageWriteThrough : 1;   // [3]
        ULONG64 PageCacheDisable : 1;   // [4]
        ULONG64 Accessed         : 1;   // [5]
        ULONG64 Dirty            : 1;   // [6]
        ULONG64 LargePage        : 1;   // [7]
        ULONG64 Global           : 1;   // [8]
        ULONG64 Ignored1         : 2;   // [10:9]
        ULONG64 HLATRestart      : 1;   // [11]
        ULONG64 PAT              : 1;   // [12]
        ULONG64 Reserved0        : 8;   // [20:13]
        ULONG64 PageFrameNumber  : 31;  // [51:21]
        ULONG64 Reserved1        : 1;   // [52]
        ULONG64 Ignored2         : 6;   // [58:53]
        ULONG64 ProtectionKey    : 4;   // [62:59]
        ULONG64 NoExecute        : 1;   // [63]

    } Large;

    //
    // Intel SDM Volume 3
    // Table 5-16. Format of a Page-Directory-Pointer-Table Entry (PDPTE) that Maps a 1-GByte Page
    //
    struct
    {
        ULONG64 Present          : 1;   // [0]
        ULONG64 Writable         : 1;   // [1]
        ULONG64 UserSupervisor   : 1;   // [2]
        ULONG64 PageWriteThrough : 1;   // [3]
        ULONG64 PageCacheDisable : 1;   // [4]
        ULONG64 Accessed         : 1;   // [5]
        ULONG64 Dirty            : 1;   // [6]
        ULONG64 HugePage         : 1;   // [7]
        ULONG64 Global           : 1;   // [8]
        ULONG64 Ignored1         : 2;   // [10:9]
        ULONG64 HLATRestart      : 1;   // [11]
        ULONG64 PAT              : 1;   // [12]
        ULONG64 Reserved0        : 17;  // [29:13]
        ULONG64 PageFrameNumber  : 22;  // [51:30]
        ULONG64 Reserved1        : 1;   // [52]
        ULONG64 Ignored2         : 6;   // [58:53]
        ULONG64 ProtectionKey    : 4;   // [62:59]
        ULONG64 NoExecute        : 1;   // [63]

    } Huge;

    union
    {
        struct
        {
            ULONG64 Readable        : 1;   // [0]
            ULONG64 Writable        : 1;   // [1]
            ULONG64 Executable      : 1;   // [2]
            ULONG64 MemoryType      : 3;   // [5:3]
            ULONG64 IgnorePAT       : 1;   // [6]
            ULONG64 Reserved0       : 1;   // [7]
            ULONG64 Accessed        : 1;   // [8]
            ULONG64 Dirty           : 1;   // [9]
            ULONG64 UserExecute     : 1;   // [10]
            ULONG64 Reserved1       : 1;   // [11]
            ULONG64 PageFrameNumber : 36;  // [47:12]
            ULONG64 Reserved2       : 15;  // [62:48]
            ULONG64 SupressVE       : 1;   // [63]
        };

        struct
        {
            ULONG64 Readable        : 1;   // [0]
            ULONG64 Writable        : 1;   // [1]
            ULONG64 Executable      : 1;   // [2]
            ULONG64 MemoryType      : 3;   // [5:3]
            ULONG64 IgnorePAT       : 1;   // [6]
            ULONG64 LargePage       : 1;   // [7]
            ULONG64 Accessed        : 1;   // [8]
            ULONG64 Dirty           : 1;   // [9]
            ULONG64 UserExecute     : 1;   // [10]
            ULONG64 Reserved1       : 10;  // [20:11]
            ULONG64 PageFrameNumber : 27;  // [47:21]
            ULONG64 Reserved2       : 15;  // [62:48]
            ULONG64 SupressVE       : 1;   // [63]

        } Large;

        struct
        {
            ULONG64 Readable        : 1;   // [0]
            ULONG64 Writable        : 1;   // [1]
            ULONG64 Executable      : 1;   // [2]
            ULONG64 MemoryType      : 3;   // [5:3]
            ULONG64 IgnorePAT       : 1;   // [6]
            ULONG64 LargePage       : 1;   // [7]
            ULONG64 Accessed        : 1;   // [8]
            ULONG64 Dirty           : 1;   // [9]
            ULONG64 UserExecute     : 1;   // [10]
            ULONG64 Reserved1       : 19;  // [29:11]
            ULONG64 PageFrameNumber : 18;  // [47:30]
            ULONG64 Reserved2       : 15;  // [62:48]
            ULONG64 SupressVE       : 1;   // [63]

        } Huge;

    } Extended;

} MM_PTE_DESCRIPTOR, * PMM_PTE_DESCRIPTOR;

static_assert(sizeof(MM_PTE_DESCRIPTOR) == sizeof(ULONG64), "MM_PTE_DESCRIPTOR Must be 8 bytes in size");


typedef union _MM_PAGING_BASE_DESCRIPTOR
{
    ULONG64 Value;

    struct
    {
        ULONG64 Reserved1             : 3;
        ULONG64 PageLevelWriteThrough : 1;
        ULONG64 PageLevelCacheDisable : 1;
        ULONG64 Reserved2             : 7;
        ULONG64 PageFrameNumber       : 36;
        ULONG64 Reserved3             : 16;
    };

    struct
    {
        ULONG64 Pcid            : 12; 
        ULONG64 PageFrameNumber : 36; 
        ULONG64 Reserved1       : 16; 
    } Pcide;

} MM_PAGING_BASE_DESCRIPTOR, * PMM_PAGING_BASE_DESCRIPTOR;

static_assert(sizeof(MM_PAGING_BASE_DESCRIPTOR) == sizeof(ULONG64), "MM_PAGING_BASE_DESCRIPTOR Must be 8 bytes");

FORCEINLINE
BOOLEAN
MmIsAddressCanonical(
    _In_ ULONG64 Value
)
{
    if (Value & MM_CANONICAL_SIGN_BIT)
    {
        return ((Value >> 48ull) == 0xFFFFull);
    }

    return ((Value >> 48ull) == 0x0000ull);
}

FORCEINLINE
ULONG64
MmMakeAddressCanonical(
    _In_ ULONG64 Value
)
{
    if (Value & MM_CANONICAL_SIGN_BIT)
    {
        Value |= MM_CANONICAL_HIGH_MASK;
    }

    return Value;
}

FORCEINLINE
ULONG64
MmMakeAddressNotCanonical(
    _In_ ULONG64 Value
)
{
    return Value & MM_CANONICAL_LOW_MASK;
}

FORCEINLINE
ULONG64
MmMakeVirtualAddress(
    _In_ ULONG64 Pml4Index,
    _In_ ULONG64 PdptIndex,
    _In_ ULONG64 PdIndex,
    _In_ ULONG64 PtIndex,
    _In_ ULONG64 Offset
)
{
    MM_VIRTUAL_ADDRESS VirtualAddress = { 0 };

    VirtualAddress.Pml4Index = Pml4Index;
    VirtualAddress.PdptIndex = PdptIndex;
    VirtualAddress.PdIndex = PdIndex;
    VirtualAddress.PtIndex = PtIndex;
    VirtualAddress.Offset = Offset;

    return MmMakeAddressCanonical(VirtualAddress.Value);
}

FORCEINLINE
ULONG64
MmMakeLargeVirtualAddress(
    _In_ ULONG64 Pml4Index,
    _In_ ULONG64 PdptIndex,
    _In_ ULONG64 PdIndex,
    _In_ ULONG64 Offset
)
{
    MM_VIRTUAL_ADDRESS VirtualAddress = { 0 };

    VirtualAddress.Large.Pml4Index = Pml4Index;
    VirtualAddress.Large.PdptIndex = PdptIndex;
    VirtualAddress.Large.PdIndex = PdIndex;
    VirtualAddress.Large.Offset = Offset;

    return MmMakeAddressCanonical(VirtualAddress.Value);
}

FORCEINLINE
ULONG64
MmMakeHugeVirtualAddress(
    _In_ ULONG64 Pml4Index,
    _In_ ULONG64 PdptIndex,
    _In_ ULONG64 Offset
)
{
    MM_VIRTUAL_ADDRESS VirtualAddress = { 0 };

    VirtualAddress.Huge.Pml4Index = Pml4Index;
    VirtualAddress.Huge.PdptIndex = PdptIndex;
    VirtualAddress.Huge.Offset = Offset;

    return MmMakeAddressCanonical(VirtualAddress.Value);
}

FORCEINLINE
ULONG64
__MmR_xProtection(
    VOID
)
{
    MM_PTE_DESCRIPTOR Protection = { 0 };
    Protection.Present   = TRUE;
    Protection.Writable  = FALSE;
    Protection.NoExecute = FALSE;
    return Protection.Value;
};

FORCEINLINE
ULONG64
__MmRw_Protection(
    VOID
)
{
    MM_PTE_DESCRIPTOR Protection = { 0 };
    Protection.Present   = TRUE;
    Protection.Writable  = TRUE;
    Protection.NoExecute = TRUE;
    return Protection.Value;
};

FORCEINLINE
ULONG64
__MmR__Protection(
    VOID
)
{
    MM_PTE_DESCRIPTOR Protection = { 0 };
    Protection.Present   = TRUE;
    Protection.Writable  = FALSE;
    Protection.NoExecute = TRUE;
    return Protection.Value;
};

FORCEINLINE
ULONG64
__MmRwxProtection(
    VOID
)
{
    MM_PTE_DESCRIPTOR Protection = { 0 };
    Protection.Present   = TRUE;
    Protection.Writable  = TRUE;
    Protection.NoExecute = FALSE;
    return Protection.Value;
};

#define MmReadExecuteProtection      \
(                                    \
    MM_PAGE_FLAG_PRESENT             \
)                                    
                                     
#define MmReadWriteProtection        \
(                                    \
    MM_PAGE_FLAG_PRESENT    |        \
    MM_PAGE_FLAG_READ_WRITE |        \
    MM_PAGE_FLAG_NO_EXECUTE          \
)

#define MmReadOnlyProtection         \
(                                    \
    MM_PAGE_FLAG_PRESENT    |        \
    MM_PAGE_FLAG_NO_EXECUTE          \
)

#define MmReadWriteExecuteProtection \
(                                    \
    MM_PAGE_FLAG_PRESENT    |        \
    MM_PAGE_FLAG_READ_WRITE          \
)                             

FORCEINLINE
ULONG64
__MmStrongCacheDisablePolicy(
    VOID
)
{
    MM_PTE_DESCRIPTOR Protection = { 0 };
    Protection.PageWriteThrough = TRUE;
    Protection.PageCacheDisable = TRUE;
    return Protection.Value;
};


FORCEINLINE
ULONG64
__MmCacheDisablePolicy(
    VOID
)
{
    MM_PTE_DESCRIPTOR Protection = { 0 };
    Protection.PageWriteThrough = FALSE;
    Protection.PageCacheDisable = TRUE;
    return Protection.Value;
};

FORCEINLINE
ULONG64
__MmWritebackPolicy(
    VOID
)
{
    MM_PTE_DESCRIPTOR Protection = { 0 };
    Protection.PageWriteThrough = FALSE;
    Protection.PageCacheDisable = FALSE;
    return Protection.Value;
};

// Strong cacheability meaning MTRRs cannot override this. UC.
#define MmStrongUncacheablePagePolicy \
(                                     \
    __MmStrongCacheDisablePolicy()    \
)

// MTRRs can override this cacheability from UC- to WC (Write-Combining).
#define MmWeakUncacheablePagePolicy   \
(                                     \
    __MmCacheDisablePolicy()          \
)    

#define MmWritebackPagePolicy         \
(                                     \
    __MmWritebackPolicy()             \
)                                     

FORCEINLINE
BOOLEAN
MmMakePageStrongUncacheable(
    PMM_PTE_DESCRIPTOR Page
)
{
    if (!Page)
    {
        return FALSE;
    }

    Page->Value |= MmStrongUncacheablePagePolicy;

    return TRUE;
}

FORCEINLINE
BOOLEAN
MmMakePageWeaKUncacheable(
    PMM_PTE_DESCRIPTOR Page
)
{
    if (!Page)
    {
        return FALSE;
    }

    Page->Value |= MmWeakUncacheablePagePolicy;

    return TRUE;
}

FORCEINLINE
BOOLEAN
MmMakePageWriteback(
    PMM_PTE_DESCRIPTOR Page
)
{
    if (!Page)
    {
        return FALSE;
    }

    Page->Value |= MmWritebackPagePolicy;

    return TRUE;
}


#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // !ARCHX64_PAGING_H