#include <scouse/runtime/console.h>

static RT_CONSOLE_CONTEXT gRtConsoleContext = { 0 };

VOID
RtConsoleUpdateContext(
	const PRT_CONSOLE_CONTEXT Context
)
{
	if( !Context )
	{
		gRtConsoleContext = (RT_CONSOLE_CONTEXT){ 0 };
		return;
	}

	gRtConsoleContext = *Context;
}

SCSTATUS
RtConsoleWrite(
	PCSTR  Buffer,
	LONG32 Length
)
{
	if ( !Buffer || !Length )
	{
		return 0;
	}

	if (!gRtConsoleContext.Write)
	{
		return -5000;
	}

	return gRtConsoleContext.Write( gRtConsoleContext.Context, Buffer, Length );
}
