#ifndef KRNL_STRSER_H
#define KRNL_STRSER_H

/*
    Objective of this file is to stess specific points in hardware; ITLB, DTLB, Warm/Cold cache lines, prefetchers etc. This is done while minimising 
    computation to get accurate results.
*/

#include <scouse/shared/typedefs.h>

#define PAGE_SIZE 0x1000
#define NUM_PAGES 1000
#define PAGE_QWORD_SIZE ( PAGE_SIZE / sizeof( ULONG64 ) )


// TODO: Dynamically get page sizes this exe is mapped with (4KiB, 2MiB, 1GiB) and create stressers for each.

// 
//  DATA STRESSERS
//

// make sure array is aligned to page size we are testing
__declspec( align( PAGE_SIZE ) )
ULONG64  LinearPointerChainArray[ PAGE_QWORD_SIZE * NUM_PAGES ];
ULONG64* LinearPointerChainHead;

__declspec( align( PAGE_SIZE ) )
ULONG64  RandomPointerChainArray[ PAGE_QWORD_SIZE * NUM_PAGES ];
ULONG64* RandomPointerChainHead;

/**
* @brief Fills pages with pointers pointing to the next page in a linear fashion.
*/
VOID
FillLinearPointerChain(
    VOID
);

/**
* @brief Walks the linear pointer chain filled by 'FillLinearPointerChain'. Essentially just
* touches each pointer and follows it to the next.
*/
__forceinline
VOID
LinearWalkPointerChain(
    VOID
);

/**
* @brief Fills pages with pointers pointing to a randomly selected page within array of pages. Idea is to minimise prefetching effects.
*/
VOID
FillRandomPointerChain(
    VOID
);

/**
* @brief Walks the random pointer chain filled by 'FillRandomPointerChain'.
*/
__forceinline
VOID
RandomWalkPointerChain(
    VOID
);


//
// INSTRUCTION STRESSERS
//


PBYTE LinearJmpChainBuffer;

typedef VOID(*JMP_STUB)(VOID);

/**
* @brief Fills pages with JMP instructions pointing to the next page in a linear fashion.
*/
VOID
FillLinearJmpChain(
    VOID
);


__forceinline
VOID
LinearExecuteJmpChain(
    VOID
);

#endif //!KRNL_STRSER_H