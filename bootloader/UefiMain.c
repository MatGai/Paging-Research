#include "filesystem.h"
#include "image.h"
#include "pagemanager.h"
#include "status.h"
#include <scouse/shared/cpuinfo.h>
#include <scouse/archx64/intrinsics.h>
#include <scouse/archx64/special.h>
#include <intrin.h>
#include <gop.h>

CHAR8* gEfiCallerBaseName = "Scouse Systems";
const UINT32 _gUefiDriverRevision = 0x0;

EFI_STATUS
BLAPI
InitalSetup( EFI_HANDLE ImageHandle );

/**
 * @brief Needed for VisualUefi for some reason?
 */
EFI_STATUS
EFIAPI
UefiUnload( EFI_HANDLE ImageHandle )
{
    return EFI_SUCCESS;
};

EFI_STATUS
BLAPI
FindExeFile( _In_ PCWSTR FileName, _Inout_ PBL_LDR_LOADED_IMAGE_INFO FileInfo )
{
    getc();
    if( EFI_ERROR( BlInitFileSystem( ) ) )
    {
        DBG_ERROR( BlGetLastFileError( ), L"File system init failed\n" );
        getc( );
        return EFI_NOT_FOUND;
    }

    if( EFI_ERROR( BlGetRootDirectory( NULL ) ) )
    {
        if( EFI_ERROR( FILE_SYSTEM_STATUS ) )
        {
            DBG_ERROR( BlGetLastFileError( ),
                       L"Failed to get root directory of current FS\n" );
        }
    }

    DBG_INFO( L"\nLooking for %s file pointer AHHH\n", FileName );
    EFI_FILE_PROTOCOL* File = NULL;
    if( !EFI_ERROR( BlFindFile( FileName, &File ) ) )
    {

        CHAR16* Buffer;
        if( BlGetFileName( File, &Buffer ) )
        {
            DBG_INFO( L"Got the file -> %s\n\n", Buffer );
        }
        FreePool( Buffer );
    }
    else
    {
        DBG_ERROR( BlGetLastFileError(), L"Failed to find %s file pointer\n", FileName);
        return EFI_LOAD_ERROR;
    }

    BlGetRootDirectory( NULL );

    BlLdrLoadPEImage64( FileName, FileInfo );

    return EFI_SUCCESS;
};

/**
 * @brief The entry point for the UEFI application.
 *
 * @param[in] EFI_HANDLE        The image handle of the UEFI application
 * @param[in] EFI_SYSTEM_TABLE* The system table of the UEFI application
 *
 * @return EFI_STATUS - The status of the UEFI application (useful if driver
 * loads this app)
 */
EFI_STATUS
EFIAPI
UefiMain( 
    EFI_HANDLE ImageHandle, 
    EFI_SYSTEM_TABLE* SystemTable 
)
{
    EFI_STATUS Status;
    Status = InitalSetup( ImageHandle );

    if( EFI_ERROR( Status ) )
    {
        DBG_ERROR( Status, L"Initial setup failed\n" );
        getc( );
        return Status;
    }

    BL_LDR_LOADED_IMAGE_INFO KernelImage = { 0 };
    Status = FindExeFile( L"kernel.exe", &KernelImage );

    if( EFI_ERROR( Status ) )
    {
        DBG_ERROR( Status, L"Failed to find image\n" );
        getc( );
        return Status;
    }

    Print( L"VirtualBase -> %p, Base -> %p , Entry %p\n", KernelImage.VirtualBase, KernelImage.Base, KernelImage.EntryPoint  );

    //
    // now we need to get details of memory, luckily efi provides this to us.
    // with memory map we are able to get what physical memory is not in use.
    // memory can be used by devices, firmware, etc.
    //
    BL_EFI_MEMORY_MAP SystemMemoryMap = { 0 };

    // get size of memory map
    EFI_STATUS MemMap = gBS->GetMemoryMap( &SystemMemoryMap.MapSize,
                                           (EFI_MEMORY_DESCRIPTOR*)SystemMemoryMap.Descriptor,
                                           &SystemMemoryMap.Key,
                                           &SystemMemoryMap.DescriptorSize,
                                           &SystemMemoryMap.Version );

    if( MemMap != EFI_BUFFER_TOO_SMALL )
    {
        DBG_ERROR( MemMap, L"Failed init Memory Map" );
        getc( );
        return 1;
    }

    // allocate memory for memory map
    SystemMemoryMap.MapSize += 2 * SystemMemoryMap.DescriptorSize;
    SystemMemoryMap.Descriptor = AllocateZeroPool( SystemMemoryMap.MapSize );

    // get memory map
    MemMap = gBS->GetMemoryMap( &SystemMemoryMap.MapSize,
                                (EFI_MEMORY_DESCRIPTOR*)SystemMemoryMap.Descriptor,
                                &SystemMemoryMap.Key,
                                &SystemMemoryMap.DescriptorSize,
                                &SystemMemoryMap.Version );

    if( EFI_ERROR( MemMap ) )
    {
        DBG_ERROR( MemMap, L"Failed get Memory Map" );
        getc( );
        return 1;
    }

    ULONG64 NumberOfDescriptors =
        SystemMemoryMap.MapSize / SystemMemoryMap.DescriptorSize;

    ULONG64 MaxAddress = 0;
    BL_EFI_MEMORY_DESCRIPTOR* Desc = SystemMemoryMap.Descriptor;

#define SsGetNextDescriptor(desc, size)                                        \
  ((BL_EFI_MEMORY_DESCRIPTOR*)(((UINT8*)(desc)) + size))

    //
    // we will get the highest memory address that is available to us
    //
    for( ULONG64 i = 0; i < NumberOfDescriptors; i++ )
    {
        switch( Desc->Type )
        {
            case EfiConventionalMemory:
            case EfiPersistentMemory:
            //case EfiBootServicesCode:
            //case EfiBootServicesData:
            {
                ULONG64 End = Desc->PhysicalStart + ( Desc->NumberOfPages * MM_PAGE_SIZE );
                if( End > MaxAddress )
                {
                    MaxAddress = End;
                }
                break;
            }
            default:
            {
                break;
            }
        }
        Desc = SsGetNextDescriptor( Desc, SystemMemoryMap.DescriptorSize );
    }

    // number of physical pages
    SsPfnCount = PHYSICAL_TO_PFN( MaxAddress );

    //
    // allocate memory for PFN entries
    //
    ULONG64 PfnSize = SsPfnCount * sizeof( PFN_ENTRY );
    ULONG64 PagesNeeded = ( PfnSize + MM_PAGE_SIZE - 1 ) / MM_PAGE_SIZE;
    ULONG64 PfnBase = 0;

    EFI_STATUS PfnAlloc = gBS->AllocatePages(
        AllocateAnyPages, EfiBootServicesData, PagesNeeded, &PfnBase );

    if( EFI_ERROR( PfnAlloc ) )
    {
        DBG_ERROR( PfnAlloc, L"Failed allocating pfn base\n" );
        getc( );
        return 0;
    }

    SsPfn = ( PFN_ENTRY* )PfnBase;

    // for now set all physical pages to reserved
    for( ULONG64 pfn = 0; pfn < SsPfnCount; pfn++ )
    {
        SsPfn[ pfn ].State = Reserved;
        SsPfn[ pfn ].Ref = 0;
        SsPfn[ pfn ].Offset = 0xffffff;
    }

    SsPfnFreeHead = 0xffffff;

    FreePool(SystemMemoryMap.Descriptor);
    SystemMemoryMap.Descriptor = NULL;
    SystemMemoryMap.MapSize = 0;

    // get size of memory map
    MemMap = gBS->GetMemoryMap(&SystemMemoryMap.MapSize,
        NULL,

        &SystemMemoryMap.Key,
        &SystemMemoryMap.DescriptorSize,
        &SystemMemoryMap.Version);

    if (MemMap != EFI_BUFFER_TOO_SMALL)
    {
        DBG_ERROR(MemMap, L"Failed init Memory Map");
        getc();
        return 1;
    }

    // allocate memory for memory map
    SystemMemoryMap.MapSize += 2 * SystemMemoryMap.DescriptorSize;
    SystemMemoryMap.Descriptor = (PBL_EFI_MEMORY_DESCRIPTOR)AllocateZeroPool( SystemMemoryMap.MapSize );

    // get memory map
    MemMap = gBS->GetMemoryMap( &SystemMemoryMap.MapSize,
                                (EFI_MEMORY_DESCRIPTOR*)SystemMemoryMap.Descriptor,
                                &SystemMemoryMap.Key,
                                &SystemMemoryMap.DescriptorSize,
                                &SystemMemoryMap.Version );

    if( EFI_ERROR( MemMap ) )
    {
        DBG_ERROR( MemMap, L"Failed get Memory Map" );
        getc( );
        return 1;
    }

    NumberOfDescriptors =
        SystemMemoryMap.MapSize / SystemMemoryMap.DescriptorSize;

    Desc = SystemMemoryMap.Descriptor;

    //
    // iterate through memory map and set physical pages to free, that are free
    // anyways.
    //
    for( ULONG64 i = 0; i < NumberOfDescriptors; i++ )
    {
        ULONG64 Start = Desc->PhysicalStart;
        ULONG64 PageCount = Desc->NumberOfPages;
        ULONG64 End = Start + PageCount * MM_PAGE_SIZE;

        switch( Desc->Type )
        {
            // realistically we do not care about firmware memory anymore.
            // Do not include runtimeservices, MMIO or other reserved memory as we are
            // not mapping them.
            case EfiConventionalMemory:
            case EfiPersistentMemory:
             //case EfiBootServicesCode:
             //case EfiBootServicesData:
            {
                ULONG64 StartPFN = PHYSICAL_TO_PFN( Start );
                ULONG64 EndPFN = PHYSICAL_TO_PFN( End );

                for( ULONG64 PFN = StartPFN; PFN < EndPFN; PFN++ )
                {
                    SsPfn[ PFN ].State = Free;
                    SsPfn[ PFN ].Ref = 0;
                    SsPfn[ PFN ].Offset = ( UINT32 )SsPfnFreeHead;
                    SsPfnFreeHead = PFN;
                }
                // Print(L"Free data to use by pfn!!!\n");
                break;
            }

            default:
            {
                break;
            }
        }
        Desc = SsGetNextDescriptor( Desc, SystemMemoryMap.DescriptorSize );
    }

    DBG_INFO( L"PFN free head -> %p\n", SsPfnFreeHead );
    DBG_INFO( L"PFN count -> %d\n", SsPfnCount );

    ULONG64 Pml4Physical = SsPagingInit( );
    if( !Pml4Physical )
    {
        //DBG_ERROR( L"Failed to allogcate Pml4 a physical address" );
        getc( );
        return 1;
    }

    DBG_INFO(L"PML4 Physical -> %p, MaxAddress -> %p\n", Pml4Physical, MaxAddress );

    ULONG64 NewStack;
    AllocatePage( &NewStack );

    EFI_STATUS St = MmMapPage( KERNEL_VA_STACK, NewStack, MmReadWriteExecuteProtection );
    if (EFI_ERROR(St))
    {
        DBG_ERROR(St, L"Failed\n");
    }

    if( EFI_ERROR( MmMappingExists( (ULONG64)gPML4, KERNEL_VA_STACK ) ) )
    {
        DBG_ERROR( L"Error", L"Stack page doesnt exit\n" );
    }

    ULONG64 TransitionFunctionLength = (ULONG64)(scouse_transition_address_space_end - scouse_transition_address_space_start);

    ULONG64 TransitionPage;
    St = AllocatePage(&TransitionPage);
    if (EFI_ERROR(St))
    {
        Print(L"Transition alloc err %r\n", St);
    }
    CopyMem((PVOID)TransitionPage, scouse_transition_address_space_start, TransitionFunctionLength);
    MmMapPage(TransitionPage, TransitionPage, MmReadWriteExecuteProtection);

    SCOUSE_TRANSITION_ADDRESS_SPACE Transition = (SCOUSE_TRANSITION_ADDRESS_SPACE)(ULONG64)TransitionPage;

    MapKernelPage( KernelImage.Base, KernelImage.VirtualBase );

    PBOOT_INFO BootInfo = NULL;
    St = gBS->AllocatePool(EfiLoaderData, sizeof(*BootInfo), (void**)&BootInfo);
    if (EFI_ERROR(St))
    {
        Print(L"%r bootinfo\n", St);
        getc();
        return St;
    }

    ZeroMem(BootInfo, sizeof(*BootInfo));

    BootInfo->DirectMapBase = 0;
    BootInfo->Pml4Physical = Pml4Physical;
    BootInfo->PfnArrayPhysical = (ULONG64)SsPfn;
    BootInfo->PfnCount = SsPfnCount;
    BootInfo->PfnFreeHead = SsPfnFreeHead;
    BootInfo->MmuStressLog.Length = 0;
    BootInfo->MmuStressLog.Capacity = MMU_STRESS_LOG_TEXT_SIZE;
    BootInfo->MmuStressLog.Text[0] = '\0';

/*    ULONG64 HeapPhysicalBase = 0;
    ULONG64 HeapLength = 0;

    Status = BlAllocateMmuStressArena(&HeapPhysicalBase, &HeapLength);
    if (EFI_ERROR(Status))
    {
        DBG_ERROR(Status, L"Failed to allocate MMU stress arena\n");
        return Status;
    }

    Status = BlBuildMmuStressSpaces(BootInfo, HeapPhysicalBase, HeapLength);
    if (EFI_ERROR(Status))
    {
        DBG_ERROR(Status, L"Failed to build MMU stress spaces\n");
        return Status;
    }*/
/*
    BlDumpLargestConventionalRegion(BootInfo);

    getc();*/

    Print(L"Press a key to jump to kernel");
    getc();

    St = BlGopPrepareGraphics(0, &BootInfo->FrameBufferDescriptor);
    if (SC_FAILED(St))
    {
        Print(L"Failed prepare graphics\n");
    }

    MmMapRange(
        MM_PAGE_ALIGN_DOWN((ULONG64)BootInfo),
        MM_PAGE_ALIGN_DOWN((ULONG64)BootInfo),
        MM_PAGE_ALIGN_UP(sizeof(*BootInfo) + ((ULONG64)BootInfo & MM_PAGE_MASK)),
        MmReadWriteExecuteProtection
    );

    MmMapRange(
        MM_PAGE_ALIGN_DOWN((ULONG64)SsPfn),
        MM_PAGE_ALIGN_DOWN((ULONG64)SsPfn),
        MM_PAGE_ALIGN_UP(PagesNeeded* MM_PAGE_SIZE),
        MmReadWriteExecuteProtection
    );

    if (
        !MmMappingExists((ULONG64)gPML4, BootInfo->FrameBufferDescriptor.FrameBufferBase)
    ) 
    {
        MmMapRange(
            BootInfo->FrameBufferDescriptor.FrameBufferBase,
            BootInfo->FrameBufferDescriptor.FrameBufferBase,
            BootInfo->FrameBufferDescriptor.FrameBufferSize,
            MmReadWriteExecuteProtection | MmStrongUncacheablePagePolicy
        );

        if (!MmMappingExists((ULONG64)gPML4, BootInfo->FrameBufferDescriptor.FrameBufferBase))
        {
            Print(L"Failed to map frame buffer base\n");
            getc();
        }
    }

    if ( !MmMappingExists((ULONG64)gPML4, (ULONG64)&BootInfo->FrameBufferDescriptor ) )
    {
        DBG_ERROR(L"Error", L"Frame buffer dsecriptor not mapped! ... 0x%llx, 0x%llx\n", &BootInfo->FrameBufferDescriptor, BootInfo->FrameBufferDescriptor);
        getc();
    }

    Status = BlAllocateStresserPhysical(
        BootInfo,
        sizeof(gMmuTestingDescriptors) / sizeof(gMmuTestingDescriptors[0])
    );
    if (EFI_ERROR(Status))
    {
        Print(L"[%r] Failed to build MMU stress arena\n", Status);
        getc();
        return Status;
    }

    ULONG64 ReturnValue = Transition(Pml4Physical, KERNEL_VA_STACK_TOP, KernelImage.VirtualBase + KernelImage.EntryPoint, BootInfo);

    Print(L"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\nRet val %d\n", ReturnValue); //this messes with the frame buffer, breaking the display.

    Print(L"Kernel returned: %llu\n", ReturnValue);
    Print(L"MMU log length: %llu bytes\n", BootInfo->MmuStressLog.Length);

    EFI_STATUS LogStatus = BlDumpCpuAndMmuStressLogToFile(BootInfo, L"\\mmu_stress.txt");
    if (EFI_ERROR(LogStatus))
    {
        Print(L"Failed to write \\mmu_stress.txt : %r\n", LogStatus);
    }
    else
    {
        Print(L"Wrote \\mmu_stress.txt successfully\n");
    }

    getc();
    getc();

    return EFI_SUCCESS;
}

EFI_STATUS
BLAPI
InitalSetup(
    EFI_HANDLE ImageHandle
)
{
    EFI_LOADED_IMAGE* LoadedIamge = NULL;
    EFI_STATUS err = gBS->HandleProtocol(
        ImageHandle, &gEfiLoadedImageProtocolGuid, &LoadedIamge );

    if( EFI_ERROR( err ) )
    {
        return err;
    }

    SCSTATUS St = BlGopInit();
    if (EFI_ERROR(St))
    {
        DBG_ERROR(St, "Failed to set initialise GOP\n");
    }

    St = BlGopSetMode(0);
    if (EFI_ERROR(St))
    {
        DBG_ERROR(St, "Failed to set gop mode to %d\n", 0);
    }

    DBG_INFO( L"handle-> %p\n", LoadedIamge->ImageBase );

    EFI_TIME time;
    gRT->GetTime( &time, NULL );

    Print( L"%02d/%02d/%04d ----- %02d:%02d:%0d.%d\r\n",
           time.Day,
           time.Month,
           time.Year,
           time.Hour,
           time.Minute,
           time.Second,
           time.Nanosecond );

    //BlDbgBreak();

    CPUINFO CpuInfo;

    GetCpuInfo( &CpuInfo );

    Print(L"Cpu info: %a, %a\n", CpuInfo.Vendor, CpuInfo.Brand);

    return EFI_SUCCESS;
};
