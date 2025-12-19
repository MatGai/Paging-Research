#ifndef ARCHX64_CPUID_H
#define ARCHX64_CPUID_H

#include <scouse/shared/typedefs.h>
#include <scouse/archx64/intrinsics.h>

typedef struct _CPUID_REGS
{
	union 
	{
		ULONG32 Registers[ 4 ];
		struct 
		{
			ULONG32 Eax;
			ULONG32 Ebx;
			ULONG32 Ecx;
			ULONG32 Edx;
		};
	}u;
} CPUID_REGS, *PCPUID_REGS;

VOID
GetCpuid(
	_In_  ULONG32 Feature,
	_Out_ PCPUID_REGS Registers
);

VOID
GetCpuidEx(
	_In_  ULONG32 Feature,
	_Out_ PCPUID_REGS Registers
);





#endif // !ARCHX64_CPUID_H