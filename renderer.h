#ifndef QRCODE_RENDERER_H_
#define QRCODE_RENDERER_H_

#include <stdbool.h>
#include <stdint.h>

#include "qrcode.h"

// qr_matrixのdata
#define QRMV_UNINIT 0
#define QRMV_W 1
#define QRMV_B 2
#define QRMV_PRE_W 3
#define QRMV_PRE_B 4

struct qr_matrix {
  uint8_t *data;
  enum qr_version version;
  enum qr_errmode mode;
};

struct coord {
  int r, c;
};

int matrix_size(enum qr_version version);

bool qr_matrix_alloc(struct qr_matrix *mat, enum qr_version version,
                     enum qr_errmode mode);

void qr_matrix_free(struct qr_matrix *mat);

void qr_matrix_dump(struct qr_matrix *mat);

int num_blocks_data(enum qr_version version, enum qr_errmode mode);

int num_blocks_err(enum qr_version version, enum qr_errmode mode);

int num_blocks_rs(enum qr_version version, enum qr_errmode mode);

void render(struct qr_matrix *mat, uint8_t const *data, uint8_t const *err);

#endif
