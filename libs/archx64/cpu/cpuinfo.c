#include <scouse/shared/cpuinfo.h>
#include <scouse/archx64/intrinsics.h>
#include <scouse/runtime/string.h>

#define CPUID_EAX 0
#define CPUID_EBX 1
#define CPUID_ECX 2
#define CPUID_EDX 3

VOID
GetCpuInfo(
    _Out_ PCPUINFO CpuInfo
)
{
    if (!CpuInfo)
    {
        return;
    }

    memset(CpuInfo, 0, sizeof(*CpuInfo));

    CpuInfo->Architecture = CPU_ARCHITECTURE_X86;

    // Standard leaf 0 vendor & max standard
    _scouse_cpuid(0x00000000, (ULONG32*)&CpuInfo->X86.Leaf0);
    CpuInfo->X86.MaxStandardFunction = CpuInfo->X86.Leaf0.Eax;

    // Vendor string is EBX, EDX, ECX (12 bytes)
    PULONG32 V = (PULONG32)CpuInfo->Vendor;
    V[0] = CpuInfo->X86.Leaf0.Ebx;
    V[1] = CpuInfo->X86.Leaf0.Edx;
    V[2] = CpuInfo->X86.Leaf0.Ecx;
    CpuInfo->Vendor[12] = '\0';

    // Extended leaf 0x80000000
    _scouse_cpuid(0x80000000, (ULONG32*)&CpuInfo->X86.Leaf80000000);
    CpuInfo->X86.MaxExtendedFunction = CpuInfo->X86.Leaf80000000.Eax;

    // Leaf 1 - family/model/stepping
    if (CpuInfo->X86.MaxStandardFunction >= 0x00000001)
    {
        _scouse_cpuid(0x00000001, (ULONG32*)&CpuInfo->X86.Leaf1);

        ULONG32 Sig = CpuInfo->X86.Leaf1.Eax;
        ULONG32 Stepping = (Sig & 0xF);
        ULONG32 Model = (Sig >> 4) & 0xF;
        ULONG32 Family = (Sig >> 8) & 0xF;
        ULONG32 ExtModel = (Sig >> 16) & 0xF;
        ULONG32 ExtFamily = (Sig >> 20) & 0xFF;

        if (Family == 0x0F) Family = Family + ExtFamily;
        if (Family == 0x06 || Family == 0x0F) Model = (ExtModel << 4) | Model;

        CpuInfo->X86.Stepping = Stepping;
        CpuInfo->X86.Model = Model;
        CpuInfo->X86.Family = Family;
    }

    // Leaf 2 
    if (CpuInfo->X86.MaxStandardFunction >= 0x00000002)
    {
        _scouse_cpuid(0x00000002, (ULONG32*)&CpuInfo->X86.Leaf2);
    }

    // Leaf 7 structured features
    CpuInfo->X86.Hybrid = FALSE;
    CpuInfo->X86.Ecore = FALSE;

    if (CpuInfo->X86.MaxStandardFunction >= 0x00000007)
    {
        _scouse_cpuidex(0x00000007, 0, (ULONG32*)&CpuInfo->X86.Leaf7);

        if (CpuInfo->X86.Leaf7.Eax >= 1)
        {
            _scouse_cpuidex(0x00000007, 1, (ULONG32*)&CpuInfo->X86.Leaf7_1);
        }

        // Hybrid flag on Intel is leaf7:EDX[15]
        if (CpuInfo->X86.Leaf7.Edx & ((ULONG32)1u << 15))
        {
            CpuInfo->X86.Hybrid = TRUE;

            if (CpuInfo->X86.MaxStandardFunction >= 0x0000001A)
            {
                REGISTER_SET Leaf1A = { 0 };
                _scouse_cpuid(0x0000001A, (ULONG32*)&Leaf1A);

                ULONG32 CoreType = (Leaf1A.Eax >> 24) & 0xFF;
                if (CoreType == 0x20)
                    CpuInfo->X86.Ecore = TRUE;
            }
        }
    }

    // Extended feature leaves
    if (CpuInfo->X86.MaxExtendedFunction >= 0x80000001)
    {
        _scouse_cpuid(0x80000001, (ULONG32*)&CpuInfo->X86.Leaf80000001);
    }
    if (CpuInfo->X86.MaxExtendedFunction >= 0x80000021)
    {
        _scouse_cpuid(0x80000021, (ULONG32*)&CpuInfo->X86.Leaf80000021);
    }

    // Brand string
    if (CpuInfo->X86.MaxExtendedFunction >= 0x80000004)
    {
        PULONG32 Brand = (PULONG32)CpuInfo->Brand;
        _scouse_cpuid(0x80000002, Brand + 0);
        _scouse_cpuid(0x80000003, Brand + 4);
        _scouse_cpuid(0x80000004, Brand + 8);
        CpuInfo->Brand[48] = '\0';
    }
    else
    {
        CpuInfo->Brand[0] = '\0';
    }
}