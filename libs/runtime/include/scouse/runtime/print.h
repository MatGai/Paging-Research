#ifndef RT_PRINT_H
#define RT_PRINT_H

#include <stdarg.h>
#include <scouse/shared/typedefs.h>

LONG32
printf(
	_In_ PCSTR Format,
	_In_ ...
);

LONG32
vprintf(
	_In_ PCSTR   Format,
	_In_ va_list Args
);

LONG32
snprintf(
	_In_ PSTR    Buffer,
	_In_ LONG32  BufferSize,
	_In_ PCSTR   Format,
	_In_ ...
);

LONG32
vsnprintf(
	_In_ PSTR    Buffer,
	_In_ LONG32  BufferSize,
	_In_ PCSTR   Format,
	_In_ va_list Args
);

#endif // !RT_PRINT_H
