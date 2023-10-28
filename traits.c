#include "traits.h"

#include <assert.h>

#include "qrcode.h"

size_t num_blocks_data(struct qr_config const *cfg) {
  // 一部形式のみ対応する
  assert(cfg->version == qrver_3);
  assert(cfg->errmode == qrerr_M || cfg->errmode == qrerr_L);
  return cfg->errmode == qrerr_M ? 44 : 55;
}

size_t num_blocks_err(struct qr_config const *cfg) {
  // 一部形式のみ対応する
  assert(cfg->version == qrver_3);
  assert(cfg->errmode == qrerr_M || cfg->errmode == qrerr_L);
  return cfg->errmode == qrerr_M ? 26 : 15;
  return 26;
}

size_t num_blocks_rs(struct qr_config const *cfg) {
  // 一部形式のみ対応する
  assert(cfg->version == qrver_3);
  assert(cfg->errmode == qrerr_M || cfg->errmode == qrerr_L);
  return 1;
}

size_t num_max_length(struct qr_config const *cfg) {
  // 一部形式のみ対応する
  assert(cfg->version == qrver_3);
  assert(cfg->errmode == qrerr_M || cfg->errmode == qrerr_L);
  assert(cfg->encmode == qrenc_8bit);
  return cfg->errmode == qrerr_M ? 42 : 53;
}
