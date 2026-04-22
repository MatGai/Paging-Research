#include <scouse/runtime/console.h>
#include <scouse/runtime/string.h>
#define STB_SPRINTF_NOFLOAT  
#define STB_SPRINTF_IMPLEMENTATION
#include <scouse/runtime/stbsprintf.h>

typedef struct _RT_STB_PRINT_CONTEXT
{
	CHAR    Buffer[STB_SPRINTF_MIN];
	ULONG64 Total;
} RT_STB_PRINT_CONTEXT, *PRT_STB_PRINT_CONTEXT;

static 
PSTR 
RtStbCallBack(
	PCSTR Buffer,
	PVOID User,
	LONG32 Length
)
{
	PRT_STB_PRINT_CONTEXT Context = (PRT_STB_PRINT_CONTEXT)User;

	if ( Buffer && Length > 0 )
	{
		SCSTATUS St = RtConsoleWrite(Buffer, Length);
		if ( SC_FAILED( St ) )
		{
			return (PSTR)NULL;
		}
		Context->Total += Length;
	}

	return Context->Buffer;
}

LONG32
printf(
	_In_ PCSTR Format,
	_In_ ...
)
{
	va_list Args;
	va_start( Args, Format );
	LONG32 Ret = vprintf( Format, Args );
	va_end( Args );
	return Ret;
}

LONG32
vprintf(
	_In_ PCSTR   Format,
	_In_ va_list Args
)
{
	if (!Format)
	{
		return -1;
	}

	RT_STB_PRINT_CONTEXT Context = { 0 };
	PSTR End = stbsp_vsprintfcb(
		RtStbCallBack,
		&Context,
		Context.Buffer,
		Format,
		Args
	);

	if( End > Context.Buffer )
	{
		ULONG64 Tail = (ULONG64)(End - Context.Buffer);
		SCSTATUS St = RtConsoleWrite(Context.Buffer, Tail);
		Context.Total += Tail;
	}

	return (LONG32)Context.Total;
}

LONG32
snprintf(
	_In_ PSTR    Buffer,
	_In_ LONG32  BufferSize,
	_In_ PCSTR   Format,
	_In_ ...
)
{
	va_list Args;
	va_start(Args, Format);
	LONG32 Ret = vsnprintf(Buffer, BufferSize, Format, Args);
	va_end(Args);
	return Ret;
}

LONG32
vsnprintf(
	_In_ PSTR    Buffer,
	_In_ LONG32  BufferSize,
	_In_ PCSTR   Format,
	_In_ va_list Args
)
{
	if( !Buffer || !BufferSize || !Format )
	{
		return -1;
	}

	LONG32 Ret = stbsp_vsnprintf(Buffer, BufferSize, Format, Args);
	return Ret;
}