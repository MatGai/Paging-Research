#ifndef ARCHX64_SPECIAL_H
#define ARCHX64_SPECIAL_H

#include <scouse/shared/typedefs.h>

#ifdef __cplusplus
extern "C"
{
#endif

    BOOLEAN
    _scouse_check_cpuid_support(
        VOID
    );

    BOOLEAN 
    _scouse_rdrand64_step(
        _Out_ ULONG64* Out
    );

    BOOLEAN 
    _scouse_rdrand64_retry(
        _Out_ ULONG64* Out,
        _In_  LONG Retries
    );

#ifdef __cplusplus
}
#endif

#endif // !ARCHX64_SPECIAL_H