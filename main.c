#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "encoder.h"
#include "painter.h"
#include "qrcode.h"
#include "renderer.h"
#include "traits.h"

int main(int argc, char *argv[]) {
  struct qr_matrix mat = {0};

  // レンダリング
  struct qr_config cfg = {
      .version = qrver_3,
      .errmode = qrerr_M,
      .encmode = qrenc_8bit,
  };

  assert(qr_matrix_alloc(&mat, &cfg, qrmsk_000));

  uint8_t *data = calloc(num_blocks_data(&cfg) * 8, sizeof(uint8_t));
  uint8_t *errcodes = calloc(num_blocks_err(&cfg) * 8, sizeof(uint8_t));

  assert(data);
  assert(errcodes);

  const char *str = argc > 1 ? argv[1] : "hello";
  const char *output_file_name = argc > 2 ? argv[2] : "output.bmp";

  encode(&cfg, str, data, errcodes);
  render(&mat, data, errcodes);
  paint(output_file_name, &mat);

  qr_matrix_free(&mat);

  free(data);
  free(errcodes);

  printf("`%s` written in `%s`.\n", str, output_file_name);

  return 0;
}
