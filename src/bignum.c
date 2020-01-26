/*
 * bignum.c
 * author: Rishabh Ranjan
 * month: January, 2020
 */

#include <stdlib.h>
#include <string.h>

// #define DEBUG

// #ifdef DEBUG
#include <stdio.h>
// #endif

/* Radix for bignum representation = 1e9 */
#define RADIX 1000000000
/* Number of decimal digits per bignum digit */
#define RNUM 9

/* Ensure digit_t is at least 32 bits; `unsigned' is not strictly portable here. */
typedef unsigned digit_t;

/*
 * Represents arbitrary precision real number.
 * Uses sign-magnitude form.
 */
struct bignum {
	int sign; /* +ve for -ve bignum, 0 for +ve, any for 0 */
	int point_offset;
	int num_digits; /* exact size of digits array; leading zeroes allowed */
	digit_t *digits;
};

/*
 * Allocate a bignum with `num_digits' digits.
 * Also set its `num_digits' field.
 * Digits are initialized to 0.
 * Return NULL if allocation failed.
 */
struct bignum *bignum_alloc(int num_digits) {
	struct bignum *ptr = malloc(sizeof(struct bignum));
	if (!ptr) return NULL;
	ptr->digits = calloc(num_digits, sizeof(digit_t));
	if (!ptr->digits) return NULL;
	ptr->num_digits = num_digits;
	return ptr;
}

/*
 * Free the memory of a bignum.
 */
void bignum_free(struct bignum *ptr) {
	free(ptr->digits);
	free(ptr);
}

const unsigned POW10[] = {
	1,
	10,
	100,
	1000,
	10000,
	100000,
	1000000,
	10000000,
	100000000,
	1000000000 // 1e9
};

#define MINUS_CHAR '-'
#define DOT_CHAR '.'
#define ZERO_CHAR '0'

/*
 * Convert string to bignum.
 * Return NULL on error.
 */
struct bignum *string_to_bignum(char *str) {
	int len = strlen(str);
	int neg = *str == MINUS_CHAR; // is it negative?

	char *dot = strchr(str, DOT_CHAR);
	int flt = dot != NULL; // is floating point present
	int ofs;
	if (flt) ofs = len - (dot - str) - 1;
	else ofs = 0;
	// number of zeroes to be added to the right for alignment
	int rhz = RNUM - ofs % RNUM;
	if (rhz == RNUM) rhz = 0;
	ofs = ofs / RNUM + (ofs % RNUM != 0); // offset of floating point

	int ndw; // number of digits in the whole part
	if (dot) {
		ndw = dot - str - neg;
	} else {
		ndw = len - neg;
	}
	ndw = ndw / RNUM + (ndw % RNUM != 0);
	int nd = ndw + ofs; // number of bignum digits

	struct bignum *ret = bignum_alloc(nd);
	if (!ret) return NULL;
	ret->sign = neg;
	ret->point_offset = ofs;

	int bdc = 0; // bignum digit counter
	int ddc = rhz; // decimal digit counter, handles padding
	for (int i = 0; i < len - neg; ++i) { // skip minus sign
		char ch = str[len - i - 1];
		if (ch == DOT_CHAR) continue; // skip point

#ifdef DEBUG
		if (ch == '_') continue;
#endif

		int dd = ch - ZERO_CHAR; // decimal digit
		ret->digits[bdc] += dd * POW10[ddc];
		if (ddc == RNUM - 1) {
			ddc = 0;
			++bdc;
		} else {
			++ddc;
		}
	}

	return ret;
}

/*
 * Convert bignum to string.
 * Return NULL on error.
 */
#define TEN 10
#define ZERO_STRING "0"
char *bignum_to_string(struct bignum *num) {
	int ofsdi = num->point_offset - 1; // offset digit index; . is to its left
	int fnzdi; // first non-zero digit index
	for (fnzdi = 0; fnzdi < num->num_digits && !num->digits[fnzdi]; ++fnzdi);
	if (fnzdi == num->num_digits) return ZERO_STRING;
	int lnzdi; // last non-zero digit index
	for (lnzdi = num->num_digits - 1; lnzdi >= 0 && !num->digits[lnzdi]; --lnzdi);

	// length of returned string
	int len = num->num_digits * RNUM + !num->point_offset + num->sign;
	// uses some extra memory, but that's ok
#ifdef DEBUG
	char *ret = malloc((len + num->num_digits + 2) * sizeof(char));
#else
	char *ret = malloc((len + 2) * sizeof(char)); // +2 for '\0' and 0.
#endif
	char *iter = ret;

	if (num->sign) *iter++ = MINUS_CHAR;

	// display 0 digits left or right of point if all non-zero digits lie on one side
	if (fnzdi > ofsdi + 1) {
		fnzdi = ofsdi + 1;
	}
	if (lnzdi <= ofsdi) {
		lnzdi = ofsdi;
		*iter++ = ZERO_CHAR; // 0.1 instead of .1
	}

	for (int di = lnzdi; di >= fnzdi; --di) {
		if (di == ofsdi) {
			// put floating point (if not at the end)
			*iter++ = DOT_CHAR;
		}

		char *mtmp = malloc((RNUM + 2) * sizeof(char));
		char *tmp = mtmp;
		++tmp; // to allow post decrementing the base pointer
		int ctr = 0; // number of digits
		unsigned cpy = num->digits[di];
		while (cpy) {
			++ctr;
			*tmp++ = (char)(cpy % TEN + ZERO_CHAR);
			cpy /= TEN;
		}
		// add zeroes unless left-most and left of point
		if (di != lnzdi || di <= ofsdi) {
			for (int j = 0; j < RNUM - ctr; ++j) {
				*iter++ = ZERO_CHAR;
			}
		}
		// add decimal digit chars
		while (ctr--) *iter++ = *--tmp;
		free(mtmp);
#ifdef DEBUG
		*iter++ = '_';
#endif
	}
	// remove extra zeroes after point
	if (fnzdi <= ofsdi) {
#ifdef DEBUG
		for (--iter; *iter == ZERO_CHAR || *iter == '_'; --iter);
#else
		while (*--iter == ZERO_CHAR);
#endif
		++iter;
	}
	*iter = '\0';
	return ret;
}

/*
 * Get digit at position `ind' from `num'.
 * Position 0 is just left of point.
 * +ve, -ve positions even outside those stored are supported.
 */
digit_t get_digit(struct bignum *num, int ind) {
	int di = ind + num->point_offset;
	if (di < 0 || di >= num->num_digits) return 0;
	return num->digits[di];
}

/*
 * Add ignoring sign.
 * TODO: learn how to free returned bignum later.
 * TODO: check error condition of bignum_malloc.
 * todos apply to other functions as well.
 */
#define max(x, y) ((x) > (y) ? (x) : (y))
struct bignum *add_unsigned(struct bignum *a, struct bignum *b) {
	int rofs = max(a->point_offset, b->point_offset); // resulting point offset
	// resulting number of digits in whole number part
	int rwnd = 1 + max(a->num_digits - a->point_offset, b->num_digits - b->point_offset);
	struct bignum *ret = bignum_alloc(rwnd + rofs);
	ret->point_offset = rofs;
	digit_t carry = 0;
	for (int i = -rofs; i < rwnd; ++i) { // last carry included
		digit_t tmp = get_digit(a, i) + get_digit(b, i) + carry;
		if (tmp >= RADIX) {
			tmp -= RADIX;
			carry = 1;
		} else {
			carry = 0;
		}
		ret->digits[i + rofs] = tmp;
	}
	return ret;
}

/*
 * Subtract ignoring sign.
 * Use for a >= b.
 * Return a - b.
 */
struct bignum *sub_unsigned(struct bignum *a, struct bignum *b) {
	int rofs = max(a->point_offset, b->point_offset); // resulting point offset
	// resulting number of digits in whole number part, assuming a >= b
	int rwnd = a->num_digits - a->point_offset;
	struct bignum *ret = bignum_alloc(rwnd + rofs);
	ret->point_offset = rofs;
	digit_t borrow = 0;
	for (int i = -rofs; i < rwnd; ++i) {
		digit_t da = get_digit(a, i);
		if (borrow) {
			if (da) {
				--da;
				borrow = 0;
			} else {
				da = RADIX - 1;
				borrow = 1;
			}
		}
		digit_t db = get_digit(b, i);
		digit_t tmp;
		if (da >= db) {
			tmp = da - db; // do not add borrow = 0 here
		} else {
			tmp = da + RADIX - db;
			borrow = 1;
		}
		ret->digits[i + rofs] = tmp;
	}
	return ret;
}

int main() {
	while (1) {
		///* PARSE
		char *inp = malloc(1000 * sizeof(char));
		printf("%s\n", "Enter a bignum as string:");
		scanf("%s", inp);
		struct bignum *num = string_to_bignum(inp);
		printf("digit[0] = %d\n", num->digits[0]);
		printf("sign: %d\npoint_offset: %d\nnum_digits: %d\n", num->sign, num->point_offset, num->num_digits);
		char *str = bignum_to_string(num);
		printf("You entered: %s\n", inp);
		printf("I understood: %s\n", str);

		/* ADD SUB
		char *ia = malloc(1000 * sizeof(char));
		char *ib = malloc(1000 * sizeof(char));
		printf("a:\n");
		scanf("%s", ia);
		printf("b:\n");
		scanf("%s", ib);
		struct bignum *a = string_to_bignum(ia);
		struct bignum *b = string_to_bignum(ib);
		struct bignum *radd = add_unsigned(a, b);
		struct bignum *rsub = sub_unsigned(a, b);
		char *iadd = bignum_to_string(radd);
		char *isub = bignum_to_string(rsub);
		printf("add:\n%s\n", iadd);
		printf("sub:\n%s\n", isub);
		*/
	}
}
