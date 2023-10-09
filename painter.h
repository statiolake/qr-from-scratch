#ifndef QRCODE_PAINTER_H
#define QRCODE_PAINTER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "renderer.h"

bool paint(struct qr_matrix *mat);

#endif
