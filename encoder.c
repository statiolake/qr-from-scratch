#include "encoder.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gf256.h"
#include "kx.h"
#include "qrcode.h"
#include "traits.h"

struct write_cursor {
  uint8_t *buf;
  int index;
  int size;
};

struct read_cursor {
  uint8_t const *buf;
  int index;
  int size;
};

static void cursor_init_write(struct write_cursor *cursor, uint8_t *buf,
                              int size) {
  cursor->buf = buf;
  cursor->index = 0;
  cursor->size = size;
}

static void cursor_init_read(struct read_cursor *cursor, uint8_t const *buf,
                             int size) {
  cursor->buf = buf;
  cursor->index = 0;
  cursor->size = size;
}

static void cursor_put_bit(struct write_cursor *cursor, uint8_t bit) {
  assert(bit == 0 || bit == 1);
  assert(cursor->index < cursor->size);
  cursor->buf[cursor->index++] = bit;
}

static void cursor_put_bits(struct write_cursor *cursor, int n, ...) {
  va_list args;
  va_start(args, n);
  for (int i = 0; i < n; i++) {
    cursor_put_bit(cursor, (uint8_t)va_arg(args, int));
  }
  va_end(args);
}

static void cursor_put_octet(struct write_cursor *cursor, uint8_t octet) {
  for (int i = 7; i >= 0; i--) {
    cursor_put_bit(cursor, (octet >> i) & 1);
  }
}

static uint8_t cursor_get_bit(struct read_cursor *cursor) {
  assert(cursor->index < cursor->size);
  return cursor->buf[cursor->index++];
}

static uint8_t cursor_get_octet(struct read_cursor *cursor) {
  // 順当に左から読むと考えるので、先頭から順に高い位のビットであることに注意
  uint8_t octet = 0;
  for (int i = 7; i >= 0; i--) {
    octet |= cursor_get_bit(cursor) << i;
  }
  return octet;
}

static void data_encode_mode(struct qr_config const *cfg,
                             struct write_cursor *cursor) {
  assert(cfg->encmode == qrenc_8bit);
  // 8bit -> 0100
  cursor_put_bits(cursor, 4, 0, 1, 0, 0);
}

static void data_encode_strlen(struct qr_config const *cfg,
                               struct write_cursor *cursor, int len) {
  assert(cfg->encmode == qrenc_8bit);
  // 8bitモードでは8bitで文字数を指定する
  assert(len < 256);
  cursor_put_octet(cursor, (uint8_t)len);
}

static void data_encode_str(struct write_cursor *cursor, char const *str) {
  while (*str) {
    cursor_put_octet(cursor, *str);
    str++;
  }
}

static void data_add_terminator(struct write_cursor *cursor) {
  // 終端に4ビット以上の空きがある場合は終端パターンとして0000を追加する
  if (cursor->size - cursor->index < 4) return;
  cursor_put_bits(cursor, 4, 0, 0, 0, 0);
}

static void data_align(struct write_cursor *cursor) {
  // 8ビットごとに区切り、最後のグループのサイズを調整する
  int extra = (8 - cursor->index % 8) % 8;
  for (int i = 0; i < extra; i++) cursor_put_bit(cursor, 0);
}

static void data_add_padding(struct qr_config const *cfg,
                             struct write_cursor *cursor) {
  // 残りの容量を11101100と00010001で埋める
  static uint8_t patterns[][8] = {
      {1, 1, 1, 0, 1, 1, 0, 0},
      {0, 0, 0, 1, 0, 0, 0, 1},
  };

  // アラインメントも済んで必ず8の倍数になっているはず
  assert(cursor->index % 8 == 0);

  int size = num_blocks_data(cfg) * 8;
  int reminder = (size - cursor->index) / 8;
  for (int p = 0; p < reminder; p++) {
    uint8_t const *pat = patterns[p % 2];
    for (int i = 0; i < 8; i++) cursor_put_bit(cursor, pat[i]);
  }
}

static void data_encode(struct qr_config const *cfg, char const *str,
                        uint8_t *data) {
  struct write_cursor cursor = {0};
  cursor_init_write(&cursor, data, num_blocks_data(cfg) * 8);

  data_encode_mode(cfg, &cursor);
  data_encode_strlen(cfg, &cursor, strlen(str));
  data_encode_str(&cursor, str);

  data_add_terminator(&cursor);

  data_align(&cursor);
  assert(cursor.index % 8 == 0);

  data_add_padding(cfg, &cursor);

  assert(cursor.index == cursor.size);
}

static bool compute_g_alloc(struct kx *g, int num_blocks_err) {
  assert(num_blocks_err > 0);

  // 求める生成多項式 g は
  // (x + a^0)(x + a^1) ... (x + a^(num_blocks_err-1))
  // らしい
  int a = gf256_from_exp(0);
  struct kx mul;
  if (!kx_alloc(ft_gf256, &mul, 1)) return false;

  if (!kx_alloc(ft_gf256, g, 0)) {
    kx_free(&mul);
    return false;
  }
  g->coeffs[0] = 1;

  for (int i = 0; i < num_blocks_err; i++) {
    mul.coeffs[0] = a;
    mul.coeffs[1] = 1;

    struct kx res;
    if (!kx_mul_alloc(&res, g, &mul)) {
      kx_free(g);
      kx_free(&mul);
      return false;
    }

    kx_free(g);
    *g = res;
    a = gf256_mul(a, gf256_from_exp(1));
  }

  return true;
}

static bool errcodes_encode_f_alloc(struct kx *f, uint8_t const *data,
                                    int num_blocks_data) {
  assert(num_blocks_data > 0);
  if (!kx_alloc(ft_gf256, f, num_blocks_data - 1)) return false;

  struct read_cursor cursor;

  cursor_init_read(&cursor, data, num_blocks_data * 8);
  for (int dim = num_blocks_data - 1; dim >= 0; dim--) {
    f->coeffs[dim] = cursor_get_octet(&cursor);
  }

  return true;
}

static void errcodes_encode(struct qr_config const *cfg, uint8_t const *data,
                            uint8_t *errcodes) {
  assert(cfg);
  assert(data);
  assert(errcodes);

  // 生成多項式 g(x) を計算する
  struct kx g;
  assert(compute_g_alloc(&g, num_blocks_err(cfg)));

  // データを多項式 f(x) の形にする
  struct kx f;
  assert(errcodes_encode_f_alloc(&f, data, num_blocks_data(cfg)));

  struct kx q, r;
  assert(kx_div_alloc(&q, &r, &f, &g));

  printf("\n");
  printf("f: ");
  kx_dump(&f);
  printf("\n");
  printf("g: ");
  kx_dump(&g);
  printf("\n");
  printf("r: ");
  kx_dump(&r);
  printf("\n");

  struct write_cursor cursor;
  cursor_init_write(&cursor, errcodes, num_blocks_err(cfg) * 8);

  for (int dim = num_blocks_err(cfg) - 1; dim >= 0; dim--) {
    cursor_put_octet(&cursor, kx_get_coeffs(&r, dim));
  }

  kx_free(&r);
  kx_free(&q);
  kx_free(&f);
  kx_free(&g);
}

void encode(struct qr_config const *cfg, char const *str, uint8_t *data,
            uint8_t *errcodes) {
  data_encode(cfg, str, data);
  errcodes_encode(cfg, data, errcodes);

  // debug
  struct read_cursor dc, ec;
  cursor_init_read(&dc, data, num_blocks_data(cfg) * 8);
  cursor_init_read(&ec, errcodes, num_blocks_err(cfg) * 8);
  for (int i = 0; i < num_blocks_data(cfg); i++) {
    printf("%d ", cursor_get_octet(&dc));
  }
  printf("\n");

  for (int i = 0; i < num_blocks_err(cfg); i++) {
    printf("%d ", cursor_get_octet(&ec));
  }
  printf("\n");
}
