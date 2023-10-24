#include "gf2.h"

#include <assert.h>

void gf2_validate(int n) { assert(n <= 1); }

int gf2_add(int a, int b) { return a ^ b; }

int gf2_sub(int a, int b) { return a ^ b; }

int gf2_mul(int a, int b) { return a & b; }

int gf2_div(int a, int b) {
  assert(b > 0);
  return a;
}
