#ifndef QRCODE_TRAITS_H_
#define QRCODE_TRAITS_H_

#include "qrcode.h"

int num_blocks_data(struct qr_config const *cfg);

int num_blocks_err(struct qr_config const *cfg);

int num_blocks_rs(struct qr_config const *cfg);

#endif
