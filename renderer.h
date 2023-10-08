#ifndef QRCODE_RENDERER_H_
#define QRCODE_RENDERER_H_

#include <stdbool.h>
#include <stdint.h>

#include "qrcode.h"

// qr_matrixのdata
#define QRMV_UNINIT 0
#define QRMV_W 2
#define QRMV_B 3
#define QRMV_PRE_W 4
#define QRMV_PRE_B 5

#define QR_FORMAT_INFO_MASK 21522;  // 101010000010010

enum qr_maskpat {
  qrmsk_000 = 0,  // (i+j) mod 2 = 0
  // qrmsk_001 = 1,  // i mod 2 = 0
  // qrmsk_010 = 2,  // j mod 3 = 0
  qrmsk_011 = 3,  // (i+j) mod 3 = 0
  // qrmsk_100 = 4,  // (( i div 2)+(j div 3)) mod 2 = 0
  // qrmsk_101 = 5,  // (ij) mod 2 + (ij) mod 3 = 0
  // qrmsk_110 = 6,  // ((ij) mod 2 +(ij) mod 3) mod 2 = 0
  // qrmsk_111 = 7,  // ((ij)mod 3 + (i+j) mod 2) mod 2 = 0
};

struct qr_matrix {
  uint8_t *data;
  enum qr_version version;
  enum qr_errmode mode;
  enum qr_maskpat mask;
};

struct coord {
  int r, c;
};

int matrix_size(enum qr_version version);

bool qr_matrix_alloc(struct qr_matrix *mat, enum qr_version version,
                     enum qr_errmode mode, enum qr_maskpat mask);

void qr_matrix_free(struct qr_matrix *mat);

void qr_matrix_dump(struct qr_matrix *mat);

void qr_matrix_dump_tsv(struct qr_matrix *mat);

int num_blocks_data(enum qr_version version, enum qr_errmode mode);

int num_blocks_err(enum qr_version version, enum qr_errmode mode);

int num_blocks_rs(enum qr_version version, enum qr_errmode mode);

void render(struct qr_matrix *mat, uint8_t const *data, uint8_t const *err);

#endif
