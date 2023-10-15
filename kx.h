#ifndef QRCODE_F2X_H_
#define QRCODE_F2X_H_

#include <stdbool.h>

#include "field.h"

struct kx {
  enum field_type k;
  int dim;
  int *coeffs;
};

bool kx_alloc(enum field_type k, struct kx *kx, int dim);

void kx_free(struct kx *kx);

int kx_get_coeffs(struct kx *kx, int d);

void kx_shrink_dim(struct kx *kx);

bool kx_add_alloc(struct kx *res, struct kx *a, struct kx *b);

bool kx_sub_alloc(struct kx *res, struct kx *a, struct kx *b);

bool kx_mul_alloc(struct kx *res, struct kx *a, struct kx *b);

bool kx_div_alloc(struct kx *quot, struct kx *rem, struct kx *a, struct kx *b);

void kx_dump(struct kx *kx);

#endif
