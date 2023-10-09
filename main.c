#include <stdio.h>
#include <stdlib.h>

#include "painter.h"
#include "qrcode.h"
#include "renderer.h"

int main(int argc, char *argv[]) {
  struct qr_matrix mat = {0};
  if (!qr_matrix_alloc(&mat, qrver_3, qrerr_M, qrmsk_000)) exit(EXIT_FAILURE);

  // レンダリング
  uint8_t data[44 * 8] = {0};
  uint8_t err[22 * 8] = {0};
  for (int i = 0; i < 44 * 8; i++) {
    data[i] = i % 2;
    if (i < 22 * 8) err[i] = i % 2;
  }

  const char *output_file_name = argc > 1 ? argv[1] : "output.bmp";
  render(&mat, data, err);
  paint(output_file_name, &mat);

  qr_matrix_free(&mat);

  return 0;
}
