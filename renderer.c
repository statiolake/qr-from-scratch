#include "renderer.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "qrcode.h"

int matrix_size(enum qr_version version) {
  // qrver_3 しか対応しない
  assert(version == qrver_3);
  return 29;
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

void qr_matrix_dump(struct qr_matrix *mat) {
  int mat_size = matrix_size(mat->version);
  for (int r = 0; r < mat_size; r++) {
    for (int c = 0; c < mat_size; c++) {
      int cell = mat->data[r * mat_size + c];
      switch (cell) {
        case QRMV_UNINIT:
          putchar('_');
          break;
        case QRMV_W:
        case QRMV_PRE_W:
          putchar('.');
          break;
        case QRMV_B:
        case QRMV_PRE_B:
          putchar('*');
          break;
      }
    }
    putchar('\n');
  }
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

static int alignment_coords(enum qr_version version, enum qr_errmode mode,
                            struct coord const **crds) {
  assert(version == qrver_3);
  assert(mode == qrerr_M);

  // QRコード3Mの位置は0-indexedで(22, 22)の1つ
  static struct coord COORDS_3M[] = {{.r = 22, .c = 22}};
  *crds = COORDS_3M;
  return 1;
}

/**
 * `c` を中心として四角いパターン (Finder, Alignment) をレンダリングする。
 * @param mat レンダリング先
 * @param c 中心座標
 * @param is_finder Finder かどうか (左上、右上、左下の大きいやつかどうか)
 */
static void render_square_pattern(struct qr_matrix *mat, struct coord crd,
                                  bool is_finder) {
  // Finder パターンは中央の塗りつぶしがちょっと広い。どれだけ余分に塗るか
  int center_extra = is_finder ? 1 : 0;

  // c がはみ出していないことを確認する
  int pat_size = 2 + center_extra;
  int mat_size = matrix_size(mat->version);
  assert(0 <= crd.c - pat_size && crd.c + pat_size < mat_size);
  assert(0 <= crd.r - pat_size && crd.r + pat_size < mat_size);

  // 塗る
  for (int dr = -pat_size; dr <= pat_size; dr++) {
    for (int dc = -pat_size; dc <= pat_size; dc++) {
      int r = crd.r + dr;
      int c = crd.c + dc;

      bool on_border = dr == -pat_size || dr == pat_size || dc == -pat_size ||
                       dc == pat_size;
      bool on_center = abs(dr) <= center_extra && abs(dc) <= center_extra;
      mat->data[r * mat_size + c] =
          on_border || on_center ? QRMV_PRE_B : QRMV_PRE_W;
    }
  }
}

/**
 * Finder パターンをレンダリングする。
 */
static void render_finders(struct qr_matrix *mat) {
  // 左上、右上、左下
  int mat_size = matrix_size(mat->version);
  render_square_pattern(mat, (struct coord){3, 3}, true);
  render_square_pattern(mat, (struct coord){mat_size - 4, 3}, true);
  render_square_pattern(mat, (struct coord){3, mat_size - 4}, true);
}

/**
 * Alignment パターンをレンダリングする。
 */
static void render_alignments(struct qr_matrix *mat) {
  struct coord const *crds;
  int num_crds = alignment_coords(mat->version, mat->mode, &crds);
  for (int i = 0; i < num_crds; i++) {
    render_square_pattern(mat, crds[i], false);
  }
}

/**
 * Timing パターンをレンダリングする。
 */
static void render_timings(struct qr_matrix *mat) {
  int mat_size = matrix_size(mat->version);

  // 縦の Timing パターン
  for (int r = 0; r < mat_size; r++) {
    uint8_t *cell = &mat->data[r * mat_size + 6];
    if (*cell == QRMV_UNINIT) {
      *cell = (r % 2 == 0) ? QRMV_PRE_B : QRMV_PRE_W;
    }
  }

  // 横の Timing パターン
  for (int c = 0; c < mat_size; c++) {
    uint8_t *cell = &mat->data[6 * mat_size + c];
    if (*cell == QRMV_UNINIT) {
      *cell = (c % 2 == 0) ? QRMV_PRE_B : QRMV_PRE_W;
    }
  }
}

void render(struct qr_matrix *mat, const uint8_t *data,
            const uint8_t *error_codes) {
  assert(data);
  assert(error_codes);

  render_finders(mat);
  render_alignments(mat);
  render_timings(mat);

  return;
}
