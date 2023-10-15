#include "kx.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "field.h"

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#define max(a, b) ((a) < (b) ? (b) : (a))
#define min(a, b) ((a) < (b) ? (a) : (b))

bool kx_alloc(enum field_type k, struct kx *kx, int dim) {
  assert(dim >= 0);
  int *co_copied = (int *)calloc((dim + 1), sizeof(int));
  if (!co_copied) return false;

  kx->k = k;
  kx->coeffs = co_copied;
  kx->dim = dim;
  return true;
}

void kx_free(struct kx *kx) {
  free(kx->coeffs);
  kx->coeffs = NULL;
}

int kx_get_coeffs(struct kx *kx, int d) {
  return (d <= kx->dim) ? kx->coeffs[d] : 0;
}

void kx_shrink_dim(struct kx *kx) {
  for (int d = kx->dim; d >= 0; d--) {
    if (kx->coeffs[d] != 0) {
      kx->dim = d;
      return;
    }
  }

  // ここまでくるなら多項式として0ということ
  kx->dim = 0;
}

static void validate(struct kx const *kx) {
  assert(kx->coeffs);
  for (int i = 0; i < kx->dim; i++) {
    field_validate(kx->k, kx->coeffs[i] <= 1);
  }
}

bool kx_add_alloc(struct kx *res, struct kx *a, struct kx *b) {
  assert(a->k == b->k);
  validate(a);
  validate(b);
  kx_shrink_dim(a);
  kx_shrink_dim(b);

  int res_dim = max(a->dim, b->dim);

  if (!kx_alloc(a->k, res, res_dim)) return false;

  for (int d = 0; d <= res_dim; d++) {
    res->coeffs[d] = field_add(a->k, kx_get_coeffs(a, d), kx_get_coeffs(b, d));
  }
  kx_shrink_dim(res);
  validate(res);

  return true;
}

bool kx_sub_alloc(struct kx *res, struct kx *a, struct kx *b) {
  assert(a->k == b->k);
  validate(a);
  validate(b);
  kx_shrink_dim(a);
  kx_shrink_dim(b);

  int res_dim = max(a->dim, b->dim);

  if (!kx_alloc(a->k, res, res_dim)) return false;

  for (int d = 0; d <= res_dim; d++) {
    res->coeffs[d] = field_sub(a->k, kx_get_coeffs(a, d), kx_get_coeffs(b, d));
  }
  kx_shrink_dim(res);
  validate(res);

  return true;
}

bool kx_mul_alloc(struct kx *res, struct kx *a, struct kx *b) {
  assert(a->k == b->k);
  validate(a);
  validate(b);
  kx_shrink_dim(a);
  kx_shrink_dim(b);

  int res_dim = a->dim + b->dim;
  if (!kx_alloc(a->k, res, res_dim)) return false;

  for (int d = 0; d <= res_dim; d++) {
    for (int ad = 0; ad <= d; ad++) {
      int bd = d - ad;
      res->coeffs[d] = field_add(
          a->k, res->coeffs[d],
          field_mul(a->k, kx_get_coeffs(a, ad), kx_get_coeffs(b, bd)));
    }
  }
  kx_shrink_dim(res);
  validate(res);

  return true;
}

bool kx_div_alloc(struct kx *quot, struct kx *rem, struct kx *a, struct kx *b) {
  assert(a->k == b->k);
  validate(a);
  validate(b);
  kx_shrink_dim(a);
  kx_shrink_dim(b);

  // ゼロ除算はさせない
  assert(b->coeffs[b->dim] > 0);

  int quot_dim = max(0, a->dim - b->dim);

  if (!kx_alloc(a->k, quot, quot_dim)) return false;

  // 本来はあまりはbの次元未満だが、計算の途中の一時変数としても使いたいため、
  // 最初はaと同じ次元からスタートする。
  if (!kx_alloc(a->k, rem, a->dim)) {
    kx_free(quot);
    return false;
  }

  for (int d = 0; d <= a->dim; d++) rem->coeffs[d] = a->coeffs[d];

  for (int d = a->dim; d >= b->dim; d--) {
    int qd = d - b->dim;
    if (rem->coeffs[d] > 0) {
      quot->coeffs[qd] = field_div(a->k, rem->coeffs[d], b->coeffs[b->dim]);

      // まず今立てた商の多項式を作る
      struct kx curr_quot;
      if (!kx_alloc(a->k, &curr_quot, qd)) {
        kx_free(quot);
        kx_free(rem);
        return false;
      }
      curr_quot.coeffs[qd] = quot->coeffs[qd];

      // 立てた商とbをかけ、あまりから引く数を作る
      struct kx to_sub;
      if (!kx_mul_alloc(&to_sub, &curr_quot, b)) {
        kx_free(quot);
        kx_free(rem);
        kx_free(&curr_quot);
        return false;
      }

      // remからrsを引く。
      struct kx next_rem;
      if (!kx_sub_alloc(&next_rem, rem, &to_sub)) {
        kx_free(quot);
        kx_free(rem);
        kx_free(&curr_quot);
        kx_free(&to_sub);
        return false;
      }
      kx_free(rem);
      kx_free(&curr_quot);
      kx_free(&to_sub);
      *rem = next_rem;
    }
  }

  kx_shrink_dim(quot);
  kx_shrink_dim(rem);
  validate(quot);
  validate(rem);

  return true;
}

void kx_dump(struct kx *kx) {
  validate(kx);
  bool is_first = true;
  for (int d = kx->dim; d >= 0; d--) {
    if (kx->coeffs[d] > 0) {
      if (!is_first) printf(" + ");
      is_first = false;
      if (kx->coeffs[d] > 1) {
        printf("%d", kx->coeffs[d]);
      }
      printf("x^%d", d);
    }
  }
  if (is_first) printf("0");
  printf("\n");
}
