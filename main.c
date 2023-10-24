#include <stdio.h>
#include <stdlib.h>

#include "encoder.h"
#include "painter.h"
#include "qrcode.h"
#include "renderer.h"

int main(int argc, char *argv[]) {
  struct qr_matrix mat = {0};
  if (!qr_matrix_alloc(&mat, qrver_3, qrerr_M, qrmsk_000)) exit(EXIT_FAILURE);

  // レンダリング
  struct qr_config config = {
      .version = qrver_3,
      .errmode = qrerr_M,
      .encmode = qrenc_8bit,
  };
  uint8_t data[44 * 8] = {0};
  uint8_t errcodes[22 * 8] = {0};

  encode(&config, "abcdefghijklmnopqrstuvwxyz", data, errcodes);

  const char *output_file_name = argc > 1 ? argv[1] : "output.bmp";
  render(&mat, data, errcodes);
  paint(output_file_name, &mat);

  qr_matrix_free(&mat);

  return 0;
}
