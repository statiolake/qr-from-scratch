#ifndef QRCODE_GF256_H_
#define QRCODE_GF256_H_

void gf256_validate(int n);

int gf256_from_exp(int exp);

int gf256_add(int a, int b);

int gf256_sub(int a, int b);

int gf256_mul(int a, int b);

int gf256_div(int a, int b);

#endif
