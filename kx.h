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

bool kx_alloc_add(struct kx *res, struct kx *a, struct kx *b);

bool kx_alloc_sub(struct kx *res, struct kx *a, struct kx *b);

bool kx_alloc_mul(struct kx *res, struct kx *a, struct kx *b);

bool kx_alloc_div(struct kx *quot, struct kx *rem, struct kx *a, struct kx *b);

void kx_dump(struct kx *kx);

#endif
