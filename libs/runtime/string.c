#include <scouse/runtime/string.h>


PVOID
memcpy(
    PVOID dst,
    CONST PVOID src,
    ULONG64 sz
)
{
    PSTR destination  = (PSTR)dst;
    PCSTR source = (PCSTR)src;
    ULONG64 length = sz / sizeof( LONG32 );

    if( sz == 0 || dst == src )
    {
        return dst;
    }

    if( ( destination < source && destination + sz > source ) || 
        ( source < destination && source + sz > destination ) )
    {
        return NULL;
    }

    // copy per byte until destination is aligned to LONG32
    while (
        sz > 0 && ( (ULONG64)destination & ( sizeof( LONG32 ) - 1 ) ) 
        )
    {
        *destination++ = *source++;
        sz--;
    }


    // copy per 4 bytes (LONG32) 
    PLONG32 dword = (PLONG32)destination;
    CONST PLONG32 sword = (CONST PLONG32)source;

    for( ULONG64 i = 0; i < sz / sizeof( LONG32 ); i++ )
    {
        dword[i] = sword[i];
    }
    
    ULONG32 v = sz % sizeof( LONG32 );

    if (v != 0)
    {
        // copy remaining bytes
        destination = (PSTR)&dword[sz / sizeof(LONG32)];
        source = (PCSTR)&sword[sz / sizeof(LONG32)];

        for (ULONG64 i = 0; i < v; i++)
        {
            destination[i] = source[i];
        }
    }

    return dst;
}

PVOID
memset(
    PVOID dst,
    LONG32 v,
    ULONG64 sz
)
{
    if( sz == 0 )
    {
        return dst;
    }

    PBYTE destination = dst;
    do 
    {
        *destination++ = (BYTE)v;
    } while ( --sz != 0 );

    return dst;
}

LONG32
memcmp(
    CONST PVOID dst,
    CONST PVOID src,
    ULONG64 sz
)
{
    if (sz == 0)
    {
        return 0;
    }

    PBYTE p1 = dst;
    PBYTE p2 = src;

    for( ; sz != 0; --sz )
    {
        if( *p1++ != *p2++ )
        {
            return ( *--p1 - *--p2 );
        }
    }
}

PSTR
strchr(
    PCSTR src,
    LONG32 c
)
{
    for( ;; ++src )
    {
        if( *src == (CHAR)c )
        {
            return (PSTR)src;
        }
        if( !( *src ) )
        {
            return (PSTR)NULL;
        }
    }
}

LONG32
strcmp(
    PCSTR s0,
    PCSTR s1
)
{
    while( *s0 == *s1++ )
    {
        if( *s0++ == 0 )
        {
            return 0;
        }
    }
    return (*(PBYTE)s0 - *(PBYTE)--s1);
}

PSTR
strlcpy(
    PSTR dst,
    PCSTR src,
    ULONG64 sz
)
{
    PCSTR src0 = src;
    ULONG64 left = sz;

    if( left != 0 )
    {
        while( --left != 0 )
        {
            if( (*dst++ = *src++) == '\0' )
            {
                break;
            }
        }
    }

    if (left == 0)
    {
        if( sz != 0 )
        {
            *dst = '\0';
        }
        while( *src++ );
    }

    return (src - src0 - 1);
}

ULONG64
strlen(
    PCSTR str
)
{
    PCSTR str0;
    for (str0 = str; *str0; ++str0);
    return (str0 - str);
}

PSTR
strstr(
    PCSTR s0,
    PCSTR s1
)
{
	for (;; )
    {
        PCSTR p = s0;
        PCSTR q = s1;
        while( *q != '\0' && *p == *q )
        {
            ++p;
            ++q;
        }
        if( *q == '\0' )
        {
            return (PSTR)s0;
        }
        if( *s0 == '\0' )
        {
            return (PSTR)NULL;
        }
        ++s0;
    }
}