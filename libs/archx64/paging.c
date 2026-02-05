#include <scouse/archx64/paging.h>

static
__forceinline
BOOLEAN
IsAddressCanonical(
	_In_ ULONG64 Value
) 
{
	if( Value & ( ULONG64 )1 << 47ull )
	{
		return (( Value >> 48ull ) == 0xFFFFull);
	}

	return (( Value >> 48ull ) == 0x0000ull);
}

static
__forceinline
ULONG64
MakeAddressCanonical(
	_In_ ULONG64 Value
)
{
	if (Value & (ULONG64)1 << 47ull)
	{
		Value |= 0xFFFF000000000000ull;
	}

	return Value;
}

static
__forceinline
ULONG64
MakeAddressNotCanonical(
	_In_ ULONG64 Value
)
{
	return Value & 0x0000FFFFFFFFFFFFull;
}

static
__forceinline
ULONG64
Pml4Index(
	_In_ ULONG64 Value
)
{
	return (ULONG64)( ( Value >> 39 ) & INDEX_MASK );
}

static
__forceinline
ULONG64
PdptIndex(
	_In_ ULONG64 Value
)
{
	return (ULONG64)( ( Value >> 30 ) & INDEX_MASK );
}

static
__forceinline
ULONG64
PdIndex(
	_In_ ULONG64 Value
)
{
	return (ULONG64)( ( Value >> 21 ) & INDEX_MASK );
}

static
__forceinline
ULONG64
PtIndex(
	_In_ ULONG64 Value
)
{
	return (ULONG64)( ( Value >> 12 ) & INDEX_MASK );
}

static
__forceinline
ULONG64
PageOffset(
	_In_ ULONG64 Value
)
{
	return (ULONG64)( Value & 0xFFFull );
}

static
__forceinline
ULONG64
PageEntryPresent(
	ULONG64 Value
)
{
	return (Value & PT_PRESENT) != 0;
}

static
__forceinline
ULONG64
PageEntryLarge(
	ULONG64 Value
)
{
	return ( Value & PT_PAGE_SIZE ) != 0;
}

static
__forceinline
ULONG64
PageEntryAddress(
	ULONG64 Value,
	ULONG64 PagePfnMask
)
{
	return Value & PagePfnMask;
}

static
__forceinline
ULONG64
PageMakeEntry(
	ULONG64 Physical,
	ULONG64 Flags,
	ULONG64 PagePfnMask
)
{
	return ( Physical | PagePfnMask ) | Flags;
}

static
__forceinline
ULONG64
Pml4MakeEntry(
	ULONG64 Physical,
	ULONG64 Flags,
	ULONG64 PagePfnMask
)
{
	return PageMakeEntry( Physical, Flags, PagePfnMask );
}

static
__forceinline
ULONG64
PdptMakeEntry(
	ULONG64 Physical,
	ULONG64 Flags,
	ULONG64 PagePfnMask
)
{
	return PageMakeEntry( Physical, Flags | PT_PAGE_SIZE, PagePfnMask );
}

static
__forceinline
ULONG64
PdptMakeTable(
	ULONG64 Physical,
	ULONG64 Flags,
	ULONG64 PagePfnMask
)
{
	return PageMakeEntry(Physical, Flags, PagePfnMask);
}

static
__forceinline
ULONG64
PdMakeEntry(
	ULONG64 Physical,
	ULONG64 Flags,
	ULONG64 PagePfnMask
)
{
	return PageMakeEntry(Physical, Flags | PT_PAGE_SIZE, PagePfnMask);
}

static
__forceinline
ULONG64
PdMakeTable(
	ULONG64 Physical,
	ULONG64 Flags,
	ULONG64 PagePfnMask
)
{
	return PageMakeEntry(Physical, Flags, PagePfnMask);
}

static
__forceinline
ULONG64
PtMakeEntry(
	ULONG64 Physical,
	ULONG64 Flags,
	ULONG64 PagePfnMask
)
{
	return PageMakeEntry(Physical, Flags, PagePfnMask);
}

static
__forceinline
ULONG64
Pml4EntryValue(
	_In_ Pml4Entry Entry
)
{
	return Entry.V;
}

static
__forceinline
ULONG64
PdptEntryValue(
	_In_ PdptEntry Entry
)
{
	return Entry.V;
}

static
__forceinline
ULONG64
PdEntryValue(
	_In_ PdEntry Entry
)
{
	return Entry.V;
}

static
__forceinline
ULONG64
PtEntryValue(
	_In_ PtEntry Entry
)
{
	return Entry.V;
}

static
__forceinline
BOOLEAN
IsAlignedPage(
	ULONG64 Value
)
{
	return (Value & (PAGE_SIZE - 1ull)) == 0;
}

static
__forceinline
BOOLEAN
IsAlignedLarge(
	ULONG64 Value
)
{
	return (Value & (LARGE_SIZE -1ull)) == 0;
}

static
__forceinline
BOOLEAN
IsAlignedHuge(
	ULONG64 Value
)
{
	return (Value & (HUGE_SIZE - 1ull)) == 0;
}
