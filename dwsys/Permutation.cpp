/* Permutation.cpp
 *
 * Copyright (C) 2005-2018 David Weenink
 *
 * This code is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This code is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this work. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 djmw 20050706
 djmw 20050722 Latest modification.
 djmw 20061212 Changed info to Melder_writeLine<x> format.
 djmw 20071012 Added: o_CAN_WRITE_AS_ENCODING.h
 djmw 20100521 Next and Previous
 djmw 20100818 Permutation_permuteTwoItems/Numbers
 djmw 20110304 Thing_new
*/

#include <time.h>
#include "Permutation.h"

#include "oo_DESTROY.h"
#include "Permutation_def.h"
#include "oo_COPY.h"
#include "Permutation_def.h"
#include "oo_EQUAL.h"
#include "Permutation_def.h"
#include "oo_CAN_WRITE_AS_ENCODING.h"
#include "Permutation_def.h"
#include "oo_WRITE_TEXT.h"
#include "Permutation_def.h"
#include "oo_WRITE_BINARY.h"
#include "Permutation_def.h"
#include "oo_READ_BINARY.h"
#include "Permutation_def.h"
#include "oo_DESCRIPTION.h"
#include "Permutation_def.h"

Thing_implement (Permutation, Daata, 0);

#define SWAP(a,b) {integer tmp; tmp = (a); a = b; b = tmp;}

static integer Permutation_checkRange (Permutation me, integer *from, integer *to) {
	if (*from == 0) {
		*from = 1;
	}
	if (*to == 0) {
		*to = my numberOfElements;
	}

	Melder_require (*from > 0 && *from <= my numberOfElements && *to > 0 && *to <= my numberOfElements,
		U"Range should be in [1, ", my numberOfElements, U"].");

	return *to - *from + 1;
}

void Permutation_checkInvariant (Permutation me) {
	autoPermutation thee = Data_copy (me);
	NUMsort_integer (thy numberOfElements, thy p);
	for (integer i = 1; i <= my numberOfElements; i ++) {
		Melder_require (thy p [i] == i, me, U": is not a valid permutation.");
	}
}

void structPermutation :: v_info () {
	structDaata :: v_info ();
	MelderInfo_writeLine (U"Number of elements: ", numberOfElements);
}

void structPermutation :: v_readText (MelderReadText text, int /*formatVersion*/) {
	numberOfElements = texgeti32 (text);
	Melder_require (numberOfElements > 0, U"Number of elements should be greater than zero.");
	
	p = NUMvector_readText_integer32BE (1, numberOfElements, text, "p");
	Permutation_checkInvariant (this);
}

void Permutation_init (Permutation me, integer numberOfElements) {
	my numberOfElements = numberOfElements;
	my p = NUMvector<integer> (1, numberOfElements);
	Permutation_sort (me);
}

void Permutation_tableJump_inline (Permutation me, integer jumpSize, integer first) {
	if (jumpSize >= my numberOfElements || first > my numberOfElements) {
		return;
	}
	autoNUMvector<integer> p_copy (NUMvector_copy<integer> (my p, 1, my numberOfElements), 1);
	integer index = first, column = 1;
	if (first > 1) {
		column = (first - 1) % jumpSize + 1;
	}
	for (integer i = 1; i <= my numberOfElements; i ++) {
		my p [i] = p_copy [index];
		index += jumpSize;
		if (index > my numberOfElements) {
			if (++ column > jumpSize) {
				column = 1;
			}
			index = column;
		}
	}
}

autoPermutation Permutation_create (integer numberOfElements) {
	try {
		autoPermutation me = Thing_new (Permutation);
		Permutation_init (me.get(), numberOfElements);
		return me;
	} catch (MelderError) {
		Melder_throw (U"Permulation not created.");
	}
}

void Permutation_sort (Permutation me) {
	for (integer i = 1; i <= my numberOfElements; i ++) {
		my p [i] = i;
	}
}

void Permutation_swapPositions (Permutation me, integer i1, integer i2) {
	try {
		Melder_require (i1 > 0 && i1 <= my numberOfElements && i2 > 0 && i2 <= my numberOfElements, 
			U"Positions should be within the range [1, ",  my numberOfElements, U"].");
		SWAP (my p [i1], my p [i2])
	} catch (MelderError) {
		Melder_throw (me, U":positions not swapped.");
	}
}

void Permutation_swapNumbers (Permutation me, integer i1, integer i2) {
	try {
		integer ip = 0;
		Melder_require (i1 > 0 && i1 <= my numberOfElements && i2 > 0 && i2 <= my numberOfElements, 
			U"Positions should be within the range [1, ",  my numberOfElements, U"].");
		
		if (i1 == i2) {
			return;
		}
		for (integer i = 1; i <= my numberOfElements; i ++) {
			if (my p [i] == i1) {
				my p [i] = i2;
				ip ++;
			} else if (my p [i] == i2) {
				my p [i] = i1;
				ip ++;
			}
			if (ip == 2) {
				break;
			}
		}
		Melder_assert (ip == 2);
	} catch (MelderError) {
		Melder_throw (me, U": numbers not swapped.");
	}
}

void Permutation_swapBlocks (Permutation me, integer from, integer to, integer blocksize) {
	try {
		Melder_require (blocksize > 0 && blocksize <= my numberOfElements / 2, U"Blocksize should be in [1, %d] range.", my numberOfElements / 2);
		Melder_require (from > 0 && to > 0 && from + blocksize <= my numberOfElements && to + blocksize <= my numberOfElements,
			U"Start and finish positions of the two blocks should be in [1,", my numberOfElements, U"] range.");

		if (from == to) {
			return;
		}

		for (integer i = 1; i <= blocksize; i++) {
			SWAP (my p [from + i - 1], my p [to + i - 1])
		}
	} catch (MelderError) {
		Melder_throw (me, U": blocks not swapped.");
	}
}

void Permutation_permuteRandomly_inplace (Permutation me, integer from, integer to) {
	try {
		integer n = Permutation_checkRange (me, & from, & to);

		if (n == 1) {
			return;
		}
		for (integer i = from; i < to; i ++) {
			integer newpos = NUMrandomInteger (from, to);
			SWAP (my p [i], my p [newpos])
		}
	} catch (MelderError) {
		Melder_throw (me, U": not permuted randomly.");
	}
}

autoPermutation Permutation_permuteRandomly (Permutation me, integer from, integer to) {
	try {
		autoPermutation thee = Data_copy (me);
		Permutation_permuteRandomly_inplace (thee.get(), from, to);
		return thee;
	} catch (MelderError) {
		Melder_throw (me, U": not permuted.");
	}
}

autoPermutation Permutation_rotate (Permutation me, integer from, integer to, integer step) {
	try {
		integer n = Permutation_checkRange (me, & from, & to);
		step = (step - 1) % n + 1;

		autoPermutation thee = Data_copy (me);
		for (integer i = from; i <= to; i ++) {
			integer ifrom = i + step;
			if (ifrom > to) {
				ifrom -= n;
			}
			if (ifrom < from) {
				ifrom += n;
			}
			thy p [ifrom] = my p[i];
		}
		return thee;
	} catch (MelderError) {
		Melder_throw (me, U": not rotated.");
	}
}

void Permutation_swapOneFromRange (Permutation me, integer from, integer to, integer pos, bool forbidsame) {
	try {
		integer n = Permutation_checkRange (me, & from, & to);
		integer newpos = NUMrandomInteger (from, to);
		if (newpos == pos && forbidsame) {
			Melder_require (n != 1, U"Impossible to satisfy \"forbid same\" constraint within the chosen range.");
			
			while ((newpos = NUMrandomInteger (from, to)) == pos) {
				;
			}
		}
		SWAP (my p [pos], my p [newpos])
	} catch (MelderError) {
		Melder_throw (me, U": one from range not swapped.");
	}
}

autoPermutation Permutation_permuteBlocksRandomly (Permutation me, integer from, integer to, integer blocksize, bool permuteWithinBlocks, bool noDoublets) {
	try {
		integer n = Permutation_checkRange (me, & from, & to);
		if (blocksize == 1 || (blocksize >= n && permuteWithinBlocks)) {
			autoPermutation thee = Permutation_permuteRandomly (me, from, to);
			return thee;
		}
		autoPermutation thee = Data_copy (me);
		if (blocksize >= n) {
			return thee;
		}

		integer nblocks  = n / blocksize, nrest = n % blocksize;
		Melder_require (nrest == 0, U"There should fit an integer number of blocks in the range.\n(The last block is only of size ", nrest, U").");
		
		autoPermutation pblocks = Permutation_create (nblocks);

		Permutation_permuteRandomly_inplace (pblocks.get(), 1, nblocks);
		integer first = from;
		for (integer iblock = 1; iblock <= nblocks; iblock ++, first += blocksize) {
			/* (n1,n2,n3,...) means: move block n1 to position 1 etc... */
			integer blocktomove = Permutation_getValueAtIndex (pblocks.get(), iblock);

			for (integer j = 1; j <= blocksize; j ++) {
				thy p [first - 1 + j] = my p [from - 1 + (blocktomove - 1) * blocksize + j];
			}

			if (permuteWithinBlocks) {
				integer last = first + blocksize - 1;
				Permutation_permuteRandomly_inplace (thee.get(), first, last);
				if (noDoublets && iblock > 0 && (thy p [first - 1] % blocksize) == (thy p [first] % blocksize)) {
					Permutation_swapOneFromRange (thee.get(), first + 1, last, first, 0);
				}
			}
		}
		return thee;
	} catch (MelderError) {
		Melder_throw (me, U": not permuted block randomly.");
	}
}

autoPermutation Permutation_interleave (Permutation me, integer from, integer to, integer blocksize, integer offset) {
	try {
		Melder_require (offset < blocksize, U"Offset should be smaller than blocksize.");
		integer n = Permutation_checkRange (me, & from, & to);
		integer nblocks = n / blocksize;
		integer nrest = n % blocksize;
		Melder_require (nrest == 0, U"There should fit an integer number of blocks in the range.\n"
				U"(The last block is only of size ", nrest, U" instead of ", blocksize, U").");
		
		autoPermutation thee = Data_copy (me);

		if (nblocks > 1) {
			autoNUMvector<integer> occupied (1, blocksize);

			integer posinblock = 1 - offset;
			for (integer i = 1; i <= n; i ++) {
				integer index, rblock = (i - 1) % nblocks + 1;

				posinblock += offset;
				if (posinblock > blocksize) {
					posinblock -= blocksize;
				}

				if (i % nblocks == 1) {
					integer count = blocksize;
					while (occupied[posinblock] == 1 && count > 0) {
						posinblock ++; count --;
						if (posinblock > blocksize) {
							posinblock -= blocksize;
						}
					}
					occupied [posinblock] = 1;
				}
				index = from - 1 + (rblock - 1) * blocksize + posinblock;
				thy p [from - 1 + i] = my p [index];
			}
		}
		return thee;
	} catch (MelderError) {
		Melder_throw (me, U": not interleaved.");
	}
}

integer Permutation_getValueAtIndex (Permutation me, integer i) {
	return i > 0 && i <= my numberOfElements ? my p [i] : -1;
}

integer Permutation_getIndexAtValue (Permutation me, integer value) {
	for (integer i = 1; i <= my numberOfElements; i ++) {
		if (my p [i] == value) {
			return i;
		}
	}
	return -1;
}

autoPermutation Permutation_invert (Permutation me) {
	try {
		autoPermutation thee = Data_copy (me);
		for (integer i = 1; i <= my numberOfElements; i ++) {
			thy p [my p [i]] = i;
		}
		return thee;
	} catch (MelderError) {
		Melder_throw (me, U": not inverted.");
	}
}

void Permutation_reverse_inline (Permutation me, integer from, integer to) {
	integer n = Permutation_checkRange (me, & from, & to);
	for (integer i = 1; i <= n / 2; i ++) {
		SWAP (my p [from + i - 1], my p [to - i + 1] )
	}
}

autoPermutation Permutation_reverse (Permutation me, integer from, integer to) {
	try {
		integer n = Permutation_checkRange (me, & from, & to);
		autoPermutation thee = Data_copy (me);
		Permutation_reverse_inline (thee.get(), from, to);
		return thee;
	} catch (MelderError) {
		Melder_throw (me, U": not reversed.");
	}
}

/* Replaces p with the next permutation (in the standard lexicographical ordering.
   Adapted from the GSL library
*/
void Permutation_next_inplace (Permutation me) {
	integer size = my numberOfElements;
	integer *p = & my p [1];

	Melder_require (size > 1, U"The permutation should have more than one element.");
	
	integer i = size - 2;

	while ((p [i] > p [i + 1]) && (i != 0)) {
		i --;
	}

	if ((i == 0) && (p [0] > p [1])) {
		Melder_throw (U"No next.");
	}

	integer k = i + 1;

	for (integer j = i + 2; j < size; j ++) {
		if ((p [j] > p [i]) && (p [j] < p [k])) {
			k = j;
		}
	}
	SWAP (p [i], p [k])

	for (integer j = i + 1; j <= (size + i) / 2; j ++) {
		SWAP (p [j], p [size + i - j])
	}
}

/* Replaces p with the previous permutation (in the standard lexicographical ordering.
   Adapted from the GSL library
*/
void Permutation_previous_inplace (Permutation me) {
	integer size = my numberOfElements;
	integer *p = & my p [1];

	Melder_require (size > 1, U"The permutation should have more than one element.");

	integer i = size - 2;

	while ((p [i] < p [i + 1]) && (i != 0)) {
		i --;
	}
	
	Melder_require (! (i == 0 && p [0] < p [1]), U"No previous element.");

	integer k = i + 1;

	for (integer j = i + 2; j < size; j ++) {
		if ((p [j] < p [i]) && (p [j] > p [k])) {
			k = j;
		}
	}
	
	SWAP (p [i], p [k])

	for (integer j = i + 1; j <= ( (size + i) / 2); j++) {
		SWAP (p [j], p [size + i - j])
	}
}

autoPermutation Permutations_multiply2 (Permutation me, Permutation thee) {
	try {
		Melder_require (my numberOfElements == thy numberOfElements, U"Number of elements should be equal.");
		
		autoPermutation him = Data_copy (me);
		for (integer i = 1; i <= my numberOfElements; i ++) {
			his p [i] = my p [thy p [i]];
		}
		return him;
	} catch (MelderError) {
		Melder_throw (me, U" & ", thee, U" not multiplied.");
	}
}

autoPermutation Permutations_multiply (OrderedOf<structPermutation>* me) {
	try {
	Melder_require (my size > 1, U"There should be at least two Permutations to multiply.");
		autoPermutation thee = Permutations_multiply2 (my at [1], my at [2]);
		for (integer i = 3; i <= my size; i ++) {
			autoPermutation him = Permutations_multiply2 (thee.get(), my at [i]);
			thee = him.move();
		}
		return thee;
	} catch (MelderError) {
		Melder_throw (U"Permutations not multiplied.");
	}
}

#undef SWAP

/* End of Permutation.cpp */
