/*
 * bignum.c
 * author: Rishabh Ranjan
 * month: January, 2020
 */

#include <stdlib.h>

/* Radix for bignum representation = 1e9 */
#define RADIX 1000000000

/* Ensure digit_t is at least 32 bits; `unsigned' is not strictly portable here. */
typedef unsigned digit_t;

/*
 * Represents arbitrary precision real number.
 * Uses sign-magnitude form.
 */
struct bignum {
	unsigned sign; /* +ve for -ve bignum, 0 for +ve, any for 0 */
	unsigned point_offset;
	unsigned num_digits; /* exact size of digits array; leading zeroes allowed */
	digit_t *digits;
};

/*
 * Allocate a bignum with `num_digits' digits.
 * Return NULL if allocation failed.
 */
struct bignum *bignum_alloc(unsigned num_digits) {
	bignum *ptr = malloc(sizeof(struct bignum));
	if (!ptr) return NULL;
	ptr->digits = malloc(num_digits * sizeof(digit_t));
	if (!ptr->digits) return NULL;
	return ptr;
}

/*
 * Free the memory of a bignum.
 */
void bignum_free(struct bignum *ptr) {
	free(ptr->digits);
	free(ptr);
}
