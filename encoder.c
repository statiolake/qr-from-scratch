#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "qrcode.h"

int main(void) {
  // 文字列格納
  char input_text[26];
  // scanf("%s", input_text);
  strcpy(input_text, "hello WORLD");
  printf("input_text: %s\n", input_text);

  // モード指示子　英数字モードは0010
  char mode_indicator[5];
  strcpy(mode_indicator, "0010");
  printf("mode_indicator: %s\n", mode_indicator);

  // 文字数カウント
  int input_text_length = strlen(input_text);
  printf("input_text_length: %d\n", input_text_length);

  // 文字数指示子　英数字モードなら9bitで表現
  char length_indicator[10];                     // 長さ9 + null文字
  itoa(input_text_length, length_indicator, 2);  // 文字数の2進数表記を計算

  // 文字列の長さが9未満の場合、前にゼロを追加
  while (strlen(length_indicator) < 9) {
    char temp[10];
    strcpy(temp, "0");
    strcat(temp, length_indicator);
    strcpy(length_indicator, temp);
  }
  printf("length_indicator: %s\n", length_indicator);

  // データの2進数化
  // まずは対応表に従って数値化
  char datacode_data_conv[input_text_length];
  for (int i = 0; i < input_text_length; i++) {
    if ((48 <= input_text[i]) && (input_text[i] <= 57)) {
      datacode_data_conv[i] = input_text[i] - 48;
    }  // 数字
    if ((65 <= input_text[i]) && (input_text[i] <= 90)) {
      datacode_data_conv[i] = input_text[i] - 55;
    }  // 大文字
    if ((97 <= input_text[i]) && (input_text[i] <= 126)) {
      datacode_data_conv[i] = input_text[i] - 87;
    }  // 小文字
    if (input_text[i] == 32) {
      datacode_data_conv[i] = 36;
    }  // space
    if (input_text[i] == 36) {
      datacode_data_conv[i] = 37;
    }  // $
    if (input_text[i] == 37) {
      datacode_data_conv[i] = 38;
    }  // %
    if (input_text[i] == 42) {
      datacode_data_conv[i] = 39;
    }  // *
    if (input_text[i] == 43) {
      datacode_data_conv[i] = 40;
    }  // +
    if (input_text[i] == 45) {
      datacode_data_conv[i] = 41;
    }  // -
    if (input_text[i] == 46) {
      datacode_data_conv[i] = 42;
    }  // .
    if (input_text[i] == 47) {
      datacode_data_conv[i] = 43;
    }  // /
    if (input_text[i] == 58) {
      datacode_data_conv[i] = 44;
    }  // :
  }

  for (int i = 0; i < input_text_length; i++) {
    printf("%d ", datacode_data_conv[i]);
  }
  printf("\n");

  // 二桁ずつ区切って、１桁目を45倍、それに2桁目を足す
  int datacode_data_dec[input_text_length / 2];  // 計算後の10進数表記
  char datacode_data_bin[11];  // 計算後の2進数表記 11bit

  for (int i = 0; i < (input_text_length / 2); i++) {
    datacode_data_dec[i] =
        datacode_data_conv[i * 2] * 45 + datacode_data_conv[(i * 2) + 1];
    itoa(datacode_data_dec[i], datacode_data_bin,
         2);  // 文字数の2進数表記を計算
    // 11bitに調整
    while (strlen(datacode_data_bin) < 11) {
      char temp[12];
      strcpy(temp, "0");
      strcat(temp, datacode_data_bin);
      strcpy(datacode_data_bin, temp);
    }
    printf("%d: %s \n", datacode_data_dec[i], datacode_data_bin);
  }

  // 二桁にならない最後はそのまま
  if (input_text_length % 2 == 1) {
    datacode_data_dec[input_text_length / 2] =
        datacode_data_conv[input_text_length - 1];
    itoa(datacode_data_dec[input_text_length / 2], datacode_data_bin,
         2);  // 文字数の2進数表記を計算
    // 6bitに調整
    while (strlen(datacode_data_bin) < 6) {
      char temp[7];
      strcpy(temp, "0");
      strcat(temp, datacode_data_bin);
      strcpy(datacode_data_bin, temp);
    }
    printf("%d: %s \n", datacode_data_dec[input_text_length / 2],
           datacode_data_bin);
  }

  // この後の作業

  // 以下の0-1文字列を順に一列にする
  // モード指示子 mode_indicator[5]　4bit
  // 文字数指示子　length_indicator[10] 9bit
  // 数値化した文字列　datacode_data_bin
  // 11bitだったり6bitだったり　そういえば保存してないや

  // 1列にした0-1文字列を8bitごとに区切る
  // 最後が8bitにならなかったら0埋め

  // 8bitの個数がデータコード数に満たなければ11101100 および
  // 00010001を交互に付加 10進数表記に変換　これが多項式の項になる

  // 誤り訂正符号の計算
  // ガロア体の指数表現-整数表記のリストを作成
  int Galois_int[256];  // 指数を入れると整数表記がわかるリスト
  int Galois_exp[256];  // 整数表記を入れると指数がわかるリスト

  // 初期値
  Galois_int[0] = 1;
  Galois_exp[0] = 0;

  // 指数表記を順に計算　expはα^8=α^4+α^3+α^2+1というもの。そういう世界。
  for (int i = 0; i < 255; i++) {
    Galois_int[i + 1] = Galois_int[i] * 2;
    if (Galois_int[i + 1] >= 256) {
      Galois_int[i + 1] -= 256;
      Galois_int[i + 1] ^= 29;
    }
    Galois_exp[i] = 0;
  }

  // Fill the Galois_exp array
  for (int i = 0; i <= 255; i++) {
    for (int j = 0; j < 255; j++) {
      if (Galois_int[j] == i) {
        Galois_exp[i] = j;
      }
    }
  }

  for (int i = 0; i < 256; i++) {
    printf("Galois_exp[%d] = %d\n", i, Galois_exp[i]);
  }

  // 多項式f(x)とg(x)を用意　 int fx[MAX_DEGREE + 1] = {0, 0,　... ,
  // 0}でいいかな fx[i] でf(x)のx^iの項がわかるイメージで。 MAX_DEGREE
  // は(データコード数 - 1) * 誤り訂正コード語数

  // f(x)には先ほどの10進数表記にしたデータを***ガロア体の指数表現に変換して***
  // 最大次数からいれていく Galois_exp[i]使えばok　残りは0埋め
  // g(x)は生成多項式　大きさ固定ならハードコーディングしていいや　https://www.swetake.com/qrcode/qr_table3.html
  // int gx[26+1] = {70, 218, 145, 153, 227, 48, 102, 13, 142, 245, 21, 161, 53,
  // 165, 28, 111, 201, 145, 17, 118, 182, 103, 2, 158, 125, 173, 1}
  // f(x)をg(x)で割って余りを求める。ただし以下のステップで行う必要がある
  // g(x)をf(x)の最大次数に合わせて掛け算　普通の割り算と同じステップ　
  // ただし、指数表記なので足し算になる　しかも**255=0**
  // 項を整数表記に変換する　Galois_int[i]使えばok
  // これを2進数に変換
  // 最大次数に合わせて掛け算したg(x)でf(x)を引く　普通の割り算と同じステップ
  // ただし、上記のように2進数で考え、**各bitでXORを取る**。　GF(2)の加法は1+1=0
  // 指数表記に戻す Galois_exp[i]使えばok
  // これをg(x)の次数以下になるまで繰り返す

  return 0;
}
