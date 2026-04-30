#include <scouse/archx64/random.h>
#include <intrin.h>

ULONG64
RandomValue(
    _In_opt_ ULONG64 Max
)
{
    if (Max == 0)
    {
        return 0;
    }

    ULONG64 RandomValue = 0;

    if ( ( Max & ( Max - 1 ) ) == 0 ) 
    {
        
        while (1)
        {
            if (!_scouse_rdrand64_step(&RandomValue))
            {
                continue;
            }
            return RandomValue & (Max - 1);
        }
    }

   ULONG64 Threshold = (ULONG64)(0 - Max) % Max;
    

    while (1)
    {
        if (!_scouse_rdrand64_retry(&RandomValue, 10))
        {
            return MAXULONG64;
        }

        ULONG64 High, Low;

        Low = _umul128(RandomValue, Max, &High);

        if (Low >= Threshold)
        {
            return High;
        }
    }
}