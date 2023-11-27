/*
 * File:    Rand.c
 * 
 * Author:  David Petrovic
 * 
 * Description:
 * 
 * EDK2 Pseudo-random number generator
 * 
 * Ref: https://opensource.apple.com/source/Libc/Libc-1353.11.2/stdlib/FreeBSD/rand.c
 */
 
#include <Uefi.h>
#include "Rand.h"

STATIC UINT32 next = 1;

INT32 Rand(void)
{
/*
 * Compute x = (7^5 * x) mod (2^31 - 1)
 * without overflowing 31 bits:
 *      (2^31 - 1) = 127773 * (7^5) + 2836
 * From "Random number generators: good ones are hard to find",
 * Park and Miller, Communications of the ACM, vol. 31, no. 10,
 * October 1988, p. 1195.
 */
    INT32 hi, lo, x;

    /* Can't be initialized with 0, so use another value. */
    if (next == 0) {
        next = 123459876;
    }
    hi = next / 127773;
    lo = next % 127773;
    x = 16807 * lo - 2836 * hi;
    if (x < 0) {
        x += 0x7fffffff;
    }
    return ((next = x) % ((UINT32)RAND_MAX + 1));
}

void Srand(UINT32 seed)
{
    next = seed;
}
