#ifndef QRCODE_TRAITS_H_
#define QRCODE_TRAITS_H_

#include <stddef.h>

#include "qrcode.h"

size_t num_blocks_data(struct qr_config const *cfg);

size_t num_blocks_err(struct qr_config const *cfg);

size_t num_blocks_rs(struct qr_config const *cfg);

size_t num_max_length(struct qr_config const *cfg);

#endif
