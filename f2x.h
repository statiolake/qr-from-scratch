#ifndef QRCODE_F2X_H_
#define QRCODE_F2X_H_

#include <stdbool.h>

struct f2x {
  int dim;
  int *coeffs;
};

bool f2x_alloc(struct f2x *f2x, int dim);

void f2x_free(struct f2x *f2x);

int f2x_get_coeffs(struct f2x *f2x, int d);

void f2x_shrink_dim(struct f2x *f2x);

bool f2x_alloc_add(struct f2x *res, struct f2x *a, struct f2x *b);

bool f2x_alloc_mul(struct f2x *res, struct f2x *a, struct f2x *b);

bool f2x_alloc_div(struct f2x *quot, struct f2x *rem, struct f2x *a,
                   struct f2x *b);

void f2x_dump(struct f2x *f2x);

#endif
