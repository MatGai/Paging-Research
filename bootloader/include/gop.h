#ifndef BL_GOP_H
#define BL_GOP_H

#include <boot.h>
#include <scouse/shared/status.h>
#include <scouse/shared/bootinfo.h>

SCSTATUS
BlGopInit(
    VOID
);

SCSTATUS
BlGopInfo(
    _Out_ ULONG32* Mode,
    _Out_ EFI_GRAPHICS_OUTPUT_MODE_INFORMATION** Info,
    _In_  ULONG64* Size
);

SCSTATUS
BlGopModeInfo(
    _In_  ULONG32 Mode,
    _Out_ EFI_GRAPHICS_OUTPUT_MODE_INFORMATION** Info,
    _In_  ULONG64* Size
);

SCSTATUS
BlGopMaxMode(
    _Out_ ULONG32* MaxMode
);

SCSTATUS
BlGopSetMode(
    _In_ ULONG32 Mode
);

SCSTATUS
BlGopPrepareGraphics(
    _In_  ULONG32 Mode,
    _Out_ PBL_GOP_FRAMEBUFFER_DESCRIPTOR Descriptor
);

#endif // !BL_GOP_H
