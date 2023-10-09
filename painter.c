#include "painter.h"

#include <assert.h>
#include <stdalign.h>
#include <stdio.h>
#include <stdlib.h>
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
  uint16_t bcBitCount;
  uint32_t biCompression;
  uint32_t biSizeImage;
  uint32_t biXPixPerMeter;
  uint32_t biYPixPerMeter;
  uint32_t biClrUsed;
  uint32_t biCirImportant;
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
  return encoded[0] == 0x34;
}

static size_t compute_metadata_size(void) {
  // アラインメント周りが不安なので sizeof は使わない
  // (バイト数と並び的に特に隙間は入らないはずだが...)
  size_t file_header_size = 2 + 4 + 2 + 2 + 4;
  size_t info_header_size = 4 + 4 + 4 + 2 + 2 + 4 + 4 + 4 + 4 + 4 + 4;

  // カラーパレットのサイズ
  size_t color_palette_size = 4 * 2;

  return file_header_size + info_header_size + color_palette_size;
}

static size_t compute_size(struct qr_matrix *mat) {
  size_t metadata_size = compute_metadata_size();

  // データのサイズ
  // 各行を4の倍数にしないといけないらしい
  size_t mat_size = matrix_size(mat->version);
  size_t line_width = (mat_size + 3) / 4 * 4;
  // 今回は1画素1バイトにする (本当は1ビットでもいいんだけどちょっといい感じに
  // ビットを詰めるのは面倒なのでuint8_tのまま1バイトで扱っていく)
  size_t data_size = mat_size * line_width;

  return metadata_size + data_size;
}

static void write_file_header(FILE *fp, struct BITMAPFILEHEADER *header) {
  assert(fwrite(header->bfType, sizeof(header->bfType[0]), 2, fp) == 2);
  assert(fwrite(&header->bfSize, sizeof(header->bfSize), 1, fp) == 1);
  assert(fwrite(&header->bfReserved1, sizeof(header->bfReserved1), 1, fp) == 1);
  assert(fwrite(&header->bfReserved2, sizeof(header->bfReserved2), 1, fp) == 1);
  assert(fwrite(&header->bfOffBits, sizeof(header->bfOffBits), 1, fp) == 1);
}

static void write_info_header(FILE *fp, struct BITMAPINFOHEADER *header) {
  assert(fwrite(&header->bcSize, sizeof(header->bcSize), 1, fp) == 1);
  assert(fwrite(&header->bcWidth, sizeof(header->bcWidth), 1, fp) == 1);
  assert(fwrite(&header->bcHeight, sizeof(header->bcHeight), 1, fp) == 1);
  assert(fwrite(&header->bcPlanes, sizeof(header->bcPlanes), 1, fp) == 1);
  assert(fwrite(&header->bcBitCount, sizeof(header->bcBitCount), 1, fp) == 1);
  assert(fwrite(&header->biCompression, sizeof(header->biCompression), 1, fp) ==
         1);
  assert(fwrite(&header->biSizeImage, sizeof(header->biSizeImage), 1, fp) == 1);
  assert(fwrite(&header->biXPixPerMeter, sizeof(header->biXPixPerMeter), 1,
                fp) == 1);
  assert(fwrite(&header->biYPixPerMeter, sizeof(header->biYPixPerMeter), 1,
                fp) == 1);
  assert(fwrite(&header->biClrUsed, sizeof(header->biClrUsed), 1, fp) == 1);
  assert(fwrite(&header->biCirImportant, sizeof(header->biCirImportant), 1,
                fp) == 1);
}

static void write_bitmap_palette(FILE *fp, struct bitmap_palette *palette) {
  assert(sizeof(struct bitmap_palette) == 4);
  assert(fwrite(&palette->rgbBlue, sizeof(palette->rgbBlue), 1, fp) == 1);
  assert(fwrite(&palette->rgbRed, sizeof(palette->rgbRed), 1, fp) == 1);
  assert(fwrite(&palette->rgbGreen, sizeof(palette->rgbGreen), 1, fp) == 1);
  assert(fwrite(&palette->rgbReserved, sizeof(palette->rgbReserved), 1, fp) ==
         1);
}

static void write_bitmap(FILE *fp, int height, int width, uint8_t *bitmap) {
  int bitmap_width = (width + 3) / 4 * 4;

  uint8_t zero = 0;
  // ビットマップ形式の画像データは下から上へ、らしい
  for (int r = height - 1; r >= 0; r--) {
    int c;
    for (c = 0; c < width; c++) {
      fwrite(&bitmap[r * width + c], sizeof(uint8_t), 1, fp);
    }
    for (; c < bitmap_width; c++) {
      fwrite(&zero, sizeof(uint8_t), 1, fp);
    }
  }
}

static void convert_to_bitmap(uint8_t *restrict dest,
                              uint8_t const *restrict src, int size) {
  for (int r = 0; r < size; r++) {
    for (int c = 0; c < size; c++) {
      assert(src[r * size + c]);
      dest[r * size + c] = src[r * size + c] & 0x1;
    }
  }
}

bool paint(struct qr_matrix *mat) {
  assert(is_little_endian());

  struct BITMAPFILEHEADER fh = {
      .bfType = {'B', 'M'},
      .bfSize = compute_size(mat),
      .bfOffBits = compute_metadata_size(),
  };

  int mat_size = matrix_size(mat->version);
  struct BITMAPINFOHEADER ih = {
      .bcSize = 40,
      .bcWidth = mat_size,
      .bcHeight = mat_size,
      .bcPlanes = 1,
      .bcBitCount = 8,
      .biCompression = 0,  // 無圧縮
      .biClrUsed = 2,
  };

  struct bitmap_palette pals[] = {
      {0xff, 0xff, 0xff, 0},
      {0x00, 0x00, 0x00, 0},
  };

  FILE *fp = fopen("test.bmp", "wb");
  if (!fp) return false;

  write_file_header(fp, &fh);
  write_info_header(fp, &ih);
  write_bitmap_palette(fp, &pals[0]);
  write_bitmap_palette(fp, &pals[1]);
  uint8_t *bitmap = (uint8_t *)malloc(mat_size * mat_size);
  convert_to_bitmap(bitmap, mat->data, mat_size);
  write_bitmap(fp, mat_size, mat_size, bitmap);

  fclose(fp);

  return true;
}
