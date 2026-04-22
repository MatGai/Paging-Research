#ifndef RT_CONSOLE_H
#define RT_CONSOLE_H

#include <scouse/shared/status.h>

typedef SCSTATUS(*RtWriteFn)(PVOID Context, PCSTR Buffer, LONG32 Length);

typedef struct _RT_CONSOLE_CONTEXT
{
	RtWriteFn Write;
	PVOID     Context;
} RT_CONSOLE_CONTEXT, *PRT_CONSOLE_CONTEXT;

VOID
RtConsoleUpdateContext(
	const PRT_CONSOLE_CONTEXT Context
);

SCSTATUS
RtConsoleWrite(
	PCSTR  Buffer,
	LONG32 Length
);

#endif