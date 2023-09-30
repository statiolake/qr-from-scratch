#include "renderer.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "qrcode.h"

int matrix_size(enum qr_version version) {
  // qrver_3 しか対応しない
  assert(version == qrver_3);
  return 29;
}

int num_blocks_data(enum qr_version version, enum qr_errmode mode) {
  // qrver_3, qrerr_M しか対応しない
  assert(version == qrver_3);
  assert(mode == qrerr_M);
  return 44;
}

int num_blocks_err(enum qr_version version, enum qr_errmode mode) {
  // qrver_3, qrerr_M しか対応しない
  assert(version == qrver_3);
  assert(mode == qrerr_M);
  return 26;
}

int num_blocks_rs(enum qr_version version, enum qr_errmode mode) {
  // qrver_3, qrerr_M しか対応しない
  assert(version == qrver_3);
  assert(mode == qrerr_M);
  return 1;
}

int pospat_coords(enum qr_version version, enum qr_errmode mode,
                  struct coord **crds) {
  assert(version == qrver_3);
  assert(mode == qrerr_M);

  // QRコード3Mの位置は0-indexedで(22, 22)の1つ
  static struct coord COORDS_3M[] = {{.r = 22, .c = 22}};
  *crds = COORDS_3M;
  return 1;
}

bool qr_matrix_alloc(struct qr_matrix *mat, enum qr_version version,
                     enum qr_errmode mode) {
  int size = matrix_size(version);
  uint8_t *data = (uint8_t *)malloc(size * size);
  if (!data) return false;

  mat->data = data;
  mat->mode = mode;
  mat->version = version;

  return true;
}

void qr_matrix_free(struct qr_matrix *mat) {
  if (!mat->data) return;
  free(mat->data);
  mat->data = NULL;
}

void render(struct qr_matrix *mat, const uint8_t *data,
            const uint8_t *error_codes) {}
