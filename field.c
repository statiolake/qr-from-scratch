#include "field.h"

#include <assert.h>
#include <stdbool.h>

#include "gf2.h"
#include "gf256.h"

void field_validate(enum field_type ft, int n) {
  switch (ft) {
    case ft_gf2:
      gf2_validate(n);
      break;
    case ft_gf256:
      gf256_validate(n);
      break;
    default:
      assert(false);
  }
}

int field_add(enum field_type ft, int a, int b) {
  switch (ft) {
    case ft_gf2:
      return gf2_add(a, b);
      break;
    case ft_gf256:
      return gf256_add(a, b);
      break;
    default:
      assert(false);
  }
}

int field_sub(enum field_type ft, int a, int b) {
  switch (ft) {
    case ft_gf2:
      return gf2_sub(a, b);
      break;
    case ft_gf256:
      return gf256_sub(a, b);
      break;
    default:
      assert(false);
  }
}

int field_mul(enum field_type ft, int a, int b) {
  switch (ft) {
    case ft_gf2:
      return gf2_mul(a, b);
      break;
    case ft_gf256:
      return gf256_mul(a, b);
      break;
    default:
      assert(false);
  }
}

int field_div(enum field_type ft, int a, int b) {
  switch (ft) {
    case ft_gf2:
      return gf2_div(a, b);
      break;
    case ft_gf256:
      return gf256_div(a, b);
      break;
    default:
      assert(false);
  }
}
