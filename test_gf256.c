#include <assert.h>
#include <stdio.h>

#include "field.h"
#include "gf256.h"
#include "kx.h"

int main(void) {
  assert(gf256_add(15, 15) == 0);
  assert(gf256_add(6, 5) == 3);
  assert(gf256_sub(6, 5) == 3);
  assert(gf256_mul(6, 3) == 10);
  assert(gf256_mul(7, 11) == 49);
  assert(gf256_div(9, 3) == 7);
  assert(gf256_div(12, 5) == 247);

  struct kx g;
  assert(kx_alloc(ft_gf256, &g, 17));
  g.coeffs[17] = gf256_from_exp(0);
  g.coeffs[16] = gf256_from_exp(43);
  g.coeffs[15] = gf256_from_exp(139);
  g.coeffs[14] = gf256_from_exp(206);
  g.coeffs[13] = gf256_from_exp(78);
  g.coeffs[12] = gf256_from_exp(43);
  g.coeffs[11] = gf256_from_exp(239);
  g.coeffs[10] = gf256_from_exp(123);
  g.coeffs[9] = gf256_from_exp(206);
  g.coeffs[8] = gf256_from_exp(214);
  g.coeffs[7] = gf256_from_exp(147);
  g.coeffs[6] = gf256_from_exp(24);
  g.coeffs[5] = gf256_from_exp(99);
  g.coeffs[4] = gf256_from_exp(150);
  g.coeffs[3] = gf256_from_exp(39);
  g.coeffs[2] = gf256_from_exp(243);
  g.coeffs[1] = gf256_from_exp(163);
  g.coeffs[0] = gf256_from_exp(136);

  struct kx f;
  assert(kx_alloc(ft_gf256, &f, 25));
  f.coeffs[25] = 32;
  f.coeffs[24] = 65;
  f.coeffs[23] = 205;
  f.coeffs[22] = 69;
  f.coeffs[21] = 41;
  f.coeffs[20] = 220;
  f.coeffs[19] = 46;
  f.coeffs[18] = 128;
  f.coeffs[17] = 236;

  struct kx q, r;
  assert(kx_div_alloc(&q, &r, &f, &g));

  kx_dump(&r);

  kx_free(&r);
  kx_free(&q);
  kx_free(&f);
  kx_free(&g);
}
