#include "strser.h"
#include <scouse/archx64/random.h>

VOID
FillLinearPointerChain(
    VOID
)
{
    LinearPointerChainHead = &LinearPointerChainArray[0];

    for (LONG Index = 0; Index < NUM_PAGES - 1; ++Index )
    {
        LinearPointerChainArray[ Index * PAGE_QWORD_SIZE ] = (ULONG64)&LinearPointerChainArray[ ( Index + 1 ) * PAGE_QWORD_SIZE ];
    }

    LinearPointerChainArray[ (NUM_PAGES - 1) * PAGE_QWORD_SIZE ] = (ULONG64)&LinearPointerChainArray[0];
}

__forceinline
VOID
LinearWalkPointerChain(
    VOID
)
{
    ULONG64* P = LinearPointerChainHead;
    for( LONG Index = 0; Index < ( NUM_PAGES * PAGE_QWORD_SIZE ); ++Index )
    {
        P = (ULONG64*)*P;
    }
}

VOID
FillRandomPointerChain(
    VOID
)
{
    ULONG64 Offset = RandomValue(511);
    Offset = Offset - ( Offset % 8 );

    RandomPointerChainHead = &RandomPointerChainArray[0];

    for (LONG Index = 0; Index < (NUM_PAGES * PAGE_QWORD_SIZE); ++Index)
    {
        RandomPointerChainArray[Index * PAGE_QWORD_SIZE] = (ULONG64)&RandomPointerChainArray[(Index + 1) * PAGE_QWORD_SIZE];
    }

    RandomPointerChainArray[(NUM_PAGES - 1) * PAGE_QWORD_SIZE] = (ULONG64)&RandomPointerChainArray[0];
}