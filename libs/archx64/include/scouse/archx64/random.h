#ifndef ARCHX64_RANDOM_H
#define ARCHX64_RANDOM_H


#include <scouse/archx64/special.h>



/**
* Generates a random unsigned 64-Bit value up to optionally maximum specified value from the rdrand instruction.
* 
* @param Max    Optional maximum value, if 0 or not specified, full 64-Bit range is used. Value specified is exclusive.
* 
* @return A random unsigned 64-Bit value.
*/
ULONG64
RandomValue(
    _In_opt_ ULONG64 Max
);


#endif
