#include "traits.h"

#include <assert.h>

#include "qrcode.h"

int num_blocks_data(struct qr_config const *cfg) {
  // qrver_3, qrerr_M しか対応しない
  assert(cfg->version == qrver_3);
  assert(cfg->errmode == qrerr_M);
  return 44;
}

int num_blocks_err(struct qr_config const *cfg) {
  // qrver_3, qrerr_M しか対応しない
  assert(cfg->version == qrver_3);
  assert(cfg->errmode == qrerr_M);
  return 26;
}

int num_blocks_rs(struct qr_config const *cfg) {
  // qrver_3, qrerr_M しか対応しない
  assert(cfg->version == qrver_3);
  assert(cfg->errmode == qrerr_M);
  return 1;
}
