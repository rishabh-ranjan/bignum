#include <stdio.h>
#include <string.h>
#include "bignum.h"

// buffersize for bignum strings
#define BF 10010

void badd() {
	char sa[BF], sb[BF];
	scanf("%s %s", sa, sb);
	struct bignum *a = string_to_bignum(sa);
	struct bignum *b = string_to_bignum(sb);
	struct bignum *r = addsub_signed(a, b, 0);
	printf("%s\n", bignum_to_string(r));
	bignum_free(a);
	bignum_free(b);
	bignum_free(r);
}

void bsub() {
	char sa[BF], sb[BF];
	scanf("%s %s", sa, sb);
	struct bignum *a = string_to_bignum(sa);
	struct bignum *b = string_to_bignum(sb);
	struct bignum *r = addsub_signed(a, b, 1);
	printf("%s\n", bignum_to_string(r));
	bignum_free(a);
	bignum_free(b);
	bignum_free(r);
}

void bmul() {
	char sa[BF], sb[BF];
	scanf("%s %s", sa, sb);
	struct bignum *a = string_to_bignum(sa);
	struct bignum *b = string_to_bignum(sb);
	struct bignum *r = long_mul(a, b);
	printf("%s\n", bignum_to_string(r));
	bignum_free(a);
	bignum_free(b);
	bignum_free(r);
}

void bdiv() {
	char sa[BF], sb[BF];
	scanf("%s %s", sa, sb);
	struct bignum *a = string_to_bignum(sa);
	struct bignum *b = string_to_bignum(sb);
	struct bignum *r = long_div(a, b);
	if (r == NULL) {
		printf("Division by zero error!\n");
	} else {
		printf("%s\n", bignum_to_string(r));
		bignum_free(r);
	}
	bignum_free(a);
	bignum_free(b);
}

void bsqrt() {
	char sa[BF];
	scanf("%s", sa);
	struct bignum *a = string_to_bignum(sa);
	struct bignum *r = sqrt_signed(a);
	if (r == NULL) {
		printf("Sqrt of negative number not supported!\n");
	} else {
		printf("%s\n", bignum_to_string(r));
		bignum_free(r);
	}
	bignum_free(a);
}

void babs() {
	char sa[BF];
	scanf("%s", sa);
	struct bignum *a = string_to_bignum(sa);
	struct bignum *r = bignum_abs(a);
	printf("%s\n", bignum_to_string(r));
	bignum_free(r);
	bignum_free(a);
}

void bpow() {
	char sa[BF], sb[BF];
	scanf("%s %s", sa, sb);
	struct bignum *a = string_to_bignum(sa);
	struct bignum *b = string_to_bignum(sb);
	struct bignum *r = long_pow(a, b);
	if (r == NULL) {
		printf("Fractional power of negative base not supported!\n");
	} else {
		printf("%s\n", bignum_to_string(r));
		bignum_free(r);
	}
	bignum_free(a);
	bignum_free(b);
}

int main() {
	char op[10];
	while (scanf("%s", op) != EOF) {
		if (strcmp(op, "ADD") == 0) badd();
		else if (strcmp(op, "SUB") == 0) bsub();
		else if (strcmp(op, "MUL") == 0) bmul();
		else if (strcmp(op, "DIV") == 0) bdiv();
		else if (strcmp(op, "SQRT") == 0) bsqrt();
		else if (strcmp(op, "ABS") == 0) babs();
		else if (strcmp(op, "POW") == 0) bpow();
	}
}
