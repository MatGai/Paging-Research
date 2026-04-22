#ifndef SHRD_CPUINFO_H
#define SHRD_CPUINFO_H

#include <scouse/shared/typedefs.h>

#define CPU_ARCHITECTURE_X86   ( 0 )
#define CPU_ARCHITECTURE_ARM64 ( 1 )

typedef union _REGISTER_SET
{
	struct {
		ULONG32 Eax;
		ULONG32 Ebx;
		ULONG32 Ecx;
		ULONG32 Edx;
	};

    ULONG32 Registers[ 4 ];
} REGISTER_SET, * PREGISTER_SET;

typedef struct _CPUINFO_X86
{
	ULONG32 Family;
	ULONG32 Model;
	ULONG32 Stepping;

	// Max function returned by function 8000_0000 for extended and 0000_0000 for 
	// standard, in the eax register
	ULONG32 MaxStandardFunction;
	ULONG32 MaxExtendedFunction;

	REGISTER_SET Leaf0;   // root
    REGISTER_SET Leaf1;   // family, model, stepping, features
	REGISTER_SET Leaf2;   // Intel cache and tlb info
	REGISTER_SET Leaf7;	  // extended features
	REGISTER_SET Leaf7_1; // extended features

	REGISTER_SET Leaf80000000; // extended root
    REGISTER_SET Leaf80000001; // AMD extended features and cache
    REGISTER_SET Leaf80000002; // processor brand string part 1
    REGISTER_SET Leaf80000003; // processor brand string part 2
    REGISTER_SET Leaf80000004; // processor brand string part 3
    REGISTER_SET Leaf80000021; // AMD extended feature id 2

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
	};

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