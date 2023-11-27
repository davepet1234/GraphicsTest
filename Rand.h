/*
 * File:    Rand.h
 * 
 * Author:  David Petrovic
 *
 * Description:
 * 
 * EDK2 Pseudo-random number generator
 */

#ifndef RAND_H
#define RAND_H

#include <Uefi.h>

/* The largest number rand will return  */
#define	RAND_MAX    MAX_INT32

/* Return a random integer between 0 and RAND_MAX inclusive */
extern INT32 Rand(VOID);

/* Seed the random number generator with the given number */
extern VOID Srand(UINT32 seed);

#endif // RAND_H
