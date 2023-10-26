#ifndef QRCODE_ENCODER_H_
#define QRCODE_ENCODER_H_

#include <stdint.h>

#include "qrcode.h"

void encode(struct qr_config const *cfg, char const *str, uint8_t *data,
            uint8_t *errcodes);

#endif
