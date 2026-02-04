#ifndef ARCHX64_INTRINSICS_H
#define ARCHX64_INTRINSICS_H

#include <scouse/shared/typedefs.h>

#ifdef __cplusplus
extern "C" {
#endif

	VOID
	_scouse_cpuid(
		ULONG32, 
		ULONG32[4]
	);

	VOID
	_scouse_cpuidex(
		ULONG32,  
		ULONG32,  
		ULONG32[4]
	);

	ULONG64
	_scouse_readmsr(
		ULONG32,
		ULONG32*
	);

	ULONG64
	_scouse_readcr0(
		VOID
	);

	ULONG64
	_scouse_writecr0(
		ULONG64
	);

	ULONG64
	_scouse_readcr2(
		VOID
	);

	ULONG64
	_scouse_writecr2(
		ULONG64
	);

	ULONG64
	_scouse_readcr3(
		VOID
	);

	ULONG64
	_scouse_writecr3(
		ULONG64
	);

	ULONG64
	_scouse_readcr4(
		VOID
	);

	ULONG64
	_scouse_writecr4(
		VOID
	);

	ULONG64	
	 _scouse_readeflags(
		VOID
	);

	VOID
	_scouse_writeeflags(
		ULONG64
	);

	VOID
	_scouse_debugbreak(
		VOID
	);

#ifdef __cplusplus
}
#endif

#endif // !ARCHX64_INTRINSICS_H