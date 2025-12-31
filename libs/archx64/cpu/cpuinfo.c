#include <scouse/shared/cpuinfo.h>
#include <scouse/archx64/intrinsics.h>

#define CPUID_EAX 0
#define CPUID_EBX 1
#define CPUID_ECX 2
#define CPUID_EDX 3

VOID
GetCpuInfo(
	_Out_ PCPUINFO CpuInfo
)
{
	if ( !CpuInfo )
	{
		return;
	}

	CpuInfo->Architecture = CPU_ARCHITECTURE_X86;

	PULONG32 VendorString = (PULONG32)CpuInfo->Vendor;
	PULONG32 BrandString  = (PULONG32)CpuInfo->Brand;

	_scouse_cpuid( 0x80000002, BrandString + 0 );
	_scouse_cpuid( 0x80000003, BrandString + 4 );
	_scouse_cpuid( 0x80000004, BrandString + 8 );

	CpuInfo->Brand[48] = '\0';

	ULONG32 Registers[ 4 ] = { 0 };

	_scouse_cpuid( 0x00000000, &CpuInfo->X86.Leaf0 );

	ULONG32 HsCall = CpuInfo->X86.MaxStandardFunction = CpuInfo->X86.Leaf0.Eax;

	VendorString[ 0 ] = CpuInfo->X86.Leaf0.Ebx;
	VendorString[ 1 ] = CpuInfo->X86.Leaf0.Edx;
	VendorString[ 2 ] = CpuInfo->X86.Leaf0.Ecx;
	CpuInfo->Vendor[12] = '\0';

	_scouse_cpuid( 0x00000001, &CpuInfo->X86.Leaf1 );

	CpuInfo->X86.Stepping = CpuInfo->X86.Leaf1.Eax & 0x0000000F;
	CpuInfo->X86.Model    = (ULONG32)( CpuInfo->X86.Leaf1.Eax & 0x000000F0 ) >> 4;
	CpuInfo->X86.Family   = (ULONG32)( CpuInfo->X86.Leaf1.Eax & 0x00000F00 ) >> 8;

	if( CpuInfo->X86.Family == 0x0F || CpuInfo->X86.Family == 0x06 )
	{
		CpuInfo->X86.Family |= (ULONG32)( ( Registers[ CPUID_EAX ] & 0x00FF00000 ) >> 16 );
		CpuInfo->X86.Model  |= (ULONG32)( ( Registers[ CPUID_EAX ] & 0x0000F0000 ) >> 12 );
	}

	CpuInfo->X86.Ecore  = FALSE;
	CpuInfo->X86.Hybrid = FALSE;

	if (CpuInfo->X86.MaxStandardFunction < 0x7)
	{
		return;
	}

	_scouse_cpuid(0x00000007, &CpuInfo->X86.Leaf7 );


	if( CpuInfo->X86.Leaf7.Ebx & ( (ULONG32)1 << 15 ) )
	{
		CpuInfo->X86.Hybrid = TRUE;

		_scouse_cpuid( 0x0000001A, Registers );

		ULONG32 CoreType = ( Registers[ CPUID_EAX ] & 0xFF000000 ) >> 24;
		if( CoreType == 0x20 )
		{
			CpuInfo->X86.Ecore = TRUE;
		}
	}
}
