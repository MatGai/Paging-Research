#ifndef TYPEDEFS_H	
#define TYPEDEFS_H	

//
// standard types and defines
//

#include <stdint.h>

#define CONST const
#define CONSTEXPR constexpr
#define VOLATILE volatile
#define EXTERN extern


typedef signed char INT8, * PINT8;
typedef unsigned char UINT8, * PUINT8;
typedef signed short INT16, *PINT16;
typedef unsigned short UINT16, * PUINT16;

typedef signed int LONG, *PLONG;
typedef unsigned int ULONG, *PULONG;
typedef LONG LONG32, * PLONG32;
typedef ULONG ULONG32, * PULONG32;

typedef char CHAR, *PCHAR, *PSTR;
typedef CONST CHAR* LPCSTR, *PCSTR;

typedef unsigned char BYTE, *PBYTE;


typedef unsigned short WCHAR, *PWCHAR, *PWSTR;
typedef CONST PWCHAR LPCWSTR, PCWSTR;

typedef signed long long LONG64, *PLONG64;

typedef unsigned long long ULONG64, *PULONG64;

typedef unsigned short WORD, *PWORD;

typedef unsigned char BOOLEAN;

#undef TRUE
#undef FALSE

#ifndef TRUE
#define TRUE ((BOOLEAN)1)
#endif
#ifndef FALSE
#define FALSE ((BOOLEAN)0)
#endif

#undef VOID
#ifndef VOID
#define VOID void
#endif

typedef void* PVOID;

#ifndef NULL
#define NULL ((PVOID)0)
#endif

#define MAXULONG32  ((ULONG32)~((ULONG32)0))
#define MAXLONG32   ((LONG32)(MAXULONG32 >> 1))
#define MINLONG32   ((LONG32)~MAXLONG32)

#define MAXULONG64  ((ULONG64)~((ULONG64)0))
#define MAXLONG64   ((LONG64)(MAXULONG64 >> 1))
#define MINLONG64   ((LONG64)~MAXLONG64)

#ifndef _In_ 
#define _In_
#endif
#ifndef _In_opt_
#define _In_opt_
#endif
#ifndef _Inout_
#define _Inout_
#endif
#ifndef _Inout_opt_
#define _Inout_opt_
#endif
#ifndef _Out_
#define _Out_
#endif
#ifndef _Out_opt_
#define _Out_opt_
#endif
#define offsetof(type, member) ((unsigned long long) &(((type*)0)->member))

#endif // !TYPEDEFS_H