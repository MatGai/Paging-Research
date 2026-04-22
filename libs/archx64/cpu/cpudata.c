#include <scouse/archx64/cpu/cpudata.h>
#include <scouse/shared/cpuinfo.h>
#include <scouse/runtime/string.h>
#include <scouse/archx64/intrinsics.h>

STATIC CacheLevelInfo EmptyCacheLevelInfo = { 0 };

FORCEINLINE 
BOOLEAN 
IsIntel(
    _In_ CONST CPUINFO* CpuInfo
)
{
    if( !CpuInfo )
    {
        return FALSE;
    }

    return memcmp(CpuInfo->Vendor, CPU_GENUINE_INTEL, 12) == 0;
}

FORCEINLINE
BOOLEAN 
IsAmd(
    _In_ CONST CPUINFO* CpuInfo
)
{
    if (!CpuInfo)
    {
        return FALSE;
    }

    return memcmp(CpuInfo->Vendor, CPU_AUTHENTIC_AMD, 12) == 0;
}

FORCEINLINE 
VOID
AddLevel(
    _Inout_ PCPU_CACHE_INFO Info, 
    _In_    CACHE_INFO Entry
)
{
    if ( Info && Info->Size < CPU_MAX_CACHE_LEVEL )
    {
        Info->Levels[Info->Size++] = Entry;
    }
}

STATIC 
ULONG32 
AmdAssocDecode(
    ULONG32 Encoding
)
{
    switch (Encoding & 0xF)
    {
        case 0x0: return 0;   // disabled
        case 0x1: return 1;
        case 0x2: return 2;
        case 0x4: return 4;
        case 0x6: return 8;
        case 0x8: return 16;
        case 0xA: return 32;
        case 0xB: return 48;
        case 0xC: return 64;
        case 0xD: return 96;
        case 0xE: return 128;
        default:  return 0;   // reserved/unknown
    }
}


VOID
ParseCacheInfo(
    _Out_ PCPU_CACHE_INFO CacheInfo,
    _In_  ULONG32 MaxLeaf,
    _In_  ULONG32 LeafId
)
{
    if (!CacheInfo) return;

    if (MaxLeaf < LeafId)
    {
        return;
    }

    for (ULONG32 Sub = 0; CacheInfo->Size < CPU_MAX_CACHE_LEVEL; ++Sub)
    {
        REGISTER_SET Leaf = { 0 };
        _scouse_cpuidex(LeafId, Sub, Leaf.Registers);

        ULONG32 Type = (Leaf.Eax & 0x1F);
        if (Type == 0)
        {
            return;
        }

        CPU_CACHE_TYPE Ct = CPU_FEATURE_CACHE_NULL;

        switch (Type)
        {
            case 1:
                Ct = CPU_FEATURE_CACHE_DATA;
                break;
            case 2:
                Ct = CPU_FEATURE_CACHE_INSTRUCTION;
                break;
            case 3:
                Ct = CPU_FEATURE_CACHE_UNIFIED;
                break;
            default:
                break;
        }

        ULONG32 Level = (Leaf.Eax >> 5) & 0x7;

        ULONG32 LineSize = (Leaf.Ebx & 0xFFF) + 1;
        ULONG32 Partitions = ((Leaf.Ebx >> 12) & 0x3FF) + 1;
        ULONG32 Ways = ((Leaf.Ebx >> 22) & 0x3FF) + 1;
        ULONG32 Sets = Leaf.Ecx + 1;

        ULONG64 SizeBytes = (ULONG64)LineSize * (ULONG64)Partitions * (ULONG64)Ways * (ULONG64)Sets;

        CACHE_INFO Out = { 0 };
        Out.Level = (LONG32)Level;
        Out.CacheType = Ct;
        Out.LineSize = (LONG32)LineSize;
        Out.Partitioning = (LONG32)Partitions;
        Out.Ways = (LONG32)Ways;

        Out.TlbEntries = (LONG32)Sets;

        if (SizeBytes > 0x7FFFFFFF)
        {
            Out.CacheSize = -1;
        }
        else
        {
            Out.CacheSize = (LONG32)SizeBytes;
        }

        AddLevel(CacheInfo, Out);
    }
}

#define CPU_MAX_CPUID_SUBLEAVES 32

STATIC
VOID
ParseIntelLeaf18Tlb(
    _Inout_ PCPU_CACHE_INFO CacheInfo,
    _In_ CONST CPUINFO* CpuInfo
)
{
    if (!CacheInfo || !CpuInfo)
    {
        return;
    }

    if (CpuInfo->X86.MaxStandardFunction < 0x18)
    {
        return;
    }

    REGISTER_SET Leaf0 = { 0 };
    _scouse_cpuidex(0x18, 0, Leaf0.Registers);

    if ((Leaf0.Eax | Leaf0.Ebx | Leaf0.Ecx | Leaf0.Edx) == 0)
    {
        return;
    }

    ULONG32 MaxSubleaf = Leaf0.Eax;

    // Hard sanity cap.
    if (MaxSubleaf > CPU_MAX_CPUID_SUBLEAVES)
    {
        MaxSubleaf = CPU_MAX_CPUID_SUBLEAVES;
    }

    // Start at 1, not 0.
    for (ULONG32 Sub = 1;
        Sub <= MaxSubleaf && CacheInfo->Size < CPU_MAX_CACHE_LEVEL;
        ++Sub)
    {
        REGISTER_SET L = { 0 };
        _scouse_cpuidex(0x18, Sub, L.Registers);

        ULONG32 TlbType = (L.Edx & 0x1F);
        if (TlbType == 0)
        {
            continue;
        }

        ULONG32 Level = (L.Edx >> 5) & 0x7;
        ULONG32 FullyAssoc = (L.Edx >> 8) & 1;

        ULONG32 PageMask = (L.Ebx & 0xF);
        ULONG32 Partition = (L.Ebx >> 8) & 0x7;
        ULONG32 Ways = (L.Ebx >> 16) & 0xFFFF;
        ULONG32 Sets = L.Ecx;

        if (Ways == 0 || Sets == 0)
        {
            continue;
        }

        ULONG64 Entries = (ULONG64)Ways * (ULONG64)Sets;
        if (Entries > 0x7FFFFFFF)
        {
            Entries = 0x7FFFFFFF;
        }

        CPU_CACHE_TYPE Ct = CPU_FEATURE_CACHE_TLB;

        switch (TlbType)
        {
        case 1:
            Ct = CPU_FEATURE_CACHE_DTLB;
            break;
        case 2:
            Ct = CPU_FEATURE_CACHE_TLB;
            break;
        case 3:
            Ct = CPU_FEATURE_CACHE_TLB;
            break;
        case 4:
        case 5:
            Ct = CPU_FEATURE_CACHE_DTLB;
            break;
        default:
            break;
        }

        CACHE_INFO Out = { 0 };
        Out.Level = (LONG32)Level;
        Out.CacheType = Ct;
        Out.Ways = (LONG32)Ways;
        Out.Partitioning = (LONG32)Partition;
        Out.LineSize = (LONG32)PageMask;
        Out.TlbEntries = (LONG32)Entries;
        Out.CacheSize = -1;

        (VOID)FullyAssoc;

        AddLevel(CacheInfo, Out);
    }
}

STATIC 
VOID
ParseAmdLegacyCachesTlbs(
    _Inout_ PCPU_CACHE_INFO CacheInfo, 
    _In_ CONST CPUINFO* CpuInfo
)
{
    if (!CacheInfo || !CpuInfo)
    {
        return;
    }

    if (CpuInfo->X86.MaxExtendedFunction < 0x80000005)
    {
        return;
    }

    REGISTER_SET L5 = { 0 };
    _scouse_cpuid(0x80000005, L5.Registers);

    // L1 D-cache: ECX
    {
        ULONG32 Line = (L5.Ecx & 0xFF);
        ULONG32 LinesPerTag = (L5.Ecx >> 8) & 0xFF;
        ULONG32 Assoc = (L5.Ecx >> 16) & 0xFF;
        ULONG32 SizeKB = (L5.Ecx >> 24) & 0xFF;

        CACHE_INFO Out = { 0 };
        Out.Level = 1;
        Out.CacheType = CPU_FEATURE_CACHE_DATA;
        Out.LineSize = (LONG32)Line;
        Out.Partitioning = (LONG32)LinesPerTag;
        Out.Ways = (LONG32)Assoc;
        Out.CacheSize = (LONG32)(SizeKB * 1024);
        Out.TlbEntries = -1;
        AddLevel(CacheInfo, Out);
    }

    // L1 I-cache EDX
    {
        ULONG32 Line = (L5.Edx & 0xFF);
        ULONG32 LinesPerTag = (L5.Edx >> 8) & 0xFF;
        ULONG32 Assoc = (L5.Edx >> 16) & 0xFF;
        ULONG32 SizeKB = (L5.Edx >> 24) & 0xFF;

        CACHE_INFO Out = { 0 };
        Out.Level = 1;
        Out.CacheType = CPU_FEATURE_CACHE_INSTRUCTION;
        Out.LineSize = (LONG32)Line;
        Out.Partitioning = (LONG32)LinesPerTag;
        Out.Ways = (LONG32)Assoc;
        Out.CacheSize = (LONG32)(SizeKB * 1024);
        Out.TlbEntries = -1;
        AddLevel(CacheInfo, Out);
    }

    // L1 huge-page TLBs EAX (2M/4M pages)
    {
        ULONG32 ITlbEnt = (L5.Eax & 0xFF);
        ULONG32 ITlbAssoc = (L5.Eax >> 8) & 0xFF;
        ULONG32 DTlbEnt = (L5.Eax >> 16) & 0xFF;
        ULONG32 DTlbAssoc = (L5.Eax >> 24) & 0xFF;

        CACHE_INFO I = { 0 };
        I.Level = 1;
        I.CacheType = CPU_FEATURE_CACHE_TLB;   // instruction TLB
        I.CacheSize = (LONG32)(2 * 1024 * 1024); // represent hugepage class
        I.Ways = (LONG32)ITlbAssoc;
        I.TlbEntries = (LONG32)ITlbEnt;
        I.LineSize = -1;
        AddLevel(CacheInfo, I);

        CACHE_INFO D = { 0 };
        D.Level = 1;
        D.CacheType = CPU_FEATURE_CACHE_DTLB;
        D.CacheSize = (LONG32)(2 * 1024 * 1024);
        D.Ways = (LONG32)DTlbAssoc;
        D.TlbEntries = (LONG32)DTlbEnt;
        D.LineSize = -1;
        AddLevel(CacheInfo, D);
    }

    // L1 4K TLBs EBX
    {
        ULONG32 ITlbEnt = (L5.Ebx & 0xFF);
        ULONG32 ITlbAssoc = (L5.Ebx >> 8) & 0xFF;
        ULONG32 DTlbEnt = (L5.Ebx >> 16) & 0xFF;
        ULONG32 DTlbAssoc = (L5.Ebx >> 24) & 0xFF;

        CACHE_INFO I = { 0 };
        I.Level = 1;
        I.CacheType = CPU_FEATURE_CACHE_TLB;
        I.CacheSize = (LONG32)(4 * 1024);
        I.Ways = (LONG32)ITlbAssoc;
        I.TlbEntries = (LONG32)ITlbEnt;
        I.LineSize = -1;
        AddLevel(CacheInfo, I);

        CACHE_INFO D = { 0 };
        D.Level = 1;
        D.CacheType = CPU_FEATURE_CACHE_DTLB;
        D.CacheSize = (LONG32)(4 * 1024);
        D.Ways = (LONG32)DTlbAssoc;
        D.TlbEntries = (LONG32)DTlbEnt;
        D.LineSize = -1;
        AddLevel(CacheInfo, D);
    }

    // Leaf 80000006  L2 (ECX) and L3 (EDX) if present
    if (CpuInfo->X86.MaxExtendedFunction >= 0x80000006 && CacheInfo->Size < CPU_MAX_CACHE_LEVEL)
    {
        REGISTER_SET L6 = { 0 };
        _scouse_cpuid(0x80000006, L6.Registers);

        // L2 in ECX line size bits 7:0, assoc bits 15:12, size KB bits 31:16
        {
            ULONG32 Line = (L6.Ecx & 0xFF);
            ULONG32 AssocEnc = (L6.Ecx >> 12) & 0xF;
            ULONG32 SizeKB = (L6.Ecx >> 16) & 0xFFFF;

            CACHE_INFO Out = { 0 };
            Out.Level = 2;
            Out.CacheType = CPU_FEATURE_CACHE_UNIFIED;
            Out.LineSize = (LONG32)Line;
            Out.Ways = (LONG32)AmdAssocDecode(AssocEnc);
            Out.CacheSize = (LONG32)(SizeKB * 1024);
            Out.TlbEntries = -1;
            Out.Partitioning = 0;
            AddLevel(CacheInfo, Out);
        }

        // L3 in EDX size in 512KiB units + assoc/lines/line
        {
            ULONG32 L3SizeUnits = (L6.Edx >> 18) & 0x3FFF;   // 512KiB units
            if (L3SizeUnits != 0)
            {
                ULONG32 Line = (L6.Edx & 0xFF);
                ULONG32 LinesPerTag = (L6.Edx >> 8) & 0xF;
                ULONG32 AssocEnc = (L6.Edx >> 12) & 0xF;

                ULONG64 SizeBytes = (ULONG64)L3SizeUnits * 512ull * 1024ull;

                CACHE_INFO Out = { 0 };
                Out.Level = 3;
                Out.CacheType = CPU_FEATURE_CACHE_UNIFIED;
                Out.LineSize = (LONG32)Line;
                Out.Partitioning = (LONG32)LinesPerTag;
                Out.Ways = (LONG32)AmdAssocDecode(AssocEnc);
                Out.TlbEntries = -1;
                Out.CacheSize = (SizeBytes > 0x7FFFFFFF) ? -1 : (LONG32)SizeBytes;
                AddLevel(CacheInfo, Out);
            }
        }
    }
}

VOID
GetCacheInfo(
    _Out_ PCPU_CACHE_INFO CacheInfo,
    _In_  ULONG32 Index
)
{
    (VOID)Index;

    if (!CacheInfo)
    {
        return;
    }

    CPUINFO CpuInfo = { 0 };
    GetCpuInfo(&CpuInfo);

    memset(CacheInfo, 0, sizeof(*CacheInfo));

    if (IsAmd(&CpuInfo))
    {
        ParseAmdLegacyCachesTlbs(CacheInfo, &CpuInfo);

        // AMD uses leaf 0x8000001D with same format as Intel leaf 4
        if (CpuInfo.X86.MaxExtendedFunction >= 0x8000001D)
        {
            ParseCacheInfo(CacheInfo, CpuInfo.X86.MaxExtendedFunction, 0x8000001D);
        }
    }
    else if (IsIntel(&CpuInfo))
    {
        if ( CpuInfo.X86.MaxStandardFunction >= 0x18 )
        {
            // Deterministic TLB
            ParseIntelLeaf18Tlb(CacheInfo, &CpuInfo);
        }

        if ( CpuInfo.X86.MaxStandardFunction >= 0x04 )
        {
            // Deterministic caches
            ParseCacheInfo(CacheInfo, CpuInfo.X86.MaxStandardFunction, 0x00000004);
        }
        //else
        //{
            // Legacy descriptors TLB/prefetch etc
            // ParseIntelLeaf2Descriptors(CacheInfo, &CpuInfo); // wtf why does this fault?
        //}
    }
}

