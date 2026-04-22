#ifndef ARCHX64_CPUDATA_H
#define ARCHX64_CPUDATA_H

#include <scouse/shared/typedefs.h>
#include <scouse/archx64/paging.h>

#define CPU_AUTHENTIC_AMD ( "AuthenticAMD" )
#define CPU_GENUINE_INTEL ( "GenuineIntel" )

typedef enum _CPU_CACHE_TYPE
{
	CPU_FEATURE_CACHE_NULL = 0,
	CPU_FEATURE_CACHE_DATA, 
	CPU_FEATURE_CACHE_INSTRUCTION,
	CPU_FEATURE_CACHE_UNIFIED,
	CPU_FEATURE_CACHE_TLB,
	CPU_FEATURE_CACHE_DTLB,
	CPU_FEATURE_CACHE_STLB,
	CPU_FEATURE_CACHE_PREFETCH
} CPU_CACHE_TYPE;

typedef struct _CACHE_LEVEL_INFO
{
	INT16 level;
	CPU_CACHE_TYPE cache_type;
	LONG32 cache_size;
	LONG32 ways;
	LONG32 line_size;
	LONG32 tlb_entries;
	LONG32 partitioning;

} CacheLevelInfo, *PCacheLevelInfo;

typedef struct _CACHE_INFO
{
	INT16 Level;
	CPU_CACHE_TYPE CacheType;
	LONG32 CacheSize;
	LONG32 Ways;
	LONG32 LineSize;
	LONG32 TlbEntries;
	LONG32 Partitioning;

} CACHE_INFO, * PCACHE_INFO;

#define CPU_MAX_CACHE_LEVEL ( 32 )

typedef struct _CPU_CACHE_INFO
{
	LONG32 Size;
	CACHE_INFO Levels[CPU_MAX_CACHE_LEVEL];
}CPU_CACHE_INFO, *PCPU_CACHE_INFO;


#define CPU_AMD_STANDARD_FUNCTIONS ( 0x000000000000FFFFull )  // Amd standard functions to query features such as AVX and FMA

VOID
GetCacheInfo(
	_Out_ PCPU_CACHE_INFO CacheInfo,
	_In_  ULONG32    Index
);

VOID
ParseCacheInfo(
	_Out_ PCPU_CACHE_INFO CacheInfo,
	_In_ ULONG32 MaxLeaf,
	_In_ ULONG32 LeafId
);

#endif // !ARCHX64_CPUDATA_H
