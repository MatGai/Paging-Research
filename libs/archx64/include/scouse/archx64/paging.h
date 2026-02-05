#ifndef ARCHX64_PAGING_H
#define ARCHX64_PAGING_H

#include <scouse/shared/typedefs.h>

#define PAGE_SHIFT  12
#define PAGE_SIZE   ( 1ull << PAGE_SHIFT )
		   
#define LARGE_SHIFT 21
#define LARGE_SIZE  ( 1ull << LARGE_SHIFT )
		   
#define HUGE_SHIFT  30
#define HUGE_SIZE   ( 1ull << HUGE_SHIFT )

#define PT_ENTRIES 512 
#define INDEX_MASK 0x1FFull 

typedef ULONG64 PteValue;

typedef struct { PteValue V; } Pml4Entry, * PPml4Entry;
typedef struct { PteValue V; } PdptEntry, * PPdptEntry;
typedef struct { PteValue V; } PdEntry,   * PPdEntry;
typedef struct { PteValue V; } PtEntry,   * PPtEntry;

typedef struct 
{ 
	Pml4Entry E [ PT_ENTRIES ];
} Pml4t, * PPml4t;

typedef struct 
{ 
	PdptEntry E [ PT_ENTRIES ];
} Pdpt, * PPdpt;
 

typedef struct{ 
	PdEntry   E [ PT_ENTRIES ];
} Pdt,   * PPdt;

typedef struct 
{ 
	PtEntry   E [ PT_ENTRIES ];
} Pt,   * PPt;

static_assert( sizeof(ULONG64) == 8, "Page entires must be 8 bytes in size" );
static_assert( sizeof(Pml4t) == 0x1000, "Page tables must be 4KiB in size" );

enum PT_FLAGS
{
	PT_PRESENT			= 1ull <<  0,
	PT_READ_WRITE		= 1ull <<  1,
	PT_USER				= 1ull <<  2,
	PT_WRITE_THROUGH	= 1ull <<  3,
	PT_CACHE_DISABLE	= 1ull <<  4,
	PT_ACCESSED			= 1ull <<  5,
	PT_DIRTY			= 1ull <<  6,
	PT_PAGE_SIZE		= 1ull <<  7,
	PT_GLOBAL			= 1ull <<  8,
	PT_NX				= 1ull <<  63
};

static
__forceinline
BOOLEAN
IsAddressCanonical(
	_In_ ULONG64 Value
);

static
__forceinline
ULONG64
MakeAddressCanonical(
	_In_ ULONG64 Value
);

static
__forceinline
ULONG64
MakeAddressNotCanonical(
	_In_ ULONG64 Value
);

static
__forceinline
ULONG64
Pml4Index(
	_In_ ULONG64 Value
);

static
__forceinline
ULONG64
PdptIndex(
	_In_ ULONG64 Value
);

static
__forceinline
ULONG64
PdIndex(
	_In_ ULONG64 Value
);

static
__forceinline
ULONG64
PtIndex(
	_In_ ULONG64 Value
);

static 
__forceinline
ULONG64
PageOffset(
	_In_ ULONG64 Value
);

static
__forceinline
ULONG64
PageEntryPresent(
	_In_ ULONG64 Value
);

static
__forceinline
ULONG64
PageEntryLarge (
	_In_ ULONG64 Value
);

static
__forceinline
ULONG64
PageEntryAddress(
	_In_ ULONG64 Value,
	_In_ ULONG64 PagePfnMask
);

static
__forceinline
ULONG64
PageMakeEntry(
	_In_ ULONG64 Physical,
	_In_ ULONG64 Flags,
	_In_ ULONG64 PagePfnMask
);

static
__forceinline
ULONG64
Pml4MakeEntry(
	_In_ ULONG64 Physical,
	_In_ ULONG64 Flags,
	_In_ ULONG64 PagePfnMask
);

static
__forceinline
ULONG64
PdptMakeEntry(
	_In_ ULONG64 Physical,
	_In_ ULONG64 Flags,
	_In_ ULONG64 PagePfnMask
);

static
__forceinline
ULONG64
PdptMakeTable(
	_In_ ULONG64 Physical,
	_In_ ULONG64 Flags,
	_In_ ULONG64 PagePfnMask
);

static
__forceinline
ULONG64
PdMakeEntry(
	_In_ ULONG64 Physical,
	_In_ ULONG64 Flags,
	_In_ ULONG64 PagePfnMask
);

static
__forceinline
ULONG64
PdMakeTable(
	_In_ ULONG64 Physical,
	_In_ ULONG64 Flags,
	_In_ ULONG64 PagePfnMask
);

static
__forceinline
ULONG64
PtMakeEntry(
	_In_ ULONG64 Physical,
	_In_ ULONG64 Flags,
	_In_ ULONG64 PagePfnMask
);

static
__forceinline
ULONG64
Pml4EntryValue(
	_In_ Pml4Entry Entry
);

static
__forceinline
ULONG64
PdptEntryValue(
	_In_ PdptEntry Entry
); 

static
__forceinline
ULONG64
PdEntryValue(
	_In_ PdEntry Entry
);

static
__forceinline
ULONG64
PtEntryValue(
	_In_ PtEntry Entry
);

static
__forceinline
BOOLEAN
IsAlignedPage(
	_In_ ULONG64 Value
);

static
__forceinline
BOOLEAN
IsAlignedLarge(
	_In_ ULONG64 Value
);

static
__forceinline
BOOLEAN
IsAlignedHuge(
	_In_ ULONG64 Value
);


#endif //!ARCHX64_PAGING_H