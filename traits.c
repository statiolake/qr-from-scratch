#include "traits.h"

#include <assert.h>

#include "qrcode.h"

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
