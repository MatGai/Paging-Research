#ifndef CPUINFO_H
#define CPUINFO_H

#include <scouse/shared/typedefs.h>

#define CPU_ARCHITECTURE_X86   ( 0 )
#define CPU_ARCHITECTURE_ARM64 ( 1 )

typedef struct _CPUINFO_X86
{
	ULONG32 Family;
	ULONG32 Model;
	ULONG32 Stepping;

	// Max function returned by function 8000_0000 for extended and 0000_0000 for 
	// standard, in the eax register
	ULONG32 MaxStandardFunction;
	ULONG32 MaxExtendedFunction;

	ULONG32 Leaf1Ecx;
	ULONG32 Leaf1Edx;

	ULONG32 Leaf7Ebx;
	ULONG32 Leaf7Ecx;
	ULONG32 Leaf7Edx;

	// For intel hybrid CPUs
	BOOLEAN Hybrid;
	BOOLEAN Ecore;

} CPUINFO_X86, * PCPUINFO_X86;

typedef struct _CPUINFO_ARM64
{
	ULONG32 TODO;
} CPUINFO_ARM64, * PCPUINFO_ARM64;

typedef struct _CPUINFO
{
	ULONG32 Architecture;
	
	CHAR Vendor[ 16 ];
	CHAR Brand[ 64 ];

	union
	{
		CPUINFO_X86 X86;
		CPUINFO_ARM64 Arm64;
	} u;

} CPUINFO, * PCPUINFO;

/*
* To be implemented by architecture
*/
VOID 
EXTERN 
GetCpuInfo(
	_Out_ PCPUINFO CpuInfo
);


#endif // !CPUINFO_H