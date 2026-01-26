#include <scouse/archx64/cpu/cpudata.h>
#include <scouse/shared/cpuinfo.h>
#include <scouse/runtime/string.h>

static CacheLevelInfo EmptyCacheLevelInfo = { 0 };

static CacheLevelInfo GetCacheLevelInfo(const uint32_t reg) {
    const int UNDEF = -1;
    const int KiB = 1024;
    const int MiB = 1024 * KiB;
    switch (reg) {
    case 0x01:
        return (CacheLevelInfo) {
            .level = UNDEF,
                .cache_type = CPU_FEATURE_CACHE_TLB,
                .cache_size = 4 * KiB,
                .ways = 4,
                .line_size = UNDEF,
                .tlb_entries = 32,
                .partitioning = 0
        };
    case 0x02:
        return (CacheLevelInfo) {
            .level = UNDEF,
                .cache_type = CPU_FEATURE_CACHE_TLB,
                .cache_size = 4 * MiB,
                .ways = 0xFF,
                .line_size = UNDEF,
                .tlb_entries = 2,
                .partitioning = 0
        };
    case 0x03:
        return (CacheLevelInfo) {
            .level = UNDEF,
                .cache_type = CPU_FEATURE_CACHE_TLB,
                .cache_size = 4 * KiB,
                .ways = 4,
                .line_size = UNDEF,
                .tlb_entries = 64,
                .partitioning = 0
        };
    case 0x04:
        return (CacheLevelInfo) {
            .level = UNDEF,
                .cache_type = CPU_FEATURE_CACHE_TLB,
                .cache_size = 4 * MiB,
                .ways = 4,
                .line_size = UNDEF,
                .tlb_entries = 8,
                .partitioning = 0
        };
    case 0x05:
        return (CacheLevelInfo) {
            .level = UNDEF,
                .cache_type = CPU_FEATURE_CACHE_TLB,
                .cache_size = 4 * MiB,
                .ways = 4,
                .line_size = UNDEF,
                .tlb_entries = 32,
                .partitioning = 0
        };
    case 0x06:
        return (CacheLevelInfo) {
            .level = 1,
                .cache_type = CPU_FEATURE_CACHE_INSTRUCTION,
                .cache_size = 8 * KiB,
                .ways = 4,
                .line_size = 32,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0x08:
        return (CacheLevelInfo) {
            .level = 1,
                .cache_type = CPU_FEATURE_CACHE_INSTRUCTION,
                .cache_size = 16 * KiB,
                .ways = 4,
                .line_size = 32,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0x09:
        return (CacheLevelInfo) {
            .level = 1,
                .cache_type = CPU_FEATURE_CACHE_INSTRUCTION,
                .cache_size = 32 * KiB,
                .ways = 4,
                .line_size = 64,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0x0A:
        return (CacheLevelInfo) {
            .level = 1,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 8 * KiB,
                .ways = 2,
                .line_size = 32,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0x0B:
        return (CacheLevelInfo) {
            .level = UNDEF,
                .cache_type = CPU_FEATURE_CACHE_TLB,
                .cache_size = 4 * MiB,
                .ways = 4,
                .line_size = UNDEF,
                .tlb_entries = 4,
                .partitioning = 0
        };
    case 0x0C:
        return (CacheLevelInfo) {
            .level = 1,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 16 * KiB,
                .ways = 4,
                .line_size = 32,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0x0D:
        return (CacheLevelInfo) {
            .level = 1,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 16 * KiB,
                .ways = 4,
                .line_size = 64,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0x0E:
        return (CacheLevelInfo) {
            .level = 1,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 24 * KiB,
                .ways = 6,
                .line_size = 64,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0x1D:
        return (CacheLevelInfo) {
            .level = 2,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 128 * KiB,
                .ways = 2,
                .line_size = 64,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0x21:
        return (CacheLevelInfo) {
            .level = 2,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 256 * KiB,
                .ways = 8,
                .line_size = 64,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0x22:
        return (CacheLevelInfo) {
            .level = 3,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 512 * KiB,
                .ways = 4,
                .line_size = 64,
                .tlb_entries = UNDEF,
                .partitioning = 2
        };
    case 0x23:
        return (CacheLevelInfo) {
            .level = 3,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 1 * MiB,
                .ways = 8,
                .line_size = 64,
                .tlb_entries = UNDEF,
                .partitioning = 2
        };
    case 0x24:
        return (CacheLevelInfo) {
            .level = 2,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 1 * MiB,
                .ways = 16,
                .line_size = 64,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0x25:
        return (CacheLevelInfo) {
            .level = 3,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 2 * MiB,
                .ways = 8,
                .line_size = 64,
                .tlb_entries = UNDEF,
                .partitioning = 2
        };
    case 0x29:
        return (CacheLevelInfo) {
            .level = 3,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 4 * MiB,
                .ways = 8,
                .line_size = 64,
                .tlb_entries = UNDEF,
                .partitioning = 2
        };
    case 0x2C:
        return (CacheLevelInfo) {
            .level = 1,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 32 * KiB,
                .ways = 8,
                .line_size = 64,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0x30:
        return (CacheLevelInfo) {
            .level = 1,
                .cache_type = CPU_FEATURE_CACHE_INSTRUCTION,
                .cache_size = 32 * KiB,
                .ways = 8,
                .line_size = 64,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0x40:
        return (CacheLevelInfo) {
            .level = UNDEF,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = UNDEF,
                .ways = UNDEF,
                .line_size = UNDEF,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0x41:
        return (CacheLevelInfo) {
            .level = 2,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 128 * KiB,
                .ways = 4,
                .line_size = 32,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0x42:
        return (CacheLevelInfo) {
            .level = 2,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 256 * KiB,
                .ways = 4,
                .line_size = 32,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0x43:
        return (CacheLevelInfo) {
            .level = 2,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 512 * KiB,
                .ways = 4,
                .line_size = 32,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0x44:
        return (CacheLevelInfo) {
            .level = 2,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 1 * MiB,
                .ways = 4,
                .line_size = 32,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0x45:
        return (CacheLevelInfo) {
            .level = 2,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 2 * MiB,
                .ways = 4,
                .line_size = 32,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0x46:
        return (CacheLevelInfo) {
            .level = 3,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 4 * MiB,
                .ways = 4,
                .line_size = 64,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0x47:
        return (CacheLevelInfo) {
            .level = 3,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 8 * MiB,
                .ways = 8,
                .line_size = 64,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0x48:
        return (CacheLevelInfo) {
            .level = 2,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 3 * MiB,
                .ways = 12,
                .line_size = 64,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0x49:
        return (CacheLevelInfo) {
            .level = 2,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 4 * MiB,
                .ways = 16,
                .line_size = 64,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case (0x49 | (1 << 8)):
        return (CacheLevelInfo) {
        .level = 3,
            .cache_type = CPU_FEATURE_CACHE_DATA,
            .cache_size = 4 * MiB,
            .ways = 16,
            .line_size = 64,
            .tlb_entries = UNDEF,
            .partitioning = 0
    };
    case 0x4A:
        return (CacheLevelInfo) {
            .level = 3,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 6 * MiB,
                .ways = 12,
                .line_size = 64,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0x4B:
        return (CacheLevelInfo) {
            .level = 3,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 8 * MiB,
                .ways = 16,
                .line_size = 64,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0x4C:
        return (CacheLevelInfo) {
            .level = 3,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 12 * MiB,
                .ways = 12,
                .line_size = 64,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0x4D:
        return (CacheLevelInfo) {
            .level = 3,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 16 * MiB,
                .ways = 16,
                .line_size = 64,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0x4E:
        return (CacheLevelInfo) {
            .level = 2,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 6 * MiB,
                .ways = 24,
                .line_size = 64,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0x4F:
        return (CacheLevelInfo) {
            .level = UNDEF,
                .cache_type = CPU_FEATURE_CACHE_TLB,
                .cache_size = 4 * KiB,
                .ways = UNDEF,
                .line_size = UNDEF,
                .tlb_entries = 32,
                .partitioning = 0
        };
    case 0x50:
        return (CacheLevelInfo) {
            .level = UNDEF,
                .cache_type = CPU_FEATURE_CACHE_TLB,
                .cache_size = 4 * KiB,
                .ways = UNDEF,
                .line_size = UNDEF,
                .tlb_entries = 64,
                .partitioning = 0
        };
    case 0x51:
        return (CacheLevelInfo) {
            .level = UNDEF,
                .cache_type = CPU_FEATURE_CACHE_TLB,
                .cache_size = 4 * KiB,
                .ways = UNDEF,
                .line_size = UNDEF,
                .tlb_entries = 128,
                .partitioning = 0
        };
    case 0x52:
        return (CacheLevelInfo) {
            .level = UNDEF,
                .cache_type = CPU_FEATURE_CACHE_TLB,
                .cache_size = 4 * KiB,
                .ways = UNDEF,
                .line_size = UNDEF,
                .tlb_entries = 256,
                .partitioning = 0
        };
    case 0x55:
        return (CacheLevelInfo) {
            .level = UNDEF,
                .cache_type = CPU_FEATURE_CACHE_TLB,
                .cache_size = 2 * MiB,
                .ways = 0xFF,
                .line_size = UNDEF,
                .tlb_entries = 7,
                .partitioning = 0
        };
    case 0x56:
        return (CacheLevelInfo) {
            .level = UNDEF,
                .cache_type = CPU_FEATURE_CACHE_TLB,
                .cache_size = 4 * MiB,
                .ways = 4,
                .line_size = UNDEF,
                .tlb_entries = 16,
                .partitioning = 0
        };
    case 0x57:
        return (CacheLevelInfo) {
            .level = UNDEF,
                .cache_type = CPU_FEATURE_CACHE_TLB,
                .cache_size = 4 * KiB,
                .ways = 4,
                .line_size = UNDEF,
                .tlb_entries = 16,
                .partitioning = 0
        };
    case 0x59:
        return (CacheLevelInfo) {
            .level = UNDEF,
                .cache_type = CPU_FEATURE_CACHE_TLB,
                .cache_size = 4 * KiB,
                .ways = 0xFF,
                .line_size = UNDEF,
                .tlb_entries = 16,
                .partitioning = 0
        };
    case 0x5A:
        return (CacheLevelInfo) {
            .level = UNDEF,
                .cache_type = CPU_FEATURE_CACHE_TLB,
                .cache_size = 2 * MiB,
                .ways = 4,
                .line_size = UNDEF,
                .tlb_entries = 32,
                .partitioning = 0
        };
    case 0x5B:
        return (CacheLevelInfo) {
            .level = UNDEF,
                .cache_type = CPU_FEATURE_CACHE_TLB,
                .cache_size = 4 * KiB,
                .ways = UNDEF,
                .line_size = UNDEF,
                .tlb_entries = 64,
                .partitioning = 0
        };
    case 0x5C:
        return (CacheLevelInfo) {
            .level = UNDEF,
                .cache_type = CPU_FEATURE_CACHE_TLB,
                .cache_size = 4 * KiB,
                .ways = UNDEF,
                .line_size = UNDEF,
                .tlb_entries = 128,
                .partitioning = 0
        };
    case 0x5D:
        return (CacheLevelInfo) {
            .level = UNDEF,
                .cache_type = CPU_FEATURE_CACHE_TLB,
                .cache_size = 4 * KiB,
                .ways = UNDEF,
                .line_size = UNDEF,
                .tlb_entries = 256,
                .partitioning = 0
        };
    case 0x60:
        return (CacheLevelInfo) {
            .level = 1,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 16 * KiB,
                .ways = 8,
                .line_size = 64,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0x61:
        return (CacheLevelInfo) {
            .level = UNDEF,
                .cache_type = CPU_FEATURE_CACHE_TLB,
                .cache_size = 4 * KiB,
                .ways = 0xFF,
                .line_size = UNDEF,
                .tlb_entries = 48,
                .partitioning = 0
        };
    case 0x63:
        return (CacheLevelInfo) {
            .level = UNDEF,
                .cache_type = CPU_FEATURE_CACHE_TLB,
                .cache_size = 2 * MiB,
                .ways = 4,
                .line_size = UNDEF,
                .tlb_entries = 4,
                .partitioning = 0
        };
    case 0x66:
        return (CacheLevelInfo) {
            .level = 1,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 8 * KiB,
                .ways = 4,
                .line_size = 64,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0x67:
        return (CacheLevelInfo) {
            .level = 1,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 16 * KiB,
                .ways = 4,
                .line_size = 64,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0x68:
        return (CacheLevelInfo) {
            .level = 1,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 32 * KiB,
                .ways = 4,
                .line_size = 64,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0x70:
        return (CacheLevelInfo) {
            .level = 1,
                .cache_type = CPU_FEATURE_CACHE_INSTRUCTION,
                .cache_size = 12 * KiB,
                .ways = 8,
                .line_size = UNDEF,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0x71:
        return (CacheLevelInfo) {
            .level = 1,
                .cache_type = CPU_FEATURE_CACHE_INSTRUCTION,
                .cache_size = 16 * KiB,
                .ways = 8,
                .line_size = UNDEF,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0x72:
        return (CacheLevelInfo) {
            .level = 1,
                .cache_type = CPU_FEATURE_CACHE_INSTRUCTION,
                .cache_size = 32 * KiB,
                .ways = 8,
                .line_size = UNDEF,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0x76:
        return (CacheLevelInfo) {
            .level = UNDEF,
                .cache_type = CPU_FEATURE_CACHE_TLB,
                .cache_size = 2 * MiB,
                .ways = 0xFF,
                .line_size = UNDEF,
                .tlb_entries = 8,
                .partitioning = 0
        };
    case 0x78:
        return (CacheLevelInfo) {
            .level = 2,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 1 * MiB,
                .ways = 4,
                .line_size = 64,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0x79:
        return (CacheLevelInfo) {
            .level = 2,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 128 * KiB,
                .ways = 8,
                .line_size = 64,
                .tlb_entries = UNDEF,
                .partitioning = 2
        };
    case 0x7A:
        return (CacheLevelInfo) {
            .level = 2,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 256 * KiB,
                .ways = 8,
                .line_size = 64,
                .tlb_entries = UNDEF,
                .partitioning = 2
        };
    case 0x7B:
        return (CacheLevelInfo) {
            .level = 2,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 512 * KiB,
                .ways = 8,
                .line_size = 64,
                .tlb_entries = UNDEF,
                .partitioning = 2
        };
    case 0x7C:
        return (CacheLevelInfo) {
            .level = 2,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 1 * MiB,
                .ways = 8,
                .line_size = 64,
                .tlb_entries = UNDEF,
                .partitioning = 2
        };
    case 0x7D:
        return (CacheLevelInfo) {
            .level = 2,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 2 * MiB,
                .ways = 8,
                .line_size = 64,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0x7F:
        return (CacheLevelInfo) {
            .level = 2,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 512 * KiB,
                .ways = 2,
                .line_size = 64,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0x80:
        return (CacheLevelInfo) {
            .level = 2,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 512 * KiB,
                .ways = 8,
                .line_size = 64,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0x82:
        return (CacheLevelInfo) {
            .level = 2,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 256 * KiB,
                .ways = 8,
                .line_size = 32,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0x83:
        return (CacheLevelInfo) {
            .level = 2,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 512 * KiB,
                .ways = 8,
                .line_size = 32,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0x84:
        return (CacheLevelInfo) {
            .level = 2,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 1 * MiB,
                .ways = 8,
                .line_size = 32,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0x85:
        return (CacheLevelInfo) {
            .level = 2,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 2 * MiB,
                .ways = 8,
                .line_size = 32,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0x86:
        return (CacheLevelInfo) {
            .level = 2,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 512 * KiB,
                .ways = 4,
                .line_size = 32,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0x87:
        return (CacheLevelInfo) {
            .level = 2,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 1 * MiB,
                .ways = 8,
                .line_size = 64,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0xA0:
        return (CacheLevelInfo) {
            .level = UNDEF,
                .cache_type = CPU_FEATURE_CACHE_DTLB,
                .cache_size = 4 * KiB,
                .ways = 0xFF,
                .line_size = UNDEF,
                .tlb_entries = 32,
                .partitioning = 0
        };
    case 0xB0:
        return (CacheLevelInfo) {
            .level = UNDEF,
                .cache_type = CPU_FEATURE_CACHE_TLB,
                .cache_size = 4 * KiB,
                .ways = 4,
                .line_size = UNDEF,
                .tlb_entries = 128,
                .partitioning = 0
        };
    case 0xB1:
        return (CacheLevelInfo) {
            .level = UNDEF,
                .cache_type = CPU_FEATURE_CACHE_TLB,
                .cache_size = 2 * MiB,
                .ways = 4,
                .line_size = UNDEF,
                .tlb_entries = 8,
                .partitioning = 0
        };
    case 0xB2:
        return (CacheLevelInfo) {
            .level = UNDEF,
                .cache_type = CPU_FEATURE_CACHE_TLB,
                .cache_size = 4 * KiB,
                .ways = 4,
                .line_size = UNDEF,
                .tlb_entries = 64,
                .partitioning = 0
        };
    case 0xB3:
        return (CacheLevelInfo) {
            .level = UNDEF,
                .cache_type = CPU_FEATURE_CACHE_TLB,
                .cache_size = 4 * KiB,
                .ways = 4,
                .line_size = UNDEF,
                .tlb_entries = 128,
                .partitioning = 0
        };
    case 0xB4:
        return (CacheLevelInfo) {
            .level = UNDEF,
                .cache_type = CPU_FEATURE_CACHE_TLB,
                .cache_size = 4 * KiB,
                .ways = 4,
                .line_size = UNDEF,
                .tlb_entries = 256,
                .partitioning = 0
        };
    case 0xB5:
        return (CacheLevelInfo) {
            .level = UNDEF,
                .cache_type = CPU_FEATURE_CACHE_TLB,
                .cache_size = 4 * KiB,
                .ways = 8,
                .line_size = UNDEF,
                .tlb_entries = 64,
                .partitioning = 0
        };
    case 0xB6:
        return (CacheLevelInfo) {
            .level = UNDEF,
                .cache_type = CPU_FEATURE_CACHE_TLB,
                .cache_size = 4 * KiB,
                .ways = 8,
                .line_size = UNDEF,
                .tlb_entries = 128,
                .partitioning = 0
        };
    case 0xBA:
        return (CacheLevelInfo) {
            .level = UNDEF,
                .cache_type = CPU_FEATURE_CACHE_TLB,
                .cache_size = 4 * KiB,
                .ways = 4,
                .line_size = UNDEF,
                .tlb_entries = 64,
                .partitioning = 0
        };
    case 0xC0:
        return (CacheLevelInfo) {
            .level = UNDEF,
                .cache_type = CPU_FEATURE_CACHE_TLB,
                .cache_size = 4 * KiB,
                .ways = 4,
                .line_size = UNDEF,
                .tlb_entries = 8,
                .partitioning = 0
        };
    case 0xC1:
        return (CacheLevelInfo) {
            .level = UNDEF,
                .cache_type = CPU_FEATURE_CACHE_STLB,
                .cache_size = 4 * KiB,
                .ways = 8,
                .line_size = UNDEF,
                .tlb_entries = 1024,
                .partitioning = 0
        };
    case 0xC2:
        return (CacheLevelInfo) {
            .level = UNDEF,
                .cache_type = CPU_FEATURE_CACHE_DTLB,
                .cache_size = 4 * KiB,
                .ways = 4,
                .line_size = UNDEF,
                .tlb_entries = 16,
                .partitioning = 0
        };
    case 0xC3:
        return (CacheLevelInfo) {
            .level = UNDEF,
                .cache_type = CPU_FEATURE_CACHE_STLB,
                .cache_size = 4 * KiB,
                .ways = 6,
                .line_size = UNDEF,
                .tlb_entries = 1536,
                .partitioning = 0
        };
    case 0xCA:
        return (CacheLevelInfo) {
            .level = UNDEF,
                .cache_type = CPU_FEATURE_CACHE_STLB,
                .cache_size = 4 * KiB,
                .ways = 4,
                .line_size = UNDEF,
                .tlb_entries = 512,
                .partitioning = 0
        };
    case 0xD0:
        return (CacheLevelInfo) {
            .level = 3,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 512 * KiB,
                .ways = 4,
                .line_size = 64,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0xD1:
        return (CacheLevelInfo) {
            .level = 3,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 1 * MiB,
                .ways = 4,
                .line_size = 64,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0xD2:
        return (CacheLevelInfo) {
            .level = 3,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 2 * MiB,
                .ways = 4,
                .line_size = 64,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0xD6:
        return (CacheLevelInfo) {
            .level = 3,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 1 * MiB,
                .ways = 8,
                .line_size = 64,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0xD7:
        return (CacheLevelInfo) {
            .level = 3,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 2 * MiB,
                .ways = 8,
                .line_size = 64,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0xD8:
        return (CacheLevelInfo) {
            .level = 3,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 4 * MiB,
                .ways = 8,
                .line_size = 64,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0xDC:
        return (CacheLevelInfo) {
            .level = 3,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 1 * 1536 * KiB,
                .ways = 12,
                .line_size = 64,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0xDD:
        return (CacheLevelInfo) {
            .level = 3,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 3 * MiB,
                .ways = 12,
                .line_size = 64,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0xDE:
        return (CacheLevelInfo) {
            .level = 3,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 6 * MiB,
                .ways = 12,
                .line_size = 64,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0xE2:
        return (CacheLevelInfo) {
            .level = 3,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 2 * MiB,
                .ways = 16,
                .line_size = 64,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0xE3:
        return (CacheLevelInfo) {
            .level = 3,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 4 * MiB,
                .ways = 16,
                .line_size = 64,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0xE4:
        return (CacheLevelInfo) {
            .level = 3,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 8 * MiB,
                .ways = 16,
                .line_size = 64,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0xEA:
        return (CacheLevelInfo) {
            .level = 3,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 12 * MiB,
                .ways = 24,
                .line_size = 64,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0xEB:
        return (CacheLevelInfo) {
            .level = 3,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 18 * MiB,
                .ways = 24,
                .line_size = 64,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0xEC:
        return (CacheLevelInfo) {
            .level = 3,
                .cache_type = CPU_FEATURE_CACHE_DATA,
                .cache_size = 24 * MiB,
                .ways = 24,
                .line_size = 64,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0xF0:
        return (CacheLevelInfo) {
            .level = UNDEF,
                .cache_type = CPU_FEATURE_CACHE_PREFETCH,
                .cache_size = 64 * KiB,
                .ways = UNDEF,
                .line_size = UNDEF,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0xF1:
        return (CacheLevelInfo) {
            .level = UNDEF,
                .cache_type = CPU_FEATURE_CACHE_PREFETCH,
                .cache_size = 128 * KiB,
                .ways = UNDEF,
                .line_size = UNDEF,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    case 0xFF:
        return (CacheLevelInfo) {
            .level = UNDEF,
                .cache_type = CPU_FEATURE_CACHE_NULL,
                .cache_size = UNDEF,
                .ways = UNDEF,
                .line_size = UNDEF,
                .tlb_entries = UNDEF,
                .partitioning = 0
        };
    default:
        return EmptyCacheLevelInfo;
    }
}

static void
ParseIntelDeterministicTlb(_Out_ PCPU_CACHE_INFO CacheInfo, _In_ const CPUINFO* CpuInfo)
{
    if (!CacheInfo || !CpuInfo) return;
    if (CpuInfo->X86.MaxStandardFunction < 0x18) return;

    for (uint32_t i = 0; CacheInfo->Size < CPU_MAX_CACHE_LEVEL; ++i)
    {
        REGISTER_SET leaf = { 0 };
        _scouse_cpuidex(0x18, i, &leaf.Registers);

        uint32_t type = leaf.Eax & 0x1F;
        if (type == 0) return; // no more translation cache info :contentReference[oaicite:14]{index=14}

        uint32_t level = (leaf.Eax >> 5) & 0x7;

        uint32_t ways = (leaf.Ebx & 0xFFFF) + 1;
        uint32_t parts = ((leaf.Ebx >> 16) & 0xFFFF) + 1;
        uint32_t sets = leaf.Ecx + 1;

        uint32_t entries = ways * parts * sets; // leaf 0x18 defines entries derived from these :contentReference[oaicite:15]{index=15}

        CacheInfo->Levels[CacheInfo->Size] = (CACHE_INFO){
            .Level = (LONG32)level,
            .CacheType = (type == 1) ? CPU_FEATURE_CACHE_DTLB :
                         (type == 2) ? CPU_FEATURE_CACHE_STLB :
                         (type == 3) ? CPU_FEATURE_CACHE_TLB : CPU_FEATURE_CACHE_TLB,
            .Ways = (LONG32)ways,
            .Partitioning = (LONG32)parts,
            .TlbEntries = (LONG32)entries,
            .LineSize = -1,       // not meaningful for TLB
            .CacheSize = -1       // consider repurposing "CacheSize" to "PageSize" for TLB records if you want
        };

        CacheInfo->Size++;
    }
}

VOID
GetCacheInfo(
	_Out_ PCPU_CACHE_INFO CacheInfo,
	_In_  ULONG32    Index
)
{
    if ( !CacheInfo )
	{
		return;
	}

    CPUINFO CpuInfo = { 0 };
	GetCpuInfo( &CpuInfo );

	CacheInfo->Size = 0;

	if ( strstr( CpuInfo.Vendor, CPU_AUTHENTIC_AMD ) )
	{
		// max extended leaf, 0x8000001D
		if( ( 1ll << 22 ) & CpuInfo.X86.Leaf80000001.Ecx )
		{
			ParseCacheInfo(CacheInfo, CpuInfo.X86.MaxExtendedFunction, 0x8000001D );
		}
	}
    else if ( strstr( CpuInfo.Vendor, CPU_GENUINE_INTEL ) )
	{
		// max standard leaf, 0x00000004
		REGISTER_SET Leaf = CpuInfo.X86.Leaf2;

		Leaf.Eax &= 0xFFFFFF00;

		if( Leaf.Eax & ( 1ll << 31 ) )
		{
			Leaf.Eax = 0;
		}
		if( Leaf.Ebx & ( 1ll << 31 ) )
		{
			Leaf.Ebx = 0;
		}
		if( Leaf.Ecx & ( 1ll << 31 ) )
		{
			Leaf.Ecx = 0;
		}
		if( Leaf.Edx & ( 1ll << 31 ) ) 
		{
			Leaf.Edx = 0;
		}

		//todo - finish parsing leaf 2
        BYTE Data[ 16 ];

        static_assert(sizeof(REGISTER_SET) == sizeof(Data), "Leaf size mismatch!");

        memcpy((PBYTE)Data, (PCSTR)&Leaf, sizeof(Data));

        for( ULONG32 Index = 0; Index < sizeof( Data ); ++Index )
        {
            const BYTE CacheDescriptor = Data[ Index ];
            if (CacheDescriptor == 0)
            {
                continue;
            }

            CacheLevelInfo Tmp = GetCacheLevelInfo(CacheDescriptor);

            CacheInfo->Levels[CacheInfo->Size] = (CACHE_INFO){
                    .Level = Tmp.level,
                    .CacheSize = Tmp.cache_size,
                    .CacheType = Tmp.cache_type,
                    .Ways = Tmp.ways,
                    .LineSize = Tmp.line_size,
                    .TlbEntries = Tmp.tlb_entries,
                    .Partitioning = Tmp.partitioning
            };

            CacheInfo->Size++;
        }

        ParseIntelDeterministicTlb( CacheInfo, &CpuInfo );

        ParseCacheInfo(CacheInfo, CpuInfo.X86.MaxStandardFunction, 0x00000004);
	}
}

VOID
ParseCacheInfo(
	_Out_ PCPU_CACHE_INFO CacheInfo,
	_In_ ULONG32 MaxLeaf,
	_In_ ULONG32 LeafId
)
{

	if (!CacheInfo)
	{
		return;
	}

	for( ULONG Index = 0; CacheInfo->Size < CPU_MAX_CACHE_LEVEL; ++Index )
	{
		REGISTER_SET Leaf = { 0 };
        _scouse_cpuidex(LeafId, Index, &Leaf.Registers);

        switch (Leaf.Eax & 0x1F)
		{
			case 0:
			{
				return;
			}
			case 1:
			{
				CacheInfo->Levels[ CacheInfo->Size ].CacheType = CPU_FEATURE_CACHE_DATA;
				break;
			}
			case 2:
			{
				CacheInfo->Levels[ CacheInfo->Size ].CacheType = CPU_FEATURE_CACHE_INSTRUCTION;
				break;
			}
			case 3:
			{
				CacheInfo->Levels[ CacheInfo->Size ].CacheType = CPU_FEATURE_CACHE_UNIFIED;
				break;
			}
			default:
			{
				CacheInfo->Levels[ CacheInfo->Size ].CacheType = CPU_FEATURE_CACHE_NULL;
				break;
			}
		}

        CacheInfo->Levels[ CacheInfo->Size ].Level		  = (LONG32)( ( Leaf.Eax >> 5 ) & 0x7 );
		CacheInfo->Levels[ CacheInfo->Size ].LineSize	  = (LONG32)( ( Leaf.Ebx & 0xFFF ) + 1 );
        CacheInfo->Levels[ CacheInfo->Size ].Ways		  = (LONG32)( ( ( Leaf.Ebx >> 22 ) & 0x3FF ) + 1 );
		CacheInfo->Levels[ CacheInfo->Size ].Partitioning = (LONG32)( ( ( Leaf.Ebx >> 12 ) & 0x3FF ) + 1 );
		CacheInfo->Levels[ CacheInfo->Size ].TlbEntries   = (LONG32)( Leaf.Ecx + 1 );
		CacheInfo->Levels[ CacheInfo->Size ].CacheSize	  = (LONG32)( CacheInfo->Levels[ CacheInfo->Size ].TlbEntries  *
																	  CacheInfo->Levels[ CacheInfo->Size ].Ways *
																	  CacheInfo->Levels[ CacheInfo->Size ].Partitioning *
																	  CacheInfo->Levels[ CacheInfo->Size ].LineSize );

		++CacheInfo->Size;
	}
}