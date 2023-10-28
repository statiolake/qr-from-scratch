#ifndef QRCODE_QRCODE_H_
#define QRCODE_QRCODE_H_

#include <stdint.h>

// バージョン3のみサポート
enum qr_version {
  // qrver_1,
  // qrver_2,
  qrver_3,
};

enum qr_errmode {
  qrerr_L = 1,
  qrerr_M = 0,
  // qrerr_Q = 3,
  // qrerr_H = 2,
};

// 8ビットのみをサポート
enum qr_encmode {
  // qrenc_num = 1,
  // qrenc_alnum = 2,
  qrenc_8bit = 4,
  // qrenc_kanji = 8,
};

struct qr_config {
  enum qr_version version;
  enum qr_errmode errmode;
  enum qr_encmode encmode;
};

#endif
