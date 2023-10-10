#include "field.h"

#include <assert.h>

#include "gf2.h"

void field_validate(enum field_type ft, int n) {
  assert(ft == ft_gf2);
  gf2_validate(n);
}

int field_add(enum field_type ft, int a, int b) {
  assert(ft == ft_gf2);
  return gf2_add(a, b);
}

int field_sub(enum field_type ft, int a, int b) {
  assert(ft == ft_gf2);
  return gf2_sub(a, b);
}

int field_mul(enum field_type ft, int a, int b) {
  assert(ft == ft_gf2);
  return gf2_mul(a, b);
}

int field_div(enum field_type ft, int a, int b) {
  assert(ft == ft_gf2);
  return gf2_div(a, b);
}
