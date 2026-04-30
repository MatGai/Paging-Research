#include <scouse/runtime/string.h>


PVOID
memcpy(
    PVOID dst,
    CONST PVOID src,
    ULONG64 sz
)
{
    UINT8* d = (UINT8*)dst;
    CONST UINT8* s = (CONST UINT8*)src;

    while (sz--)
    {
        *d++ = *s++;
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

    return 0;
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

    return (PSTR)(src - src0 - 1);
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