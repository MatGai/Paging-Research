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

#ifdef __cplusplus
}
#endif

#endif // !ARCHX64_SPECIAL_H