#ifndef RT_RENDER_H
#define RT_RENDER_H


#include <scouse/shared/status.h>
#include <scouse/shared/bootinfo.h>

SCSTATUS
RtGfxInitContext(
    _In_ PBL_GOP_FRAMEBUFFER_DESCRIPTOR Context
);

SCSTATUS
RtGfxInitConsole(
    VOID
);

LONG32
rtputc(
    char c
);

#endif // !RT_REDNER_H
