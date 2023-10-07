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

/**
 * `c` を中心として位置パターンをレンダリングする。
 * @param mat レンダリング先
 * @param c 中心座標
 * @param is_large 大きいかどうか (左上、右上、左下の大きめのパターンかどうか)
 */
static void render_single_pospats(struct qr_matrix *mat, struct coord crd,
                                  bool is_large) {
  // is_large な位置パターンは、中央の塗りつぶしがちょっと広い。どれだけ余分に
  // 塗るか
  int center_extra = is_large ? 1 : 0;

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
 * 位置パターンをレンダリングする。
 */
static void render_pospats(struct qr_matrix *mat) {
  // 左上、右上、左下の位置パターン
  int mat_size = matrix_size(mat->version);
  render_single_pospats(mat, (struct coord){3, 3}, true);
  render_single_pospats(mat, (struct coord){mat_size - 4, 3}, true);
  render_single_pospats(mat, (struct coord){3, mat_size - 4}, true);

  // それ以外の位置パターン
  struct coord *crds;
  int num_crds = pospat_coords(mat->version, mat->mode, &crds);
  for (int i = 0; i < num_crds; i++) {
    render_single_pospats(mat, crds[i], false);
  }
}

/**
 * タイミングパターンをレンダリングする。
 */
static void render_timpats(struct qr_matrix *mat) {
  int mat_size = matrix_size(mat->version);

  // 縦のタイミングパターン
  for (int r = 0; r < mat_size; r++) {
    uint8_t *cell = &mat->data[r * mat_size + 6];
    if (*cell == QRMV_UNINIT) {
      *cell = (r % 2 == 0) ? QRMV_PRE_B : QRMV_PRE_W;
    }
  }

  // 横のタイミングパターン
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
  render_pospats(mat);
  render_timpats(mat);
  return;
}
