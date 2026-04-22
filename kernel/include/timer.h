#ifndef KRNL_TIMER_H 
#define KRNL_TIMER_H

// have to use compiler instrinics to avoid call/stack overhead if I where to write my own assembly
//#include <intrin.h>
#include <scouse/shared/typedefs.h>
#include <scouse/shared/cpuinfo.h>

//
// Simple timer using RDTSC instruction, using m/l fence to serialise and prevent out of ordering. CPUID is another option but is slower.
//

/**
* Uses RDTSC with lfence and mfence to ensure serialisation (i.e. all previous instructions have completed) as
* otherwise processor might place RDTSC in an unexpected order. Not good if we want semi-reliable cycles.
* 
* @return 64-bit value of TSC
* 
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
    ULONG64 R = __rdtsc( ); // __rdtscp not supported on many systems so not worth it!
    _mm_lfence();
    return R;
}

__forceinline
ULONG64 
TscFrequency(

)
{
    REGISTER_SET Regs15;
    __cpuidex( Regs15.Registers, 0x15, 0 );

    if(Regs15.Eax && Regs15.Ebx && Regs15.Ecx )
    {
        return (ULONG64)Regs15.Ecx * (ULONG64)Regs15.Ebx / (ULONG64)Regs15.Eax;;
    }

    REGISTER_SET Regs16;
    __cpuidex( Regs16.Registers, 0x16, 0 );

    LONG32 BaseMhz = (LONG32)(Regs16.Eax & 0xFFFF);
    if( BaseMhz )
    {
        return (ULONG64)BaseMhz * 1000000ull;
    }

    return 0;
}


#endif // !KRNL_TIMER_H 

