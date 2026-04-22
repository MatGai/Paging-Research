#define SSFN_CONSOLEBITMAP_TRUECOLOR   
#include <scouse/runtime/gfx/ssfn.h>

#include <scouse/runtime/gfx/render.h>
#include <scouse/runtime/gfx/vga16.h>

#include <scouse/runtime/console.h>

SCSTATUS
RtGfxInitContext(
    _In_ PBL_GOP_FRAMEBUFFER_DESCRIPTOR Context
)
{
    if (!Context || !Context->FrameBufferBase || !Context->ResolutionWidth || !Context->ResolutionHeight )
    {
        return SC_INVALID_PARAMETER;
    }

    ssfn_src     = (ssfn_font_t*)u_vga16_sfn;
    ssfn_dst.ptr = (PVOID)Context->FrameBufferBase;
    ssfn_dst.w   = (LONG32)Context->ResolutionWidth;
    ssfn_dst.h   = (LONG32)Context->ResolutionHeight;
    ssfn_dst.p   = (UINT16)Context->BytesPerScanLine;
    ssfn_dst.x   = 0;
    ssfn_dst.y   = 0;

    ssfn_dst.fg  = 0xFFFFFFFF;
    ssfn_dst.bg  = 0x0;

    return SC_OK;
}

static 
SCSTATUS 
RtSsfWrite(
    _In_ PVOID Context, 
    _In_ PCSTR Buffer, 
    _In_ LONG32 Length
)
{
    (VOID)Context; // to stop msvc crying
    if (!Buffer)
    {
        return -1;
    }

    for( ULONG64 Index = 0; Index < Length; ++Index )
    {
        BYTE Character = (BYTE)Buffer[Index];

        switch (Character)
        {
            case '\r':
            {
                ssfn_dst.x = 0;
                break;
            }
            case '\n':
            {
                ssfn_dst.x = 0;
                ssfn_dst.y += 16;
                break;
            }
            case '\t':
            {
                for (int k = 0; k < 4; ++k) 
                {
                    ssfn_putc((ULONG32)' ');
                }
                break;
            }
            default:
            {
                ssfn_putc((ULONG32)Character);
                break;
            }
        }
    }

    return SC_OK;
}

LONG32
rtputc(
    char c
)
{
    return ssfn_putc((ULONG32)c);
}

SCSTATUS
RtGfxInitConsole(
    VOID
)
{
    static const RT_CONSOLE_CONTEXT Context = { RtSsfWrite, NULL };
    RtConsoleUpdateContext(&Context);
    return SC_OK;
}

