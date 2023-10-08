#include "f2x.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#define max(a, b) ((a) < (b) ? (b) : (a))
#define min(a, b) ((a) < (b) ? (a) : (b))

bool f2x_alloc(struct f2x *f2x, int dim) {
  assert(dim >= 0);
  int *co_copied = (int *)calloc((dim + 1), sizeof(int));
  if (!co_copied) return false;

  f2x->coeffs = co_copied;
  f2x->dim = dim;
  return true;
}

void f2x_free(struct f2x *f2x) {
  free(f2x->coeffs);
  f2x->coeffs = NULL;
}

int f2x_get_coeffs(struct f2x *f2x, int d) {
  return (d <= f2x->dim) ? f2x->coeffs[d] : 0;
}

void f2x_shrink_dim(struct f2x *f2x) {
  for (int d = f2x->dim; d >= 0; d--) {
    if (f2x->coeffs[d] != 0) {
      f2x->dim = d;
      return;
    }
  }

  // ここまでくるなら多項式として0ということ
  f2x->dim = 0;
}

static void validate(struct f2x const *f2x) {
  assert(f2x->coeffs);
  for (int i = 0; i < f2x->dim; i++) {
    assert(f2x->coeffs[i] <= 1);
  }
}

bool f2x_alloc_add(struct f2x *res, struct f2x *a, struct f2x *b) {
  validate(a);
  validate(b);
  f2x_shrink_dim(a);
  f2x_shrink_dim(b);

  int res_dim = max(a->dim, b->dim);

  if (!f2x_alloc(res, res_dim)) return false;

  for (int d = 0; d <= res_dim; d++) {
    res->coeffs[d] = f2x_get_coeffs(a, d) ^ f2x_get_coeffs(b, d);
  }
  f2x_shrink_dim(res);
  validate(res);

  return true;
}

bool f2x_alloc_mul(struct f2x *res, struct f2x *a, struct f2x *b) {
  validate(a);
  validate(b);
  f2x_shrink_dim(a);
  f2x_shrink_dim(b);

  int res_dim = a->dim + b->dim;
  if (!f2x_alloc(res, res_dim)) return false;

  for (int d = 0; d <= res_dim; d++) {
    for (int ad = 0; ad <= d; ad++) {
      int bd = d - ad;
      res->coeffs[d] ^= f2x_get_coeffs(a, ad) * f2x_get_coeffs(b, bd);

      int c = 1;
      if (ad <= a->dim) c *= a->coeffs[ad];
      if (bd <= b->dim) c *= b->coeffs[bd];
    }
  }
  f2x_shrink_dim(res);
  validate(res);

  return true;
}

bool f2x_alloc_div(struct f2x *quot, struct f2x *rem, struct f2x *a,
                   struct f2x *b) {
  validate(a);
  validate(b);
  f2x_shrink_dim(a);
  f2x_shrink_dim(b);

  // ゼロ除算はさせない
  assert(b->coeffs[b->dim] == 1);

  int quot_dim = max(0, a->dim - b->dim);

  if (!f2x_alloc(quot, quot_dim)) return false;

  // 本来はあまりはbの次元未満だが、計算の途中の一時変数としても使いたいため、
  // 最初はaと同じ次元からスタートする。
  if (!f2x_alloc(rem, a->dim)) {
    f2x_free(quot);
    return false;
  }

  for (int d = 0; d <= a->dim; d++) rem->coeffs[d] = a->coeffs[d];

  for (int d = a->dim; d >= b->dim; d--) {
    int qd = d - b->dim;
    if (rem->coeffs[d] == 1) {
      quot->coeffs[qd] = 1;
      // 商を立てたのであまりから引く数を計算する
      struct f2x curq;
      if (!f2x_alloc(&curq, qd)) {
        f2x_free(quot);
        f2x_free(rem);
        return false;
      }
      curq.coeffs[qd] = 1;

      struct f2x rs;
      if (!f2x_alloc_mul(&rs, &curq, b)) {
        f2x_free(quot);
        f2x_free(rem);
        f2x_free(&curq);
        return false;
      }

      // remからrsを引く。引くがF2[x]では足すのと同じ。
      struct f2x next_rem;
      if (!f2x_alloc_add(&next_rem, rem, &rs)) {
        f2x_free(quot);
        f2x_free(rem);
        f2x_free(&curq);
        f2x_free(&rs);
        return false;
      }
      f2x_free(rem);
      f2x_free(&curq);
      f2x_free(&rs);
      *rem = next_rem;
    }
  }

  f2x_shrink_dim(quot);
  f2x_shrink_dim(rem);
  validate(quot);
  validate(rem);

  return true;
}

void f2x_dump(struct f2x *f2x) {
  validate(f2x);
  bool is_first = true;
  for (int d = f2x->dim; d >= 0; d--) {
    if (f2x->coeffs[d] > 0) {
      if (!is_first) printf(" + ");
      is_first = false;
      printf("x^%d", d);
    }
  }
  if (is_first) printf("0");
  printf("\n");
}
