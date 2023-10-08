#include "renderer.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "f2x.h"
#include "qrcode.h"

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#define max(a, b) ((a) < (b) ? (b) : (a))
#define min(a, b) ((a) < (b) ? (a) : (b))

int matrix_size(enum qr_version version) {
  // qrver_3 しか対応しない
  assert(version == qrver_3);
  return 29;
}

bool qr_matrix_alloc(struct qr_matrix *mat, enum qr_version version,
                     enum qr_errmode mode, enum qr_maskpat mask) {
  int size = matrix_size(version);
  uint8_t *data = (uint8_t *)malloc(size * size);
  if (!data) return false;

  mat->data = data;
  mat->version = version;
  mat->mode = mode;
  mat->mask = mask;

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
        default:
          if (cell >= 10) putchar('0' + (cell - 10));
      }
    }
    putchar('\n');
  }
}

void qr_matrix_dump_tsv(struct qr_matrix *mat) {
  int mat_size = matrix_size(mat->version);
  for (int r = 0; r < mat_size; r++) {
    for (int c = 0; c < mat_size; c++) {
      if (c > 0) putchar('\t');
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
        default:
          if (cell >= 10) putchar('0' + (cell - 10));
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

  // Finder パターンは外周をもう一つ分白く塗る仕様
  if (is_finder) {
    for (int dr = -pat_size - 1; dr <= pat_size + 1; dr++) {
      for (int dc = -pat_size - 1; dc <= pat_size + 1; dc++) {
        bool on_border = dr == -pat_size - 1 || dr == pat_size + 1 ||
                         dc == -pat_size - 1 || dc == pat_size + 1;
        if (!on_border) continue;

        int r = crd.r + dr;
        int c = crd.c + dc;
        if (r < 0 || r >= mat_size || c < 0 || c >= mat_size) continue;

        mat->data[r * mat_size + c] = QRMV_PRE_W;
      }
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

static void compute_format_info(enum qr_errmode mode, enum qr_maskpat mask,
                                int *bits) {
#define BITS_OF(num, nth) ((num & (1 << (nth))) >> (nth))

  // 誤り訂正レベルを入れる
  bits[0] = BITS_OF(mode, 1);
  bits[1] = BITS_OF(mode, 0);

  // マスク指示子を入れる
  bits[2] = BITS_OF(mask, 2);
  bits[3] = BITS_OF(mask, 1);
  bits[4] = BITS_OF(mask, 0);
#undef BITS_OF

  // 誤り訂正符号を計算する
  struct f2x f;
  assert(f2x_alloc(&f, 14));
  f.coeffs[14] = bits[0];
  f.coeffs[13] = bits[1];
  f.coeffs[12] = bits[2];
  f.coeffs[11] = bits[3];
  f.coeffs[10] = bits[4];

  struct f2x g;
  assert(f2x_alloc(&g, 10));
  g.coeffs[10] = 1;
  g.coeffs[8] = 1;
  g.coeffs[5] = 1;
  g.coeffs[4] = 1;
  g.coeffs[2] = 1;
  g.coeffs[1] = 1;
  g.coeffs[0] = 1;

  struct f2x q, r;
  assert(f2x_alloc_div(&q, &r, &f, &g));

  bits[5] = f2x_get_coeffs(&r, 9);
  bits[6] = f2x_get_coeffs(&r, 8);
  bits[7] = f2x_get_coeffs(&r, 7);
  bits[8] = f2x_get_coeffs(&r, 6);
  bits[9] = f2x_get_coeffs(&r, 5);
  bits[10] = f2x_get_coeffs(&r, 4);
  bits[11] = f2x_get_coeffs(&r, 3);
  bits[12] = f2x_get_coeffs(&r, 2);
  bits[13] = f2x_get_coeffs(&r, 1);
  bits[14] = f2x_get_coeffs(&r, 0);

  f2x_free(&f);
  f2x_free(&g);
  f2x_free(&q);
  f2x_free(&r);

  // 出力をマスクする
  int pattern[] = {1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0};

  for (int i = 0; i < 15; i++) {
    bits[i] ^= pattern[i];
  }
}

static void render_format_info(struct qr_matrix *mat) {
  int bits[15] = {0};
  compute_format_info(mat->mode, mat->mask, bits);

  int size = matrix_size(mat->version);
#define SET(r, c, v) mat->data[(r)*size + (c)] = (v ? QRMV_PRE_B : QRMV_PRE_W)
  // 左上に埋めるパターン
  SET(0, 8, bits[0]);
  SET(1, 8, bits[1]);
  SET(2, 8, bits[2]);
  SET(3, 8, bits[3]);
  SET(4, 8, bits[4]);
  SET(5, 8, bits[5]);
  SET(7, 8, bits[6]);
  SET(8, 8, bits[7]);
  SET(8, 7, bits[8]);
  SET(8, 5, bits[9]);
  SET(8, 4, bits[10]);
  SET(8, 3, bits[11]);
  SET(8, 2, bits[12]);
  SET(8, 1, bits[13]);
  SET(8, 0, bits[14]);

  // 右上と左下に埋めるパターン
  SET(8, size - 1, bits[0]);
  SET(8, size - 2, bits[1]);
  SET(8, size - 3, bits[2]);
  SET(8, size - 4, bits[3]);
  SET(8, size - 5, bits[4]);
  SET(8, size - 6, bits[5]);
  SET(8, size - 7, bits[6]);
  SET(8, size - 8, bits[7]);
  SET(size - 8, 8, 1);
  SET(size - 7, 8, bits[8]);
  SET(size - 6, 8, bits[9]);
  SET(size - 5, 8, bits[10]);
  SET(size - 4, 8, bits[11]);
  SET(size - 3, 8, bits[12]);
  SET(size - 2, 8, bits[13]);
  SET(size - 1, 8, bits[14]);
#undef SET
}

struct data_iter {
  uint8_t const *data;
  uint8_t const *errcodes;
  int max_dit, dit, max_eit, eit;
};

static void dit_init(struct data_iter *dit, enum qr_version version,
                     enum qr_errmode mode, uint8_t const *data,
                     uint8_t const *errcodes) {
  dit->data = data;
  dit->errcodes = errcodes;
  dit->max_dit = 8 * num_blocks_data(version, mode);
  dit->max_eit = 8 * num_blocks_err(version, mode);
  dit->dit = dit->eit = 0;
}

static uint8_t const *dit_next(struct data_iter *dit) {
  if (dit->dit < dit->max_dit) return &dit->data[dit->dit++];
  if (dit->eit < dit->max_eit) return &dit->errcodes[dit->eit++];
  return NULL;
}

struct coord_iter {
  struct qr_matrix *mat;
  struct coord curr;
  int col_dir, row_dir;
};

static void cit_init(struct coord_iter *cit, struct qr_matrix *mat) {
  int size = matrix_size(mat->version);
  // 右下からスタート
  // 最初の cit_next() で右下に入るよう、初期値はもう一つ分遠くにする
  cit->mat = mat;
  cit->curr.r = size - 1;
  cit->curr.c = size;
  cit->col_dir = 1;
  cit->row_dir = 1;
}

static struct coord const *cit_step(struct coord_iter *cit) {
  int size = matrix_size(cit->mat->version);

  if (cit->curr.r >= size - 1 && cit->curr.c <= 0) {
    // すでに尽くした
    return NULL;
  }

  if (cit->curr.r == 0 && cit->col_dir == 1 && cit->row_dir == -1) {
    // 上端まで到達
    cit->curr.c--;
    if (cit->curr.c == 6) {
      // 第6列はタイミングパターンがあって一列全部潰れるのでスキップする仕様
      cit->curr.c--;
    }

    cit->row_dir = 1;
    cit->col_dir = -1;
    return &cit->curr;
  }

  if (cit->curr.r == size - 1 && cit->col_dir == 1 && cit->row_dir == 1) {
    // 下端まで到達
    cit->curr.c--;
    if (cit->curr.c == 6) {
      // 第6列はタイミングパターンがあって一列全部潰れるのでスキップする仕様
      cit->curr.c--;
    }

    cit->row_dir = -1;
    cit->col_dir = -1;
    return &cit->curr;
  }

  // それ以外
  // 行を進めるのは col_dir == 1 のときのみ
  cit->curr.r += cit->row_dir * max(0, cit->col_dir);
  cit->curr.c += cit->col_dir;
  cit->col_dir *= -1;
  return &cit->curr;
}

static struct coord const *cit_next(struct coord_iter *cit) {
  int mat_size = matrix_size(cit->mat->version);
  struct coord const *curr;
  while ((curr = cit_step(cit))) {
    assert(curr->r >= 0 && curr->c >= 0 && curr->r < mat_size &&
           curr->c < mat_size);
    uint8_t cell = cit->mat->data[curr->r * mat_size + curr->c];
    if (cell != QRMV_PRE_W && cell != QRMV_PRE_B) {
      return curr;
    }
  }

  return NULL;
}

static void render_data(struct qr_matrix *mat, uint8_t const *data,
                        uint8_t const *errcodes) {
  struct data_iter dit;
  dit_init(&dit, mat->version, mat->mode, data, errcodes);

  struct coord const *curr;
  struct coord_iter cit;
  cit_init(&cit, mat);

  int size = matrix_size(mat->version);
  while ((curr = cit_next(&cit))) {
    uint8_t const *curr_data = dit_next(&dit);
    uint8_t qrmv = curr_data && *curr_data ? QRMV_B : QRMV_W;
    mat->data[curr->r * size + curr->c] = qrmv;
  }
}

static void mask_data(struct qr_matrix *mat) {
  assert(mat->mask == qrmsk_000);

  struct coord const *curr;
  struct coord_iter cit;
  cit_init(&cit, mat);

  int size = matrix_size(mat->version);
  while ((curr = cit_next(&cit))) {
    if ((curr->c + curr->r) % 2 == 0) {
      // QRMV_W <-> QRMV_B の反転は最下位ビットの反転だけで完了する
      // (QRMV_W == 0b10, QRMV_B == 0b11 なので)
      mat->data[curr->r * size + curr->c] ^= 1;
    }
  }
}

void render(struct qr_matrix *mat, uint8_t const *data,
            uint8_t const *errcodes) {
  assert(data);
  assert(errcodes);

  render_finders(mat);
  render_alignments(mat);
  render_timings(mat);
  render_format_info(mat);
  render_data(mat, data, errcodes);
  mask_data(mat);

  return;
}
