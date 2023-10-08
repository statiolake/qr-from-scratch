#include "painter.h"

#include <assert.h>
#include <stdalign.h>
#include <stdio.h>
#include <string.h>

#include "qrcode.h"
#include "renderer.h"

struct BITMAPFILEHEADER {
  char bfType[2];
  uint32_t bfSize;
  uint16_t bfReserved1;
  uint16_t bfReserved2;
  uint32_t bfOffBits;
};

struct BITMAPINFOHEADER {
  uint32_t bcSize;
  uint32_t bcWidth;
  uint32_t bcHeight;
  uint16_t bcPlanes;
  uint16_t bcBitCoun;
  uint32_t biCompression;
  uint32_t biSizeImage;
  uint32_t biXPixPerMete;
  uint32_t biYPixPerMeter;
  uint32_t biClrUsed;
  uint32_t biCirImportan;
};

struct bitmap_palette {
  uint8_t rgbBlue;
  uint8_t rgbRed;
  uint8_t rgbGreen;
  uint8_t rgbReserved;
};

static bool is_little_endian(void) {
  uint16_t orig = 0x1234;
  uint8_t encoded[2] = {0};
  memcpy(encoded, &orig, sizeof(orig));
  printf("%x, %x\n", encoded[0], encoded[1]);
  return encoded[0] == 0x34;
}

size_t compute_size(struct qr_matrix *mat) {
  // アラインメント周りが不安なので sizeof は使わない
  // (バイト数と並び的に特に隙間は入らないはずだが...)
  int file_header_size = 2 + 4 + 2 + 2 + 4;
  int info_header_size = 4 + 4 + 4 + 2 + 2 + 4 + 4 + 4 + 4 + 4 + 4;
  // 各行を4の倍数にしないといけないらしい
  int mat_size = matrix_size(mat->version);
  int line_width = (mat_size + 3) / 4 * 4;
  // 今回は1画素1バイトにする (本当は1ビットでもいいんだけどちょっといい感じに
  // ビットを詰めるのは面倒なのでuint8_tのまま1バイトで扱っていく)
  int data_size = mat_size * line_width;

  return file_header_size + info_header_size + data_size;
}

void paint(uint8_t *buf, struct qr_matrix *mat) {
  assert(is_little_endian());
  // TODO
}
