#ifndef QRCODE_PAINTER_H
#define QRCODE_PAINTER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "renderer.h"

size_t compute_size(struct qr_matrix *mat);

void paint(uint8_t *buf, struct qr_matrix *mat);

#endif
