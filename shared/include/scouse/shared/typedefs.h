#if !defined( SHRD_TYPEDEFS_H ) && !defined( _STDTYPES ) && !defined( _WINDEF_ ) && !defined( _NTDEF_ ) && !defined( _BASETSD_H_ )
#define SHRD_TYPEDEFS_H	

//
// standard types and defines
//

#define STATIC static
#define CONST const
#define CONSTEXPR constexpr
#define VOLATILE volatile
#define EXTERN extern

#ifndef FORCEINLINE
#define FORCEINLINE __forceinline
#endif

#if defined( __cplusplus )
#define EXTERN_C extern "C"
#else
#define EXTERN_C extern
#endif
#define EXTERN extern
#define VOLATILE volatile

#define DLLEXPORT __declspec( dllexport )
#define DLLIMPORT __declspec( dllimport )
#define DECLSPEC_ALLOCATE( Name ) __declspec( allocate( Name ) )
#ifndef DECLSPEC_NOINLINE
#define DECLSPEC_NOINLINE __declspec( noinline )
#endif

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

#define max( a, b ) ( ( ( a ) > ( b ) ) ? ( a ) : ( b ) )
#define min( a, b ) ( ( ( a ) < ( b ) ) ? ( a ) : ( b ) )
#define offsetof(type, member) ((unsigned long long) &(((type*)0)->member))

#endif // !TYPEDEFS_H