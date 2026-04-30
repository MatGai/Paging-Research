#include "image.h"
#include "pe.h"
#include "filesystem.h"

EFI_STATUS
BLAPI
BlLdrLoadPEImageFile(
    _In_ PCWSTR ImagePath,
    _Inout_ PBL_LDR_FILE_IMAGE FileImageData
)
{
    EFI_STATUS LastError = ( EFI_STATUS )NULL;

    if( ImagePath == NULL || FileImageData == NULL )
    {
        return EFI_INVALID_PARAMETER;
    }

    EFI_FILE_PROTOCOL* ImageFileHandle = NULL;
    LastError = BlFindFile( ImagePath, &ImageFileHandle );
    if( EFI_ERROR( LastError ) )
    {
        //ImageFileHandle->Close( ImageFileHandle );
        return LastError;
    }

    EFI_FILE_INFO* ImageFileInfo = NULL;
    LastError = BlGetFileInfo( ImageFileHandle, &ImageFileInfo );
    if( EFI_ERROR( LastError ) )
    {
        ImageFileHandle->Close( ImageFileHandle );
        return LastError;
    }

    UINTN FileImageSize = ImageFileInfo->FileSize;

    gBS->FreePool( ImageFileInfo );

    PBYTE FileImage = NULL;
    LastError = gBS->AllocatePool( EfiBootServicesData, FileImageSize, &FileImage );
    if( EFI_ERROR( LastError ) )
    {
        ImageFileHandle->Close( ImageFileHandle );
        return LastError;
    }

    LastError = ImageFileHandle->Read(ImageFileHandle, &FileImageSize, FileImage);
    if (EFI_ERROR(LastError))
    {
        gBS->FreePool(FileImage);
        ImageFileHandle->Close(ImageFileHandle);
        return LastError;
    }

    ImageFileHandle->Close( ImageFileHandle );
    FileImageData->File = FileImage;
    FileImageData->FileSize = FileImageSize;

    return EFI_SUCCESS;
}

EFI_STATUS
BLAPI
BlLdrAllocatePEImagePages(
    _In_ PBL_LDR_FILE_IMAGE FileImage,
    _Inout_ PBYTE* ImagePages,
    _Out_ EFI_PHYSICAL_ADDRESS* ImagePagesPhysical
)
{
    if (FileImage == NULL || ImagePagesPhysical == NULL || ImagePages == NULL)
    {
        return EFI_INVALID_PARAMETER;
    }

    PEFI_IMAGE_NT_HEADERS FileNtHeaders = EFI_IMAGE_NTHEADERS(FileImage->File);
    ULONG64 Pages = EFI_SIZE_TO_PAGES(FileNtHeaders->OptionalHeader.SizeOfImage);
    //*ImagePagesPhysical = FileNtHeaders->OptionalHeader.ImageBase;

    EFI_PHYSICAL_ADDRESS ImageBase = FileNtHeaders->OptionalHeader.ImageBase;

    //
    // try to allocate at prefered base
    // Edit: realistically whilst this is happening in UEFI, we use direct addressing. Trying to map a phyical
    // address to preferred virtual base is unrealistic so just allocate anywhere on physical!
    //
    //gBS->AllocatePages(AllocateAnyPages/*AllocateAddress*/, EfiBootServicesCode, Pages, ImagePagesPhysical);
    EFI_STATUS PageStatus = gBS->AllocatePages(AllocateAnyPages/*AllocateAddress*/, EfiBootServicesCode, Pages, &ImageBase);

    if (EFI_ERROR(PageStatus))
    {
        if (PageStatus == EFI_NOT_FOUND)
        {

            //
            // I guess we can try prefered?
            //
            PageStatus = gBS->AllocatePages(/*AllocateAnyPages*/AllocateAddress, EfiBootServicesCode, Pages, &ImageBase);

            if (EFI_ERROR(PageStatus))
            {
                //
                // we failed to allocate memory for the image
                //
                return PageStatus;
            }
        }
        else
        {
            return PageStatus;
        }
    }

    // not really needed...Index hope...
    //ZeroMem(
    //	*ImagePagesPhysical,
    //	(Pages << EFI_PAGE_SHIFT)
    //);

    CopyMem(
        (PVOID)(UINTN)ImageBase,
        (PVOID)FileImage->File,
        FileNtHeaders->OptionalHeader.SizeOfHeaders
    );

    //
    // at this stage memory is identity mapped so the 
    // virtual address will be the same as the physical
    //
    *ImagePagesPhysical = ImageBase;
    *ImagePages = (PBYTE)(UINTN)ImageBase;

    return EFI_SUCCESS;
}

EFI_STATUS
BLAPI
BlLdrLoadPEImage64(
    _In_ PCWSTR ImagePath,
    _Inout_ PBL_LDR_LOADED_IMAGE_INFO LoadedImageInfo
)
{
    if( ImagePath == NULL || LoadedImageInfo == NULL )
    {
        return EFI_INVALID_PARAMETER;
    }

    EFI_STATUS LastError = ( EFI_STATUS )NULL;

    BL_LDR_FILE_IMAGE FileImage;
    //EfiZeroMemory(&FileImage, sizeof(BL_LDR_FILE_IMAGE));

    LastError = BlLdrLoadPEImageFile( ImagePath, &FileImage );
    if( EFI_ERROR( LastError ) )
    {
        return LastError;
    }

    if( PeIsValidImage( FileImage.File ) == FALSE )
    {
        return BL_STATUS_INVALID_PE_IMAGE;
    }

    PBYTE Image = NULL;
    EFI_PHYSICAL_ADDRESS ImagePhysical = ( EFI_PHYSICAL_ADDRESS )NULL;
    LastError = BlLdrAllocatePEImagePages( &FileImage, &Image, &ImagePhysical );
    if( EFI_ERROR( LastError ) )
    {
        return LastError;
    }

    if( PeIsValidImage( Image ) == FALSE )
    {
        return BL_STATUS_INVALID_PE_IMAGE;
    }

    if( BL_ERROR( BlLdrAlignFileImage( &FileImage, Image ) ) )
    {
        return BL_STATUS_GENERIC_ERROR;
    }

    EFI_IMAGE_DOS_HEADER* ImageDosHeader = (EFI_IMAGE_DOS_HEADER*)Image;
    EFI_IMAGE_NT_HEADERS* ImageNtHeaders = (EFI_IMAGE_NT_HEADERS*)((ULONG64)Image + ImageDosHeader->e_lfanew);

    if( BL_ERROR( BlLdrImageRelocation( Image, ImageNtHeaders->OptionalHeader.ImageBase ) ) )
    {
        return BL_STATUS_GENERIC_ERROR;
    }

    LoadedImageInfo->Base        = ( ULONG64 )Image;
    LoadedImageInfo->EntryPoint  = ( ULONG64 )ImageNtHeaders->OptionalHeader.AddressOfEntryPoint;
    LoadedImageInfo->VirtualBase = ( ULONG64 )ImageNtHeaders->OptionalHeader.ImageBase;
    LoadedImageInfo->Size        = ( ULONG64 )ImageNtHeaders->OptionalHeader.SizeOfImage;

    return BL_STATUS_OK;
}

EFI_STATUS
BLAPI
BlLdrAlignFileImage(
    _In_    PBL_LDR_FILE_IMAGE FileImage,
    _Inout_ PBYTE Image
)
{
    if( !Image )
    {
        return BL_STATUS_INVALID_PARAMETER;
    }

    if( PeIsValidImage( Image ) == FALSE )
    {
        return BL_STATUS_INVALID_PE_IMAGE;
    }

    EFI_IMAGE_DOS_HEADER* ImageDosHeader = ( EFI_IMAGE_DOS_HEADER* )Image;
    EFI_IMAGE_NT_HEADERS* ImageNtHeaders = ( EFI_IMAGE_NT_HEADERS* )( Image + ImageDosHeader->e_lfanew );

    //
    // use the IMAGE_FIRST_SECTION macro to get the pointer to the first section
    //
    EFI_IMAGE_SECTION_HEADER* FileSectionHeader = EFI_IMAGE_FIRST_SECTION( ImageNtHeaders );

    //
    // loop through each section and copy its raw data to the appropriate offset
    // in the new memory buffer based on the sections virtual address
    //
    for( ULONG64 i = 0; i < ImageNtHeaders->FileHeader.NumberOfSections; i++ )
    {
        EFI_IMAGE_SECTION_HEADER* CurrentSection = &FileSectionHeader[ i ];

        PVOID SectionAddress = ( PVOID )( ULONG64 )( ( EFI_PHYSICAL_ADDRESS )Image + ( EFI_PHYSICAL_ADDRESS )CurrentSection->VirtualAddress );
        PVOID SectionSource = FileImage->File + CurrentSection->PointerToRawData;

        ULONG32 Raw = CurrentSection->SizeOfRawData;
        ULONG32 Virtual = CurrentSection->Misc.VirtualSize;

        if( Raw > 0 )
        {
            CopyMem(
                SectionAddress,
                SectionSource,
                CurrentSection->SizeOfRawData
            );
        }

        if( Virtual > Raw )
        {
            SetMem( ( UINT8* )SectionAddress + Raw, Virtual - Raw, 0 );
        }
    }

    return EFI_SUCCESS;
}

EFI_STATUS
BLAPI
BlLdrImageRelocation(
    _Inout_ PBYTE Image,
    _In_	ULONG64 RuntimeVirtual
)
{
    if (!Image)
    {
        return EFI_INVALID_PARAMETER;
    }

    if (!PeIsValidImage(Image))
    {
        return BL_STATUS_INVALID_PE_IMAGE;
    }

    EFI_IMAGE_DOS_HEADER* Dos = (EFI_IMAGE_DOS_HEADER*)Image;
    EFI_IMAGE_NT_HEADERS* Nt = (EFI_IMAGE_NT_HEADERS*)(Image + Dos->e_lfanew);

    if (Nt->OptionalHeader.NumberOfRvaAndSizes <= EFI_IMAGE_DIRECTORY_ENTRY_BASERELOC)
    {
        return EFI_SUCCESS;
    }

    ULONG64 PreferredBase = Nt->OptionalHeader.ImageBase;
    LONG64 Delta = (LONG64)RuntimeVirtual - (LONG64)PreferredBase;

    if (Delta == 0)
    {
        return EFI_SUCCESS;
    }

    EFI_IMAGE_DATA_DIRECTORY* RelocDir =
        &Nt->OptionalHeader.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_BASERELOC];

    if (RelocDir->VirtualAddress == 0 || RelocDir->Size == 0)
    {
        return EFI_SUCCESS;
    }

    EFI_IMAGE_BASE_RELOCATION* Block =
        (EFI_IMAGE_BASE_RELOCATION*)(Image + RelocDir->VirtualAddress);

    EFI_IMAGE_BASE_RELOCATION* End =
        (EFI_IMAGE_BASE_RELOCATION*)((UINT8*)Block + RelocDir->Size);

    while ((Block < End) && Block->SizeOfBlock)
    {
        UINT16* Entry = (UINT16*)((UINT8*)Block + EFI_IMAGE_SIZEOF_BASE_RELOCATION);
        ULONG64 Count = (Block->SizeOfBlock - EFI_IMAGE_SIZEOF_BASE_RELOCATION) / sizeof(UINT16);

        for (ULONG64 Index = 0; Index < Count; ++Index)
        {
            UINT16 Type   = Entry[Index] >> 12;
            UINT16 Offset = Entry[Index] & 0x0FFF;

            switch (Type)
            {
                case EFI_IMAGE_REL_BASED_ABSOLUTE:
                {
                    break;
                }
                case EFI_IMAGE_REL_BASED_DIR64:
                {
                    ULONG64* Fixup = (ULONG64*)(Image + Block->VirtualAddress + Offset);
                    *Fixup        = (ULONG64)((LONG64)(*Fixup) + Delta);
                    break;
                }
                default:
                {
                    return BL_STATUS_GENERIC_ERROR;
                }
            }
        }

        Block = (EFI_IMAGE_BASE_RELOCATION*)((UINT8*)Block + Block->SizeOfBlock);
    }

    return EFI_SUCCESS;
}