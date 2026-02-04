
#include <scouse/runtime/string.h>

typedef ULONG64 (*__WriteConsole)( PWCHAR );

ULONG32 
ConsoleWrite(
	PWCHAR str
)
{
	/*return &str;*/
	return (ULONG32)0;
}

ULONG64 
putc(
	__WriteConsole WriteConsole,
	WCHAR character

)
{
	WCHAR tmp[2] = { character, '\0' };

	//if (character == L'\n')
	//{
	//	//sputc( WriteConsole, L'\r' );
	//}

	if (WriteConsole)
	{
		return WriteConsole( tmp );
	}

	return 0;
}

ULONG64
printf(
	unsigned __int16* Format,
	...
)
{
	/*va_list args;*/
	return (ULONG64)0;

}


VOID
print(

)
{
	//sputc(ConsoleWrite, L"A");
	return;
}