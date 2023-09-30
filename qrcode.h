#ifndef QRCODE_QRCODE_H_
#define QRCODE_QRCODE_H_

#include <stdint.h>

// バージョン3のみサポート
enum qr_version {
  // qrver_1,
  // qrver_2,
  qrver_3,
};

// Hのみサポート
enum qr_errmode {
  // qrerr_L,
  qrerr_M,
  // qrerr_Q,
  // qrerr_H,
};

// 8ビットのみをサポート
enum qr_encmode {
  // qrenc_num,
  // qrenc_alnum,
  qrenc_8bit,
  // qrenc_kanji,
};

#endif
