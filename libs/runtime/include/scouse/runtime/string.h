#ifndef RT_STRING_H
#define RT_STRING_H

#include <scouse/shared/typedefs.h>

#ifdef __cplusplus
extern "C" {
#endif

static_assert( sizeof( LONG32 ) == 4, "Long is not 4 bytes");

//
//    MEMORY  
//

PVOID 
memcpy(
    PVOID dst,
    CONST PVOID src,
    ULONG64 sz
);

PVOID
memset(
    PVOID dst,
    LONG32 v,
    ULONG64 sz
);

LONG32
memcmp(
    CONST PVOID dst,
    CONST PVOID src,
    ULONG64 sz
);

//
//   STRING  
//

PSTR
strchr(
    PCSTR src,
    LONG32 c
);

LONG32
strcmp(
    PCSTR s0,
    PCSTR s1
);

PSTR
strlcpy(
    PSTR dst,
    PCSTR src,
    ULONG64 sz
);

ULONG64
strlen(
    PCSTR str
);

PSTR
strstr(
    PCSTR s0,
    PCSTR s1
);

#ifdef __cplusplus
}
#endif

#endif // !RT_STRING_H