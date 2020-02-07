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
//
// TODO: use static to qualify some functions and global variables

/* Radix for bignum representation = 1e9 */
#define RADIX 1000000000
//#define RADIX 10
/* Number of decimal digits per bignum digit */
#define RNUM 9
//#define RNUM 1

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
 * sign and point_offset are initialized to 0.
 * Return NULL if allocation failed.
 */
struct bignum *bignum_alloc(int num_digits) {
	struct bignum *ptr = malloc(sizeof(struct bignum));
	if (!ptr) return NULL;
	ptr->digits = calloc(num_digits, sizeof(digit_t));
	if (!ptr->digits) return NULL;
	ptr->num_digits = num_digits;
	ptr->sign = 0;
	ptr->point_offset = 0;
	return ptr;
}

/*
 * Free the memory of a bignum.
 */
void bignum_free(struct bignum *ptr) {
	free(ptr->digits);
	free(ptr);
}

/*
 * Clone a bignum.
 * TODO: Make rem in div and sqrt use this.
 */
struct bignum *clone(struct bignum *num) {
	struct bignum *ret = bignum_alloc(num->num_digits);
	ret->sign = num->sign;
	ret->point_offset = num->point_offset;
	for (int i = 0; i < num->num_digits; ++i) {
		ret->digits[i] = num->digits[i];
	}
	return ret;
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
struct bignum *string_to_bignum(const char *str) {
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
char *bignum_to_string(const struct bignum *num) {
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
digit_t get_digit(const struct bignum *num, int ind) {
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
struct bignum *add_unsigned(const struct bignum *a, const struct bignum *b) {
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
struct bignum *sub_unsigned(const struct bignum *a, const struct bignum *b) {
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

/*
 * Return -ve if |a| < |b|, 0 if |a| == |b|, +ve if |a| > |b|.
 */
int mag_comp(const struct bignum *a, const struct bignum *b) {
	int rofs = max(a->point_offset, b->point_offset); // resulting point offset
	// resulting number of digits in whole number part
	int rwnd = max(a->num_digits - a->point_offset, b->num_digits - b->point_offset);

	// do not compare sizes, as leading digits may be zero
	// avoid unsigned subtraction
	for (int i = rwnd - 1; i >= -rofs; --i) {
		digit_t da = get_digit(a, i);
		digit_t db = get_digit(b, i);
		if (da < db) return -1;
		if (da > db) return 1;
	}
	return 0;
}

// TODO: explore const parameters
/*
 * Signed add if `sub' = 0, signed sub if `sub' = 1.
 */
struct bignum *addsub_signed(const struct bignum *a, const struct bignum *b, int sub) {
	int sa = a->sign;
	int sb = b->sign ^ sub;
	struct bignum *ret;
	if (sa == sb) {
		ret = add_unsigned(a, b);
		ret->sign = sa;
	} else {
		if (mag_comp(a, b) > 0) {
			ret = sub_unsigned(a, b);
			ret->sign = sa;
		} else {
			ret = sub_unsigned(b, a);
			ret->sign = sb;
		}
	}
	return ret;
}

/*
 * Return a * b (signed).
 * Use long multiplication.
 */
typedef unsigned long long lldigit_t;
struct bignum *long_mul(const struct bignum *a, const struct bignum *b) {
	struct bignum *ret = bignum_alloc(a->num_digits + b->num_digits);
	ret->sign = a->sign ^ b->sign;
	ret->point_offset = a->point_offset + b->point_offset;
	for (int ai = 0; ai < a->num_digits; ++ai) {
		lldigit_t carry = 0;
		for (int bi = 0; bi < b->num_digits; ++bi) {
			lldigit_t tmp = (lldigit_t)a->digits[ai] * b->digits[bi];
			tmp += ret->digits[ai + bi] + carry;
			ret->digits[ai + bi] = tmp % RADIX;
			carry = tmp / RADIX;
		}
		ret->digits[ai + b->num_digits] = carry;
	}
	return ret;
}

/*
 * Return signed a/b to `PRECISION' bignum digits of precision after point.
 * Use long division.
 * Return NULL when b = 0.
 * TODO: free all newly malloc'ed structs
 * TODO: report div by 0
 */
#define PRECISION 20
struct bignum *long_div(const struct bignum *a, const struct bignum *b) {
	int lzb; // number of leading zeroes in b
	for (lzb = 0; lzb < b->num_digits && !b->digits[b->num_digits - 1 - lzb]; ++lzb);
	if (lzb == b->num_digits) {
		// division by zero
		return NULL;
	}
	// number of digits in b ignoring leading zeroes
	// important for deciding the leading digit place of quotient
	int bnd = b->num_digits - lzb;
	// number of zeroes appended to a for precision
	int naz = PRECISION + b->point_offset - a->point_offset;
	struct bignum *ret = bignum_alloc(a->num_digits + naz - bnd + 1);
	ret->sign = a->sign ^ b->sign;
	ret->point_offset = PRECISION;
	// intermediate remainder at each step
	struct bignum *rem = bignum_alloc(a->num_digits + naz + 1);
	// initialize remainder
	for (int i = 0; i < a->num_digits; ++i) {
		rem->digits[naz + i] = a->digits[i];
	}
	// hide other digits by shifting pointer
	rem->digits += a->num_digits + naz - bnd + 1;
	// the number of digits of remainder under consideration at every step
	rem->num_digits = bnd + 1;
	// digit being checked at each step
	struct bignum *dig = bignum_alloc(1);
	// copy of b wihtout it's point_offset
	struct bignum *bcpy = malloc(sizeof(struct bignum));
	bcpy->point_offset = 0;
	bcpy->num_digits = bnd;
	bcpy->digits = b->digits;
	for (int ai = a->num_digits - bnd; ai >= -naz; --ai) {
		// expose a new digit of rem
		--rem->digits;
		// binary search next digit of quotient
		// TODO: make it digit_t
		int lo = 0, hi = RADIX - 1;	
		int mid;
		struct bignum *isub; // intermediate subtraction result
		while (hi >= lo) {
			mid = (lo + hi)/2;
			dig->digits[0] = mid;
			struct bignum *tmp = long_mul(dig, bcpy);
			int comp = mag_comp(tmp, rem);
			if (comp <= 0) {
				isub = sub_unsigned(rem, tmp);
				if (mag_comp(isub, bcpy) < 0) {
					break;
				} else {
					lo = mid + 1;
				}
			} else {
				hi = mid - 1;
			}
		}
		ret->digits[ai + naz] = mid;
		// overwrite rem with isub
		for (int ri = 0; ri < rem->num_digits; ++ri) {
			rem->digits[ri] = isub->digits[ri];
		}
	}
	return ret;
}

/*
 * Return sqrt a to `PRECISION' bignum digits of precision.
 * Ignore sign.
 */
// TODO: there is too much memory leak as of now!
// TODO: error checking
// TODO: can bignum_to_string handle num_digits = 0?
// does .00000000000000000000000 break it?
// TODO: make precision a parameter
struct bignum *trim_fraction(struct bignum *num, int precision) {
	fprintf(stderr, "trimming");
	struct bignum *ret = bignum_alloc(num->num_digits - num->point_offset + precision);
	ret->sign = num->sign;
	ret->point_offset = precision;
	for (int i = 0; i < ret->num_digits; ++i) {
		ret->digits[i] = num->digits[i + num->point_offset - precision];
	}
	return ret;
}

struct bignum *sqrt_unsigned(struct bignum *a) {
	fprintf(stderr, "sqrt_u <- %s\n", bignum_to_string(a));
	if (a->point_offset > PRECISION*2) {
		trim_fraction(a, PRECISION*2);
	}
	// sz is number of digits in a after adding zeroes left and right
	// to get paired digits that give required precision
	// sz is even
	int wnd = a->num_digits - a->point_offset; // digits in whole number part
	int sz = wnd + (wnd & 1) + PRECISION*2;
	struct bignum *ret = bignum_alloc(sz/2); // answer
	// hide all digits of ret
	ret->digits += sz/2;
	ret->num_digits = 0;
	struct bignum *rem = bignum_alloc(sz + 1); // intermediate remainder
	int naz = PRECISION*2 - a->point_offset; // number of zeroes added for precision
	// initialize remainder
	for (int i = 0; i < a->num_digits; ++i) {
		rem->digits[i + naz] = a->digits[i];
	}
	// hide all digits of rem
	rem->digits += sz;
	rem->num_digits = 1;
	struct bignum *dig = bignum_alloc(1); // placeholder for small numbers
	struct bignum *rad = bignum_alloc(2); // bignum representation of RADIX
	rad->digits[1] = 1;
	for (int i = 0; i < sz/2; ++i) { // an iteration for each digit in ret
		// expose 2 digits of rem
		rem->digits -= 2;
		rem->num_digits += 2;
		// binary search the digit
		digit_t lo = 0, hi = RADIX - 1;
		digit_t el; // will store max x s.t. (2*ret*RADIX + x)*x <= rem
		struct bignum *tmpel; // store tmp for the el
		while (hi >= lo) {
			digit_t mid = (lo + hi)/2;
			dig->digits[0] = 2;
			struct bignum *tmp1 = long_mul(dig, ret);
			struct bignum *tmp2 = long_mul(rad, tmp1);
			dig->digits[0] = mid;
			struct bignum *tmp3 = add_unsigned(dig, tmp2);
			struct bignum *tmp = long_mul(dig, tmp3);
			// final result: tmp = (2*ret*RADIX + mid)*mid

			/*
			printf("ret:\n%s\n", bignum_to_string(ret));
			printf("mid:\n%s\n", bignum_to_string(dig));
			printf("tmp:\n%s\n", bignum_to_string(tmp));
			printf("rem:\n%s\n", bignum_to_string(rem));
			*/

			int comp = mag_comp(tmp, rem);
			if (comp <= 0) {
				el = mid;
				tmpel = bignum_alloc(tmp->num_digits);
				for (int j = 0; j < tmp->num_digits; ++j) {
					tmpel->digits[j] = tmp->digits[j];
				}
				lo = mid + 1;
			} else {
				hi = mid - 1;
			}
		}
		// add el to result
		--ret->digits;
		++ret->num_digits;
		ret->digits[0] = el;
		// update the remainder by overwriting with rem - tmpel
		struct bignum *sub = sub_unsigned(rem, tmpel);
		for (int j = 0; j < rem->num_digits; ++j) {
			rem->digits[j] = sub->digits[j];
		}
	}
	ret->point_offset = PRECISION;
	return ret;
}

/*
 * Raise to small exponents.
 * Return a ^ b.
 * Ignore sign of a, b expected to be non-negative.
 */
// TODO: why does 1 ^ 10000 in base 10 take so much time
struct bignum *pow_small(struct bignum *a, int b) {
	// TODO: make a global single digit bignum (?)
	// TODO: replace raw instantiations to use bignum_alloc
	fprintf(stderr, "obo");
	struct bignum *ret = bignum_alloc(1);
	ret->digits[0] = 1;
	struct bignum *tmp = clone(a);
	while (b) {
		fprintf(stderr, "b = %d\n", b);
		if (b & 1) {
			ret = long_mul(ret, tmp);
		}
		tmp = long_mul(tmp, tmp);
		b /= 2;
	}
	fprintf(stderr, "tkt");
	return ret;
}

/*
 * Raise to arbitrary integer exponents (i.e. ignoring point_offset).
 * Return a ^ b.
 * Ignore signs of a and b.
 */
struct bignum *pow_uint(struct bignum *a, struct bignum *b) {
	fprintf(stderr, "tada");
	struct bignum *ret = bignum_alloc(1);
	ret->digits[0] = 1;
	struct bignum *acc = clone(a);
	for (int i = 0; i < b->num_digits; ++i) {
		struct bignum *tmp = pow_small(acc, b->digits[i]);
		ret = long_mul(ret, tmp);
		if (i != b->num_digits - 1) {
			acc = pow_small(acc, RADIX);
		}
	}
	return ret;
}

/*
 * Raise to unsigned fractional exponent.
 * Return a ^ b.
 * Ignore signs of a and b.
 * Works for 0 <= b < 1.
 */
struct bignum *pow_ufrac(struct bignum *a, double b) {
	fprintf(stderr, "b = %lf\n", b);
	struct bignum *ret = bignum_alloc(1);
	ret->digits[0] = 1;
	struct bignum *tmp = clone(a);
	while (b) {
		tmp = sqrt_unsigned(tmp);
		b *= 2;
		int d = (int)b;
		if (d) {
			ret = long_mul(ret, tmp);
		}
		b -= d;
	}
	return ret;
}

/*
 * Raise power to signed ints.
 * Return a ^ b.
 * Sign of a is ignored.
 */
struct bignum *pow_sint(struct bignum *a, struct bignum *b) {
	struct bignum *ret = pow_uint(a, b);
	if (b->sign) {
		struct bignum *dig = bignum_alloc(1);
		dig->digits[0] = 1;
		ret = long_div(dig, ret);
	}
	return ret;
}

/*
 * Raise power to signed fractions.
 * Return a ^ b.
 * Sign of a is ignored.
 */
struct bignum *pow_sfrac(struct bignum *a, double b) {
	struct bignum *ret = pow_ufrac(a, b < 0.0 ? -b : b);
	if (b < 0) {
		struct bignum *dig = bignum_alloc(1);
		dig->digits[0] = 1;
		ret = long_div(dig, ret);
	}
	return ret;
}

/*
 * Raise to arbitrary bignum powers.
 * Only one bignum digit to the right of point is considered.
 */
struct bignum *long_pow(struct bignum *a, struct bignum *b) {
	struct bignum *c;
	double d;
	// extract integer and fractional parts
	if (b->point_offset == 0) {
		c = b;
		d = 0;
	} else {
		c = trim_fraction(b, 0);
		d = (double)b->digits[b->point_offset - 1] / RADIX;
		if (b->sign) d = -d;
	}
	fprintf(stderr, "here");
	fprintf(stderr, "c = %s\n", bignum_to_string(c));
	struct bignum *tmp1 = pow_sint(a, c);
	fprintf(stderr, "woo");
	// TODO: can be optimized for when d = 0
	struct bignum *tmp2 = pow_sfrac(a, d);
	fprintf(stderr, "tmp1: %s\n", bignum_to_string(tmp1));
	fprintf(stderr, "tmp2: %s\n", bignum_to_string(tmp2));
	struct bignum *ret = long_mul(tmp1, tmp2);
	return ret;
}

int main() {
	while (1) {
		/* PARSE
		char *inp = malloc(1000 * sizeof(char));
		printf("%s\n", "Enter a bignum as string:");
		scanf("%s", inp);
		struct bignum *num = string_to_bignum(inp);
		printf("digit[0] = %d\n", num->digits[0]);
		printf("sign: %d\npoint_offset: %d\nnum_digits: %d\n", num->sign, num->point_offset, num->num_digits);
		char *str = bignum_to_string(num);
		printf("You entered: %s\n", inp);
		printf("I understood: %s\n", str);
		*/

#if 1
		char *ia = malloc(1000 * sizeof(char));
		char *ib = malloc(1000 * sizeof(char));
		//int c;
		//double d;
		printf("a:\n");
		scanf("%s", ia);
		printf("b:\n");
		scanf("%s", ib);
		//printf("c:\n");
		//scanf("%d", &c);
		//printf("d:\n");
		//scanf("%lf", &d);
		struct bignum *a = string_to_bignum(ia);
		struct bignum *b = string_to_bignum(ib);
		/*
		struct bignum *radd = add_unsigned(a, b);
		struct bignum *rsub = sub_unsigned(a, b);
		struct bignum *rsig = addsub_signed(a, b, 1);
		char *iadd = bignum_to_string(radd);
		char *isub = bignum_to_string(rsub);
		char *isig = bignum_to_string(rsig);
		printf("add:\n%s\n", iadd);
		printf("sub:\n%s\n", isub);
		printf("sig:\n%s\n", isig);
		struct bignum *rmul = long_mul(a, b);
		char *imul = bignum_to_string(rmul);
		struct bignum *rdiv = long_div(a, b);
		char *idiv = bignum_to_string(rdiv);
		printf("div:\n%s\n", idiv);
		struct bignum *rsqrt = sqrt_unsigned(a);
		char *isqrt = bignum_to_string(rsqrt);
		printf("sqrt:\n%s\n", isqrt);
		struct bignum *rpow = pow_uint(a, b);
		char *ipow = bignum_to_string(rpow);
		printf("pow:\n%s\n", ipow);
		*/
		/*
		struct bignum *rroot = root_small(a, c);
		char *iroot = bignum_to_string(rroot);
		printf("root:\n%s\n", iroot);
		*/
		struct bignum *rpow = long_pow(a, b);
		char *ipow = bignum_to_string(rpow);
		printf("pow:\n%s\n", ipow);
		/*
		struct bignum *rdpow = pow_ufrac(a, d);
		char *idpow = bignum_to_string(rdpow);
		printf("pow:\n%s\n", idpow);
		*/
		//
		// TODO: negative to power fraction should given error msg
		// TODO: division by zero should give error
#endif
	}
}
