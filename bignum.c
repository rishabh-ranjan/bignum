/*
 * bignum.c
 * author: Rishabh Ranjan
 * month: January, 2020
 */

#include <stdlib.h>
#include <string.h>

// only for debugging
#include <stdio.h>

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
	unsigned sign; /* +ve for -ve bignum, 0 for +ve, any for 0 */
	unsigned point_offset;
	unsigned num_digits; /* exact size of digits array; leading zeroes allowed */
	digit_t *digits;
};

/*
 * Allocate a bignum with `num_digits' digits.
 * Digits are initialized to 0.
 * Return NULL if allocation failed.
 */
struct bignum *bignum_alloc(unsigned num_digits) {
	struct bignum *ptr = malloc(sizeof(struct bignum));
	if (!ptr) return NULL;
	ptr->digits = calloc(num_digits, sizeof(digit_t));
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
	ret->num_digits = nd;

	int bdc = 0; // bignum digit counter
	int ddc = rhz; // decimal digit counter, handles padding
	for (int i = 0; i < len - neg; ++i) { // skip minus sign
		char ch = str[len - i - 1];
		if (ch == DOT_CHAR) continue; // skip point
		int dd = ch - ZERO_CHAR; // decimal digit
		ret->digits[bdc] += dd * POW10[ddc];
		printf("%d\n", ret->digits[bdc]);
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
char *bignum_to_string(struct bignum *num) {
	// length of returned string
	int len = num->num_digits * RNUM + !num->point_offset + num->sign;
	// char *ret = malloc((len + 1) * sizeof(char)); // +1 for '\0'
	// TODO: use above for non-debug
	char *ret = malloc((len + num->num_digits + 1) * sizeof(char));
	char *iter = ret;
	if (num->sign) *iter++ = MINUS_CHAR;
	for (int i = 0; i < (int)num->num_digits; ++i) {
		if (num->num_digits - i == num->point_offset) {
			// put floating point (if not at the end)
			*iter++ = DOT_CHAR;
		}
		char *mtmp = malloc((RNUM + 2) * sizeof(char));
		char *tmp = mtmp;
		++tmp; // to allow post decrementing the base pointer
		int ctr = 0; // number of digits
		unsigned cpy = num->digits[num->num_digits - i - 1];
		while (cpy) {
			++ctr;
			int d = cpy % TEN;
			// if right-most and non-zero point offset, skip 0s
			if (i == (int)num->num_digits - 1 && num->point_offset && !d) {
				*tmp++ = '\0';
			} else {
				*tmp++ = (char)(d + ZERO_CHAR);
			}
			cpy /= TEN;
		}
		// add zeroes unless left-most
		if (i != 0) {
			for (int j = 0; j < RNUM - ctr; ++j) {
				*iter++ = ZERO_CHAR;
			}
		}
		// add decimal digit chars
		while (ctr--) *iter++ = *--tmp;
		free(mtmp);

		// TODO: remove after debug
		*iter++ = '_';
	}
	*iter = '\0';
	return ret;
}

// - zero bignum
// - trailing zeroes
int main() {
	while (1) {
		char *inp = malloc(1000 * sizeof(char));
		printf("%s\n", "Enter a bignum as string:");
		scanf("%s", inp);
		struct bignum *num = string_to_bignum(inp);
		printf("sign: %d\npoint_offset: %d\nnum_digits: %d\n", num->sign, num->point_offset, num->num_digits);
		char *str = bignum_to_string(num);
		printf("You entered: %s\n", inp);
		printf("I understood: %s\n", str);
	}
}
