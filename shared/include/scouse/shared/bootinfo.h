#ifndef SHRD_BOOTINFO_H
#define SHRD_BOOTINFO_H

#include <scouse/shared/typedefs.h>
#include <scouse/archx64/paging.h>

//
// MEMORY MAP
//

typedef struct _BL_EFI_MEMORY_DESCRIPTOR
{
    ULONG32 Type;
    ULONG64 PhysicalStart;
    ULONG64 VirtualStart;
    ULONG64 NumberOfPages;
    ULONG64 Attribute;

} BL_EFI_MEMORY_DESCRIPTOR, * PBL_EFI_MEMORY_DESCRIPTOR;

static_assert(sizeof(BL_EFI_MEMORY_DESCRIPTOR) == 40, "BL_EFI_MEMORY_DESCRIPTOR must be aligned to 8 bytes");

typedef struct _BL_EFI_MEMORY_MAP
{
    PBL_EFI_MEMORY_DESCRIPTOR Descriptor;
    ULONG64 MapSize;
    ULONG64 Key;
    ULONG64 DescriptorSize;
    ULONG32 Version;

} BL_EFI_MEMORY_MAP, * PBL_EFI_MEMORY_MAP;

static_assert(sizeof(BL_EFI_MEMORY_MAP) == 40, "BL_EFI_MEMORY_MAP must be aligned to 8 bytes");

typedef enum _MMU_STRESS_MODE
{
    MmuStressModePages,
    MmuStressModeBytes
} MMU_STRESS_MODE;

typedef struct _MMU_STRESS_SPACE_DESCRIPTOR
{
    ULONG64 VirtualBase;
    ULONG64 PageSize;

    MMU_STRESS_MODE SizingMode;
    ULONG64 SizeValue;

    ULONG64 PageFlags;

} MMU_STRESS_SPACE_DESCRIPTOR, * PMMU_STRESS_SPACE_DESCRIPTOR;

typedef struct _MMU_STRESS_SPACE_SETS
{
    ULONG64 SetIdentifier;

    PMMU_STRESS_SPACE_DESCRIPTOR SpacePage;
    PMMU_STRESS_SPACE_DESCRIPTOR SpaceLargePage;
    PMMU_STRESS_SPACE_DESCRIPTOR SpaceHugePage;

} MMU_STRESS_SPACE_SETS, * PMMU_STRESS_SPACE_SETS;

typedef struct _ADDRESS_SPACE_DESCRIPTORS
{
    MM_PAGING_BASE_DESCRIPTOR AddressSpace;

    ULONG64 HeapVirtualBase;
    ULONG64 HeapPhysicalBase;

    ULONG64 HeapLength;
    ULONG64 HeapPages;

    ULONG64 PageSize;

    ULONG64 SetIdentifier;

    PMM_PTE_DESCRIPTOR HeapPageDescriptors;

} ADDRESS_SPACE_DESCRIPTORS, * PADDRESS_SPACE_DESCRIPTORS;

typedef struct _MMU_STRESSER_DESCRIPTORS
{
    PADDRESS_SPACE_DESCRIPTORS AddressSpaces;
    ULONG64 AddressSpaceCount;

} MMU_STRESSER_DESCRIPTORS, * PMMU_STRESSER_DESCRIPTORS;

//
// SHARED MMU LOG BUFFER
//

#define MMU_STRESS_LOG_TEXT_SIZE (64 * 1024)

typedef struct _MMU_STRESS_LOG_BUFFER
{
    ULONG64 Length;
    ULONG64 Capacity;
    CHAR   Text[MMU_STRESS_LOG_TEXT_SIZE];

} MMU_STRESS_LOG_BUFFER, * PMMU_STRESS_LOG_BUFFER;

//
// GOP INFO
//

typedef struct _BL_GOP_FRAMEBUFFER_DESCRIPTOR
{
    ULONG64 FrameBufferBase;
    ULONG64 FrameBufferSize;
    UINT16  BytesPerScanLine;
    LONG32  ResolutionWidth;
    LONG32  ResolutionHeight;
    ULONG32 RedMask;
    ULONG32 GreenMask;
    ULONG32 BlueMask;
    ULONG32 ResvMask;

} BL_GOP_FRAMEBUFFER_DESCRIPTOR, * PBL_GOP_FRAMEBUFFER_DESCRIPTOR;

typedef struct _BOOT_INFO
{
    MMU_STRESSER_DESCRIPTORS      MmuStresserDescriptors;
    MMU_STRESS_LOG_BUFFER         MmuStressLog;

    BL_GOP_FRAMEBUFFER_DESCRIPTOR FrameBufferDescriptor;
    BL_EFI_MEMORY_MAP             MemoryMap;

    ULONG64 DirectMapBase;
    ULONG64 Pml4Physical;
    ULONG64 PfnArrayPhysical;
    ULONG64 PfnCount;
    ULONG64 PfnFreeHead;

} BOOT_INFO, * PBOOT_INFO;

#endif // !SHRD_BOOTINFO_H