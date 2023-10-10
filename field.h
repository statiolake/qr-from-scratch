#ifndef QRCODE_FIELD_H_
#define QRCODE_FIELD_H_

enum field_type {
  ft_gf2,
};

void field_validate(enum field_type ft, int n);

int field_add(enum field_type ft, int a, int b);

int field_sub(enum field_type ft, int a, int b);

int field_mul(enum field_type ft, int a, int b);

int field_div(enum field_type ft, int a, int b);

#endif
