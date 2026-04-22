#include "gop.h"

static EFI_GRAPHICS_OUTPUT_PROTOCOL* UefiGop = (EFI_GRAPHICS_OUTPUT_PROTOCOL*)NULL;
static EFI_GUID GopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;

SCSTATUS
BlGopInit(
    VOID
)
{
    EFI_STATUS St = gBS->LocateProtocol( &GopGuid, NULL, (PVOID*)&UefiGop );
    if( EFI_ERROR( St ) )
    {
        return (SCSTATUS)St;
    }

    return SC_OK;
}

SCSTATUS
BlGopInfo(
    _Out_ ULONG32* Mode,
    _Out_ EFI_GRAPHICS_OUTPUT_MODE_INFORMATION** Info,
    _In_  ULONG64* Size
)
{
    if( !UefiGop )
    {
        return SC_GOP_UNINITIALISED;
    }

    if (!Mode || !Info || !Size)
    {
        return SC_INVALID_PARAMETER;
    }

    *Mode = UefiGop->Mode->Mode;
    EFI_STATUS St = UefiGop->QueryMode(UefiGop, *Mode, Size, Info);
    if (EFI_ERROR(St))
    {
        return (SCSTATUS)St;
    }

    return SC_OK;
}

SCSTATUS
BlGopModeInfo(
    _In_  ULONG32 Mode,
    _Out_ EFI_GRAPHICS_OUTPUT_MODE_INFORMATION** Info,
    _In_  ULONG64* Size
)
{
    if (!UefiGop)
    {
        return SC_GOP_UNINITIALISED;
    }

    if( !Info || !Size )
    {
        return SC_INVALID_PARAMETER;
    }

    EFI_STATUS St = UefiGop->QueryMode( UefiGop, Mode, Size, Info );
    if( EFI_ERROR( St ))
    {
        return (SCSTATUS)St;
    }

    return SC_OK;
}

SCSTATUS
BlGopMaxMode(
    _Out_ ULONG32* MaxMode
)
{
    if (!UefiGop)
    {
        return SC_GOP_UNINITIALISED;
    }

    if( !MaxMode )
    {
        return SC_INVALID_PARAMETER;
    }
    *MaxMode = UefiGop->Mode->MaxMode;

    return SC_OK;
}

SCSTATUS
BlGopSetMode(
    ULONG32 Mode
)
{

    if (!UefiGop)
    {
        return SC_GOP_UNINITIALISED;
    }

    if (Mode == MAXULONG32)
    {
        Mode = UefiGop->Mode->Mode;
    }

    EFI_STATUS St = UefiGop->SetMode(UefiGop, Mode);
    if (EFI_ERROR(St))
    {
        return (SCSTATUS)St;
    }

    return SC_OK;
}

SCSTATUS
BlGopPrepareGraphics(
    _In_  ULONG32 Mode,
    _Out_ PBL_GOP_FRAMEBUFFER_DESCRIPTOR Descriptor
)
{
    if( !UefiGop )
    {
        return SC_GOP_UNINITIALISED;
    }

    if ( !Descriptor )
    {
        return SC_INVALID_PARAMETER;
    }

    SCSTATUS St = 0;

    if( UefiGop->Mode->Mode != Mode )
    {
        St = BlGopSetMode( Mode );
        if( SC_FAILED( St ) )
        {
            return St;
        }
    }

    gST->ConOut->SetCursorPosition(gST->ConOut, 0, 0);

    EFI_GRAPHICS_OUTPUT_BLT_PIXEL Pixel = { 0 };
    Pixel.Blue  = 0x0;
    Pixel.Green = 0x0;
    Pixel.Red   = 0x0;

    St = UefiGop->Blt(
            UefiGop, &Pixel, EfiBltVideoFill, 
            0, 0, 0, 0, 
            UefiGop->Mode->Info->HorizontalResolution, 
            UefiGop->Mode->Info->VerticalResolution, 
            0
        );

    if( SC_FAILED( St ) )
    {
        return St;
    }

    Descriptor->FrameBufferBase  = (ULONG64)UefiGop->Mode->FrameBufferBase;
    Descriptor->FrameBufferSize  = (ULONG64)UefiGop->Mode->FrameBufferSize;
    Descriptor->BytesPerScanLine = (UINT16)(UefiGop->Mode->Info->PixelsPerScanLine * 4); // turn pixels into bytes...
    Descriptor->ResolutionHeight = (LONG32)UefiGop->Mode->Info->VerticalResolution;
    Descriptor->ResolutionWidth  = (LONG32)UefiGop->Mode->Info->HorizontalResolution;

    //Print(L"Buffer 0x%llx, Size 0x%llx, BytesPerLine %d, Resz %d x %d\n", Descriptor->FrameBufferBase, Descriptor->FrameBufferSize, Descriptor->BytesPerScanLine, Descriptor->ResolutionHeight, Descriptor->ResolutionWidth);
    
    switch( UefiGop->Mode->Info->PixelFormat )
    {
        case PixelRedGreenBlueReserved8BitPerColor:
        {
            Descriptor->RedMask   = 0x000000FF;
            Descriptor->GreenMask = 0x0000FF00;
            Descriptor->BlueMask  = 0x00FF0000;
            Descriptor->ResvMask  = 0xFF000000;
            //Print(L"RGB\n");
            break;
        }
        case PixelBlueGreenRedReserved8BitPerColor:
        {
            Descriptor->BlueMask  = 0x000000FF;
            Descriptor->GreenMask = 0x0000FF00;
            Descriptor->RedMask   = 0x00FF0000;
            Descriptor->ResvMask  = 0xFF000000;
            //Print(L"BGR\n");
            break;
        }
        case PixelBitMask:
        {
            Descriptor->RedMask   = UefiGop->Mode->Info->PixelInformation.RedMask;
            Descriptor->GreenMask = UefiGop->Mode->Info->PixelInformation.GreenMask;
            Descriptor->BlueMask  = UefiGop->Mode->Info->PixelInformation.BlueMask;
            Descriptor->ResvMask  = UefiGop->Mode->Info->PixelInformation.ReservedMask;
            //Print(L"BitMask\n");
            break;
        }
        default:
        {
            //Print(L"UNSUPPORTED?!\n");
            return SC_UNSUPPORTED;
        }
    }

    return SC_OK;
}