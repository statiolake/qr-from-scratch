#ifndef QRCODE_TRAITS_H_
#define QRCODE_TRAITS_H_

#include "qrcode.h"

int num_blocks_data(enum qr_version version, enum qr_errmode mode);

int num_blocks_err(enum qr_version version, enum qr_errmode mode);

int num_blocks_rs(enum qr_version version, enum qr_errmode mode);

#endif
