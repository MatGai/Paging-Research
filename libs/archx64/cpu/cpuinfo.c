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

	_scouse_cpuid( 0x00000000, Registers );

	ULONG32 HsCall = CpuInfo->u.X86.MaxStandardFunction = Registers[ CPUID_EAX ];

	VendorString[ 0 ] = Registers[ CPUID_EBX ];
	VendorString[ 1 ] = Registers[ CPUID_EDX ];
	VendorString[ 2 ] = Registers[ CPUID_ECX ];
	CpuInfo->Vendor[48] = '\0';

	_scouse_cpuid( 0x00000001, Registers );

	CpuInfo->u.X86.Leaf1Ecx = Registers[ CPUID_ECX ];
	CpuInfo->u.X86.Leaf1Edx = Registers[ CPUID_EDX ];
	CpuInfo->u.X86.Stepping = Registers[ CPUID_EAX ] & 0x0000000F;
	CpuInfo->u.X86.Model    = (ULONG32)( Registers[ CPUID_EAX ] & 0x000000F0 ) >> 4;
	CpuInfo->u.X86.Family   = (ULONG32)( Registers[ CPUID_EAX ] & 0x00000F00 ) >> 8;

	if( CpuInfo->u.X86.Family == 0x0F || CpuInfo->u.X86.Family == 0x06 )
	{
		CpuInfo->u.X86.Family |= (ULONG32)( ( Registers[ CPUID_EAX ] & 0x00FF00000 ) >> 16 );
		CpuInfo->u.X86.Model  |= (ULONG32)( ( Registers[ CPUID_EAX ] & 0x0000F0000 ) >> 12 );
	}

	CpuInfo->u.X86.Ecore  = FALSE;
	CpuInfo->u.X86.Hybrid = FALSE;

	if (CpuInfo->u.X86.MaxStandardFunction < 0x7)
	{
		return;
	}

	_scouse_cpuid(0x00000007, Registers);

	CpuInfo->u.X86.Leaf7Ebx = Registers[ CPUID_EBX ];
	CpuInfo->u.X86.Leaf7Ecx = Registers[ CPUID_ECX ];
	CpuInfo->u.X86.Leaf7Edx = Registers[ CPUID_EDX ];

	if( Registers[ CPUID_EBX ] & ( (ULONG32)1 << 15 ) )
	{
		CpuInfo->u.X86.Hybrid = TRUE;

		_scouse_cpuid( 0x0000001A, Registers );

		ULONG32 CoreType = ( Registers[ CPUID_EAX ] & 0xFF000000 ) >> 24;
		if( CoreType == 0x20 )
		{
			CpuInfo->u.X86.Ecore = TRUE;
		}
	}
}
