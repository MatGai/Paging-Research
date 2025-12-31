#include <scouse/archx64/cpu/cpudata.h>
#include <scouse/shared/cpuinfo.h>
#include <scouse/runtime/string.h>



VOID
GetCacheInfo(
	_Out_ PCPU_CACHE_INFO CacheInfo,
	_In_  ULONG32    Index
)
{
    if ( !CacheInfo )
	{
		return;
	}

    CPUINFO CpuInfo = { 0 };
	GetCpuInfo( &CpuInfo );

	CacheInfo->Size = 0;

	if ( strstr( CpuInfo.Vendor, CPU_AUTHENTIC_AMD ) )
	{
		// max extended leaf, 0x8000001D
		if( ( 1ll << 22 ) & CpuInfo.X86.Leaf80000001.Ecx )
		{
			ParseCacheInfo(CacheInfo, CpuInfo.X86.MaxExtendedFunction, 0x8000001D );
		}
	}
    else if ( strstr( CpuInfo.Vendor, CPU_GENUINE_INTEL ) )
	{
		// max standard leaf, 0x00000004
		REGISTER_SET Leaf = CpuInfo.X86.Leaf2;

		Leaf.Eax &= 0xFFFFFF00;

		if( Leaf.Eax & ( 1ll << 31 ) )
		{
			Leaf.Eax = 0;
		}
		if( Leaf.Ebx & ( 1ll << 31 ) )
		{
			Leaf.Ebx = 0;
		}
		if( Leaf.Ecx & ( 1ll << 31 ) )
		{
			Leaf.Ecx = 0;
		}
		if( Leaf.Edx & ( 1ll << 31 ) ) 
		{
			Leaf.Edx = 0;
		}

		//todo - finish parsing leaf 2

        ParseCacheInfo(CacheInfo, CpuInfo.X86.MaxStandardFunction, 0x00000004);
	}
}

VOID
ParseCacheInfo(
	_Out_ PCPU_CACHE_INFO CacheInfo,
	_In_ ULONG32 MaxLeaf,
	_In_ ULONG32 LeafId
)
{

	if (!CacheInfo)
	{
		return;
	}

	for( ULONG Index = 0; CacheInfo->Size < CPU_MAX_CACHE_LEVEL; ++Index )
	{
		REGISTER_SET Leaf = { 0 };
        _scouse_cpuidex(LeafId, Index, &Leaf.Registers);

        switch (Leaf.Eax & 0x1F)
		{
			case 0:
			{
				return;
			}
			case 1:
			{
				CacheInfo->Levels[ CacheInfo->Size ].CacheType = CPU_FEATURE_CACHE_DATA;
				break;
			}
			case 2:
			{
				CacheInfo->Levels[ CacheInfo->Size ].CacheType = CPU_FEATURE_CACHE_INSTRUCTION;
				break;
			}
			case 3:
			{
				CacheInfo->Levels[ CacheInfo->Size ].CacheType = CPU_FEATURE_CACHE_UNIFIED;
				break;
			}
			default:
			{
				CacheInfo->Levels[ CacheInfo->Size ].CacheType = CPU_FEATURE_CACHE_NULL;
				break;
			}
		}

        CacheInfo->Levels[ CacheInfo->Size ].Level		  = (LONG32)( ( Leaf.Eax >> 5 ) & 0x7 );
		CacheInfo->Levels[ CacheInfo->Size ].LineSize	  = (LONG32)( ( Leaf.Ebx & 0xFFF ) + 1 );
        CacheInfo->Levels[ CacheInfo->Size ].Ways		  = (LONG32)( ( ( Leaf.Ebx >> 22 ) & 0x3FF ) + 1 );
		CacheInfo->Levels[ CacheInfo->Size ].Partitioning = (LONG32)( ( ( Leaf.Ebx >> 12 ) & 0x3FF ) + 1 );
		CacheInfo->Levels[ CacheInfo->Size ].TlbEntries   = (LONG32)( Leaf.Ecx + 1 );
		CacheInfo->Levels[ CacheInfo->Size ].CacheSize	  = (LONG32)( CacheInfo->Levels[ CacheInfo->Size ].TlbEntries  *
																	  CacheInfo->Levels[ CacheInfo->Size ].Ways *
																	  CacheInfo->Levels[ CacheInfo->Size ].Partitioning *
																	  CacheInfo->Levels[ CacheInfo->Size ].LineSize );

		++CacheInfo->Size;
	}
}