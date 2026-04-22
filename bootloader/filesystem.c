#include "filesystem.h"

EFI_STATUS         FILE_SYSTEM_STATUS;

static EFI_GUID           __FileSystemProtoclGUID__ = SIMPLE_FILE_SYSTEM_PROTOCOL;
static PWSTR              CurrentDirectoryString;
static EFI_FILE_PROTOCOL* CurrentDirectory;
static EFI_HANDLE*        CurrentFileSystemHandle;
static EFI_LOADED_IMAGE_PROTOCOL* LoadedImage;

EFI_STATUS
BLAPI
BlInitFileSystem(
    VOID
)
{
    EFI_GUID LoadedImageProtocolGUID = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    FILE_SYSTEM_STATUS = gBS->HandleProtocol( gImageHandle, &LoadedImageProtocolGUID, ( VOID** )&LoadedImage );

    if( EFI_ERROR( FILE_SYSTEM_STATUS ) )
    {
        DBG_ERROR( FILE_SYSTEM_STATUS, L"Failed to get loaded image protocol" );
        return FILE_SYSTEM_STATUS;
    }

    return EFI_SUCCESS;
}

EFI_STATUS
BLAPI
BlGetRootDirectory(
    _Out_opt_ EFI_FILE_PROTOCOL** Directory
)
{
    if( !LoadedImage )
    {
        EFI_STATUS LastError = BlGetLastFileError( );
        DBG_ERROR( LastError, L"Loaded image was null, maybe failed to get it ? " );
        return LastError;
    }

    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* FileProtocol;
    FILE_SYSTEM_STATUS = gBS->HandleProtocol( LoadedImage->DeviceHandle, &__FileSystemProtoclGUID__, ( VOID** )&FileProtocol );

    if( EFI_ERROR( FILE_SYSTEM_STATUS ) )
    {
        DBG_ERROR( FILE_SYSTEM_STATUS, L"Could not get file protocol" );
        return FILE_SYSTEM_STATUS;
    }

    EFI_FILE_PROTOCOL* Root;
    FILE_SYSTEM_STATUS = FileProtocol->OpenVolume( FileProtocol, &Root );

    if( EFI_ERROR( FILE_SYSTEM_STATUS ) )
    {
        DBG_ERROR( FILE_SYSTEM_STATUS, L"Could not open the root directory" );
        return FILE_SYSTEM_STATUS;
    }

    CurrentDirectory = Root;

    if( Directory )
    {
        *Directory = Root;
    }

    return EFI_SUCCESS;
}

EFI_STATUS
BLAPI
BlGetRootDirectoryByIndex(
    _In_ FILE_SYSTEM Index,
    _Out_opt_ EFI_FILE_PROTOCOL** Directory
)
{
    EFI_HANDLE* FileSystemHandles;
    ULONG64 HandleCount = 0;

    // Get all handles to all file systems on this system
    FILE_SYSTEM_STATUS = gBS->LocateHandleBuffer( ByProtocol, &__FileSystemProtoclGUID__, NULL, &HandleCount, &FileSystemHandles );

    if( EFI_ERROR( FILE_SYSTEM_STATUS ) || !HandleCount )
    {
        return FILE_SYSTEM_STATUS;
    }

    // well this just shouldn't happen!
    if( ( ULONG64 )Index > HandleCount )
    {
        FreePool( FileSystemHandles );
        return FALSE;
    }

    // open the file system by handle index
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* FileProtocol;
    FILE_SYSTEM_STATUS = gBS->OpenProtocol( FileSystemHandles[ Index ], &__FileSystemProtoclGUID__, ( VOID** )&FileProtocol, gImageHandle, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL );

    if( EFI_ERROR( FILE_SYSTEM_STATUS ) || !FileProtocol )
    {
        FreePool( FileSystemHandles );
        return FILE_SYSTEM_STATUS;
    }

    // now actually get the directory!
    EFI_FILE_PROTOCOL* Root;
    FILE_SYSTEM_STATUS = FileProtocol->OpenVolume( FileProtocol, &Root );

    if( EFI_ERROR( FILE_SYSTEM_STATUS ) )
    {
        return FILE_SYSTEM_STATUS;
    }

    CurrentFileSystemHandle = FileSystemHandles[ Index ];
    CurrentDirectory = Root;

    if( Directory )
    {
        *Directory = Root;
    }

    return EFI_SUCCESS;
}

EFI_STATUS
BLAPI
BlOpenSubDirectory(
    _In_  EFI_FILE_PROTOCOL* BaseDirectory,
    _In_  PCWSTR Path,
    _Out_ EFI_FILE_PROTOCOL** OutDirectory
)
{
    if( !BaseDirectory || !Path || !OutDirectory )
    {
        FILE_SYSTEM_STATUS = EFI_INVALID_PARAMETER;
        return FILE_SYSTEM_STATUS;
    }

    EFI_FILE_PROTOCOL* NewDirectory = NULL;

    // passing 0 as attributes returns if it is directory or not
    FILE_SYSTEM_STATUS = BaseDirectory->Open(
        BaseDirectory,
        &NewDirectory,
        Path,
        EFI_FILE_MODE_READ,
        0
    );

    // this is not a directory!
    if( FILE_SYSTEM_STATUS != EFI_FILE_DIRECTORY )
    {
        DBG_ERROR( FILE_SYSTEM_STATUS, L"Passed in path '%s' is not a directory to open!!!!", Path );
        FILE_SYSTEM_STATUS = EFI_INVALID_PARAMETER;
        return FILE_SYSTEM_STATUS;
    }

    //pretty confident it is a directory now
    FILE_SYSTEM_STATUS = BaseDirectory->Open(
        BaseDirectory,
        &NewDirectory,
        Path,
        EFI_FILE_MODE_READ,
        EFI_FILE_DIRECTORY
    );

    if( EFI_ERROR( FILE_SYSTEM_STATUS ) )
    {
        DBG_ERROR( FILE_SYSTEM_STATUS, L"Failed to open '%s' as directory", Path );
        return FILE_SYSTEM_STATUS;
    }

    *OutDirectory = NewDirectory;
    return EFI_SUCCESS;
}

EFI_STATUS
BLAPI
BlFindFile(
    _In_ PCWSTR File,
    _Out_ EFI_FILE_PROTOCOL** OutFile
)
{
    if( File == NULL )
    {
        return FALSE;
    }

    // Make sure we have a CurrentDirectory
    if( !CurrentDirectory )
    {
        // default to fs0: root
        BlGetRootDirectory( NULL );

        // if this happens then...well...
        if( !CurrentDirectory )
        {
            return BL_STATUS_GENERIC_ERROR;
        }
    }

    EFI_FILE_PROTOCOL* OpenedFile = NULL;
    FILE_SYSTEM_STATUS = CurrentDirectory->Open(
        CurrentDirectory,
        &OpenedFile,
        File,
        EFI_FILE_MODE_READ,
        0
    );


    if( EFI_ERROR( FILE_SYSTEM_STATUS ) )
    {
        DBG_ERROR( FILE_SYSTEM_STATUS, L"Failed to open file '%s'\n", File );
        getc();
        return FALSE;
    }
    

    *OutFile = OpenedFile;
    return TRUE;
}

EFI_STATUS
BLAPI
BlListDirectoryRecursive(
    _In_ EFI_FILE_PROTOCOL* Directory,
    _In_ ULONG64 Depth
)
{
    if( Directory == NULL )
    {
        return FALSE;
    }

    // reset position so we read from the start
    FILE_SYSTEM_STATUS = Directory->SetPosition( Directory, 0 );
    if( EFI_ERROR( FILE_SYSTEM_STATUS ) )
    {
        //DBG_ERROR( L"Failed to SetPosition on directory\n" );
        return FALSE;
    }

    // allocate a buffer for EFI_FILE_INFO. Sketchy as won't work for longer names.
    ULONG64 BufferSize = 0x128;
    EFI_FILE_INFO* FileInfo = AllocateZeroPool( BufferSize );
    if( !FileInfo )
    {
        //probably out of resources?
        FILE_SYSTEM_STATUS = EFI_OUT_OF_RESOURCES;
        return FALSE;
    }

    // Keep reading entries until we reach the end of the directory
    while( TRUE )
    {
        ULONG64 Size = BufferSize;
        FILE_SYSTEM_STATUS = Directory->Read( Directory, &Size, FileInfo );
        if( EFI_ERROR( FILE_SYSTEM_STATUS ) )
        {
            DBG_ERROR( FILE_SYSTEM_STATUS, L"Failed to read directory 's'\n", FileInfo->FileName );
            FreePool( FileInfo );
            return FALSE;
        }

        if( Size == 0 )
        {
            //no more to read
            break;
        }

        // skip the '.' and '..' at the start of directories to avoid infinite recursion
        if( ( StrCmp( FileInfo->FileName, L"." ) == 0 ) ||
            ( StrCmp( FileInfo->FileName, L".." ) == 0 ) )
        {
            continue;
        }

        // just add indentation
        for( ULONG64 i = 0; i < Depth; i++ )
        {
            Print( L"  " );
        }

        //check if its directory and list its contents
        if( FileInfo->Attribute & EFI_FILE_DIRECTORY )
        {
            Print( L"<DIR> %s\n", FileInfo->FileName );

            // try to open the subdirectory
            EFI_FILE_PROTOCOL* SubDirectory = NULL;
            FILE_SYSTEM_STATUS = Directory->Open(
                Directory,
                &SubDirectory,
                FileInfo->FileName,
                EFI_FILE_MODE_READ,
                0
            );

            if( EFI_ERROR( FILE_SYSTEM_STATUS ) && !SubDirectory )
            {
                DBG_ERROR( FILE_SYSTEM_STATUS, L"Cannot open subdirectory %s\n", FileInfo->FileName );
                FreePool( FileInfo );
                return FALSE;
            }

            // recursive call
            BlListDirectoryRecursive( SubDirectory, Depth + 1 );
            // make sure to close
            SubDirectory->Close( SubDirectory );
        }
        else
        {
            //if its a file then display contents
            Print( L"%-6d  %s\n", FileInfo->FileSize, FileInfo->FileName );
        }
    }

    FreePool( FileInfo );
    return TRUE;
}


EFI_STATUS
BLAPI
BlListAllFiles(
    VOID
)
{
    if( !CurrentDirectory )
    {
        //try get current directory
        BlGetRootDirectory( NULL );

        //well that is just nice
        if( !CurrentDirectory )
        {
            return FALSE;
        }
    }

    Print( L"\nDirectory listing -> \n" );
    return BlListDirectoryRecursive( CurrentDirectory, 0 );
}

EFI_STATUS
BLAPI
BlGetLastFileError(
    VOID
)
{
    return FILE_SYSTEM_STATUS;
}

EFI_STATUS
BLAPI
BlSetWorkingDirectory(
    _In_ PCWSTR Directory
)
{
    if( Directory == NULL || Directory[ 0 ] == '\0' )
    {
        FILE_SYSTEM_STATUS = EFI_INVALID_PARAMETER;
        return EFI_INVALID_PARAMETER;
    }

    // if no set directory set it!
    if( CurrentDirectory == NULL )
    {
        if( EFI_ERROR( BlGetRootDirectory( NULL ) ) )
        {
            if( EFI_ERROR( FILE_SYSTEM_STATUS ) )
            {
                DBG_ERROR( FILE_SYSTEM_STATUS, L"Failed to get root directory of fs0\n" );
                return FALSE;
            }
        }
    }

    if( CurrentDirectory )
    {
        EFI_FILE_PROTOCOL* NewDirectory = NULL;

        if( BlOpenSubDirectory( CurrentDirectory, Directory, &NewDirectory ) && NewDirectory )
        {
            // we got new direectory!
            CurrentDirectory = NewDirectory;
            return TRUE;
        }

        DBG_ERROR( FILE_SYSTEM_STATUS, L"Failed to get new directory '%s'\n", Directory );
    }

    return FALSE;
}

EFI_STATUS
BlGetFileName(
    _In_ EFI_FILE_PROTOCOL* FileProtocol,
    _Out_ PWSTR* Out
)
{
    if( FileProtocol == NULL )
    {
        FILE_SYSTEM_STATUS = EFI_INVALID_PARAMETER;
        return FILE_SYSTEM_STATUS;
    }

    ULONG64        InfoSize = 0x128;
    EFI_FILE_INFO* FileInfo = AllocateZeroPool( InfoSize );
    if( !FileInfo )
    {
        FILE_SYSTEM_STATUS = EFI_OUT_OF_RESOURCES;
        return FILE_SYSTEM_STATUS;
    }

    FILE_SYSTEM_STATUS = FileProtocol->GetInfo(
        FileProtocol,
        &gEfiFileInfoGuid,
        &InfoSize,
        FileInfo
    );

    if( EFI_ERROR( FILE_SYSTEM_STATUS ) )
    {
        FreePool( FileInfo );
        return FILE_SYSTEM_STATUS;
    }

    strfmt( Out, L"%s", FileInfo->FileName );

    FreePool( FileInfo );
    return EFI_SUCCESS;
}

EFI_STATUS
BlGetFileInfo(
    _In_ EFI_FILE_HANDLE FileHandle,
    _Inout_ EFI_FILE_INFO** FileInfo
)
{
    if( FileHandle == NULL || FileInfo == NULL )
    {
        FILE_SYSTEM_STATUS = EFI_INVALID_PARAMETER;
        return FILE_SYSTEM_STATUS;
    }

    //
    // allocate for sizeof( EFI_FILE_INFO ) + padding for file name size
    //
    UINTN FileInfoInfoAllocSize = sizeof( EFI_FILE_INFO ) + 1024;
    EFI_STATUS Result = gBS->AllocatePool( EfiBootServicesData, FileInfoInfoAllocSize, FileInfo );
    if( EFI_ERROR( Result ) )
    {
        FILE_SYSTEM_STATUS = Result;
        return FILE_SYSTEM_STATUS;
    }

    Result = FileHandle->GetInfo( FileHandle, &gEfiFileInfoGuid, &FileInfoInfoAllocSize, *FileInfo );
    if( EFI_ERROR( Result ) )
    {
        //
        // if somehow file name size was > 1024 re allocate and try again
        //
        if( Result == EFI_BUFFER_TOO_SMALL )
        {
            gBS->FreePool( *FileInfo );
            Result = gBS->AllocatePool( EfiBootServicesData, FileInfoInfoAllocSize, FileInfo );
            if( EFI_ERROR( Result ) )
            {
                FILE_SYSTEM_STATUS = Result;
                return FILE_SYSTEM_STATUS;
            }

            Result = FileHandle->GetInfo( FileHandle, &gEfiFileInfoGuid, &FileInfoInfoAllocSize, *FileInfo );

            if( EFI_ERROR( Result ) )
            {
                FILE_SYSTEM_STATUS = Result;
                gBS->FreePool( *FileInfo );
                return FILE_SYSTEM_STATUS;
            }
        }
        else
        {
            FILE_SYSTEM_STATUS = Result;
            return FILE_SYSTEM_STATUS;
        }
    }

    return EFI_SUCCESS;
}

EFI_STATUS
BLAPI
BlOpenOrCreateLogFile(
    _In_  PCWSTR Path,
    _Out_ EFI_FILE_PROTOCOL** OutFile
)
{
    EFI_STATUS Status;
    EFI_FILE_PROTOCOL* Root = NULL;

    if (!Path || !OutFile)
    {
        return EFI_INVALID_PARAMETER;
    }

    *OutFile = NULL;

    Status = BlGetRootDirectory(&Root);
    if (EFI_ERROR(Status))
    {
        return Status;
    }

    Status = Root->Open(
        Root,
        OutFile,
        (CHAR16*)Path,
        EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE,
        0
    );

    if (EFI_ERROR(Status))
    {
        return Status;
    }

    //
    // Append at EOF.
    //
    Status = (*OutFile)->SetPosition(*OutFile, MAX_UINT64);
    if (EFI_ERROR(Status))
    {
        (*OutFile)->Close(*OutFile);
        *OutFile = NULL;
        return Status;
    }

    return EFI_SUCCESS;
}

EFI_STATUS
BLAPI
BlWriteAscii(
    _In_ EFI_FILE_PROTOCOL* File,
    CONST VOID* Buffer,
    _In_ UINTN Size
)
{
    if (!File || (!Buffer && Size != 0))
    {
        return EFI_INVALID_PARAMETER;
    }

    return File->Write(File, &Size, (VOID*)Buffer);
}

EFI_STATUS
BLAPI
BlLogAscii(
    _In_ EFI_FILE_PROTOCOL* File,
    _In_ PCSTR Format,
    ...
)
{
    CHAR8 Buffer[512];
    VA_LIST Args;
    UINTN Length;

    if (!File || !Format)
    {
        return EFI_INVALID_PARAMETER;
    }

    VA_START(Args, Format);
    Length = AsciiVSPrint(Buffer, sizeof(Buffer), Format, Args);
    VA_END(Args);

    return BlWriteAscii(File, Buffer, Length);
}

EFI_STATUS
BLAPI
BlWriteBufferToNewFile(
    _In_ PCWSTR Path,
    CONST VOID* Buffer,
    _In_ UINTN Size
)
{
    EFI_STATUS Status;
    EFI_FILE_PROTOCOL* Root = NULL;
    EFI_FILE_PROTOCOL* Existing = NULL;
    EFI_FILE_PROTOCOL* File = NULL;
    UINTN WriteSize;

    if (!Path || (!Buffer && Size != 0))
    {
        return EFI_INVALID_PARAMETER;
    }

    Status = BlGetRootDirectory(&Root);
    if (EFI_ERROR(Status))
    {
        return Status;
    }

    //
    // Try to delete an older file first so this acts like overwrite.
    //
    Status = Root->Open(
        Root,
        &Existing,
        (CHAR16*)Path,
        EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE,
        0
    );

    if (!EFI_ERROR(Status) && Existing)
    {
        Existing->Delete(Existing);
        Existing = NULL;
    }

    Status = Root->Open(
        Root,
        &File,
        (CHAR16*)Path,
        EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE,
        0
    );

    if (EFI_ERROR(Status))
    {
        return Status;
    }

    WriteSize = Size;
    Status = File->Write(File, &WriteSize, (VOID*)Buffer);

    File->Flush(File);
    File->Close(File);

    return Status;
}

EFI_STATUS
BLAPI
BlDumpMmuStressLogToFile(
    _In_ PBOOT_INFO BootInfo,
    _In_ PCWSTR Path
)
{
    UINTN Size;

    if (!BootInfo || !Path)
    {
        return EFI_INVALID_PARAMETER;
    }

    if (BootInfo->MmuStressLog.Length > BootInfo->MmuStressLog.Capacity)
    {
        return EFI_BAD_BUFFER_SIZE;
    }

    Size = (UINTN)BootInfo->MmuStressLog.Length;

    return BlWriteBufferToNewFile(
        Path,
        BootInfo->MmuStressLog.Text,
        Size
    );
}

STATIC
PCSTR
BlCacheTypeStr(
    CPU_CACHE_TYPE CacheType
)
{
    switch (CacheType)
    {
    case CPU_FEATURE_CACHE_DATA:        return "Cache Data";
    case CPU_FEATURE_CACHE_INSTRUCTION: return "Cache Inst";
    case CPU_FEATURE_CACHE_UNIFIED:     return "Cache Unified";
    case CPU_FEATURE_CACHE_TLB:         return "TLB";
    case CPU_FEATURE_CACHE_DTLB:        return "DTLB";
    case CPU_FEATURE_CACHE_STLB:        return "STLB";
    case CPU_FEATURE_CACHE_PREFETCH:    return "PREF";
    default:                            return "NULL";
    }
}

STATIC
EFI_STATUS
BlCreateFreshFile(
    _In_ PCWSTR Path,
    _Out_ EFI_FILE_PROTOCOL** OutFile
)
{
    EFI_STATUS Status;
    EFI_FILE_PROTOCOL* Root = NULL;
    EFI_FILE_PROTOCOL* Existing = NULL;

    if (!Path || !OutFile)
    {
        return EFI_INVALID_PARAMETER;
    }

    *OutFile = NULL;

    Status = BlGetRootDirectory(&Root);
    if (EFI_ERROR(Status))
    {
        return Status;
    }

    //
    // Delete old file if it exists so this behaves like overwrite.
    //
    Status = Root->Open(
        Root,
        &Existing,
        (CHAR16*)Path,
        EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE,
        0
    );

    if (!EFI_ERROR(Status) && Existing)
    {
        Existing->Delete(Existing);
        Existing = NULL;
    }

    Status = Root->Open(
        Root,
        OutFile,
        (CHAR16*)Path,
        EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE,
        0
    );

    return Status;
}

STATIC
VOID
BlGetCpuBrandString(
     CHAR8 Brand[49]
)
{
    REGISTER_SET Reg = { 0 };
    ULONG32* Out;

    if (!Brand)
    {
        return;
    }

    memset(Brand, 0, 49);

    _scouse_cpuid(0x80000000, Reg.Registers);
    if (Reg.Eax < 0x80000004)
    {
        return;
    }

    Out = (ULONG32*)Brand;

    for (ULONG32 Leaf = 0x80000002; Leaf <= 0x80000004; ++Leaf)
    {
        _scouse_cpuid(Leaf, Reg.Registers);
        *Out++ = Reg.Eax;
        *Out++ = Reg.Ebx;
        *Out++ = Reg.Ecx;
        *Out++ = Reg.Edx;
    }

    Brand[48] = '\0';
}

STATIC
EFI_STATUS
BlWritePageMaskAscii(
    _In_ EFI_FILE_PROTOCOL* File,
    _In_ LONG32 Mask
)
{
    EFI_STATUS Status;

    if (!File)
    {
        return EFI_INVALID_PARAMETER;
    }

    if (Mask < 0)
    {
        return EFI_SUCCESS;
    }

    Status = BlLogAscii(File, "[");
    if (EFI_ERROR(Status))
    {
        return Status;
    }

    if (Mask & 0x1)
    {
        Status = BlLogAscii(File, "4K ");
        if (EFI_ERROR(Status))
        {
            return Status;
        }
    }

    if (Mask & 0x2)
    {
        Status = BlLogAscii(File, "2M ");
        if (EFI_ERROR(Status))
        {
            return Status;
        }
    }

    if (Mask & 0x4)
    {
        Status = BlLogAscii(File, "4M ");
        if (EFI_ERROR(Status))
        {
            return Status;
        }
    }

    if (Mask & 0x8)
    {
        Status = BlLogAscii(File, "1G ");
        if (EFI_ERROR(Status))
        {
            return Status;
        }
    }

    return BlLogAscii(File, "]");
}

EFI_STATUS
BLAPI
BlDumpCpuAndMmuStressLogToFile(
    _In_ PBOOT_INFO BootInfo,
    _In_ PCWSTR Path
)
{
    EFI_STATUS Status;
    EFI_FILE_PROTOCOL* File = NULL;
    CPUINFO CpuInfo = { 0 };
    CPU_CACHE_INFO CacheInfo = { 0 };
    CHAR8 Vendor[13] = { 0 };
    CHAR8 Brand[49] = { 0 };

    if (!BootInfo || !Path)
    {
        return EFI_INVALID_PARAMETER;
    }

    if (BootInfo->MmuStressLog.Length > BootInfo->MmuStressLog.Capacity)
    {
        return EFI_BAD_BUFFER_SIZE;
    }

    Status = BlCreateFreshFile(Path, &File);
    if (EFI_ERROR(Status))
    {
        return Status;
    }

    GetCpuInfo(&CpuInfo);
    memcpy(Vendor, CpuInfo.Vendor, 12);
    Vendor[12] = '\0';

    BlGetCpuBrandString(Brand);
    GetCacheInfo(&CacheInfo, 0);

    Status = BlLogAscii(File, "CPU Vendor: %a\n", Vendor);
    if (EFI_ERROR(Status))
    {
        goto Cleanup;
    }

    Status = BlLogAscii(
        File,
        "CPU Brand : %a\n\n",
        Brand[0] ? Brand : "Unknown"
    );
    if (EFI_ERROR(Status))
    {
        goto Cleanup;
    }

    Status = BlLogAscii(File, "CacheInfo: %d records\n", CacheInfo.Size);
    if (EFI_ERROR(Status))
    {
        goto Cleanup;
    }

    for (LONG32 Index = 0; Index < CacheInfo.Size; ++Index)
    {
        CACHE_INFO* L = &CacheInfo.Levels[Index];

        Status = BlLogAscii(
            File,
            "#%d  L%d  %a  Size=%d  Ways=%d  Part=%d  Line=%d  Entries/Sets=%d",
            Index,
            (LONG32)L->Level,
            BlCacheTypeStr(L->CacheType),
            (LONG32)L->CacheSize,
            (LONG32)L->Ways,
            (LONG32)L->Partitioning,
            (LONG32)L->LineSize,
            (LONG32)L->TlbEntries
        );
        if (EFI_ERROR(Status))
        {
            goto Cleanup;
        }

        if (L->CacheSize == -1 &&
            (L->CacheType == CPU_FEATURE_CACHE_TLB ||
                L->CacheType == CPU_FEATURE_CACHE_DTLB ||
                L->CacheType == CPU_FEATURE_CACHE_STLB))
        {
            Status = BlLogAscii(File, " PageSizes=");
            if (EFI_ERROR(Status))
            {
                goto Cleanup;
            }

            Status = BlWritePageMaskAscii(File, L->LineSize);
            if (EFI_ERROR(Status))
            {
                goto Cleanup;
            }
        }

        Status = BlLogAscii(File, "\n");
        if (EFI_ERROR(Status))
        {
            goto Cleanup;
        }
    }

    Status = BlLogAscii(
        File,
        "\nAddress spaces %Lx Count %Lu\n",
        (UINT64)(UINTN)BootInfo->MmuStresserDescriptors.AddressSpaces,
        (UINT64)BootInfo->MmuStresserDescriptors.AddressSpaceCount
    );
    if (EFI_ERROR(Status))
    {
        goto Cleanup;
    }

    Status = BlLogAscii(File, "\n==== MMU Stress Log ====\n");
    if (EFI_ERROR(Status))
    {
        goto Cleanup;
    }

    if (BootInfo->MmuStressLog.Text && BootInfo->MmuStressLog.Length)
    {
        Status = BlWriteAscii(
            File,
            BootInfo->MmuStressLog.Text,
            (UINTN)BootInfo->MmuStressLog.Length
        );
        if (EFI_ERROR(Status))
        {
            goto Cleanup;
        }
    }

    Status = File->Flush(File);

Cleanup:
    if (File)
    {
        File->Close(File);
    }

    return Status;
}