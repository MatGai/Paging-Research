#ifndef KRNL_TIMER_H 
#define KRNL_TIMER_H

// have to use compiler instrinics to avoid call/stack overhead if I where to write my own assembly
#include <intrin.h>
#include <scouse/shared/typedefs.h>

//
// Simple timer using RDTSC instruction, using m/l fence to serialise and prevent out of ordering. CPUID is another option but is slower.
//

/**
* Uses RDTSC with lfence and mfence to ensure serialisation (i.e. all previous instructions have completed) as
* otherwise processor might place RDTSC in an unexpected order. Not good if we want semi-reliable cycles.
* 
* @return 64-bit value of TSC
*/
__forceinline
ULONG64 
TscStart(

)
{ 
    _mm_mfence();
    _mm_lfence();
    return __rdtsc();
}

/**
* Uses RDTSCP which is instrinsically serialised, however does not serialise
* later instructions hence mfence is run after.
* 
* @return 64-bit value of TSC
*/
__forceinline
ULONG64 
TscEnd(

)
{
    ULONG32 T;
    ULONG64 R = __rdtscp( &T ); // still don't know what this paramter is
    _mm_lfence();
    return R;

}


#endif // !KRNL_TIMER_H 

