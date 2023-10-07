#include <stdio.h>
#include <stdlib.h>

#include "qrcode.h"
#include "renderer.h"

int main(void) {
  struct qr_matrix mat = {0};
  if (!qr_matrix_alloc(&mat, qrver_3, qrerr_M, qrmsk_000)) exit(EXIT_FAILURE);

  // レンダリング
  uint8_t data[44 * 8] = {0};
  uint8_t err[22 * 8] = {0};
  render(&mat, data, err);

  qr_matrix_dump(&mat);
  qr_matrix_free(&mat);

  return 0;
}
