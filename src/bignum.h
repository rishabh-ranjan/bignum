#ifndef BIGNUM_H
#define BIGNUM_H

struct bignum;

void bignum_free(struct bignum*);
struct bignum *clone(const struct bignum*);
struct bignum *bignum_abs(const struct bignum*);
struct bignum *string_to_bignum(const char*);
char *bignum_to_string(const struct bignum*);
int mag_comp(const struct bignum*, const struct bignum*);
struct bignum *addsub_signed(const struct bignum*, const struct bignum*, int);
struct bignum *long_mul(const struct bignum*, const struct bignum*);
struct bignum *long_div(const struct bignum*, const struct bignum*);
struct bignum *sqrt_unsigned(const struct bignum*);
struct bignum *long_pow(const struct bignum*, const struct bignum*);

#endif
