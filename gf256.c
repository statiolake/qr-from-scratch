#include "gf256.h"

#include <assert.h>

#include "field.h"
#include "kx.h"

// GF(2^8) --- GF(2)[x]/(x^8 + x^4 + x^3 + x^2 + 1)
// 7 == 0b111 -> x^2 + x^1 + x^0 ≡ x^?

void gf256_validate(int n) { assert(n < 256); }

// 数値 -> 多項式
// 必ず7次以下の多項式になる
static bool to_gf2xmod_alloc(struct kx *res, int n) {
  if (!kx_alloc(ft_gf2, res, 7)) return false;
  for (int i = 0; i < 8; i++) {
    if ((n >> i) & 1) res->coeffs[i] = 1;
  }
  kx_shrink_dim(res);

  return true;
}

// 多項式 -> 数値
static int to_256mod(struct kx *kx) {
  int res = 0;
  for (int i = 0; i < 8; i++) {
    if (kx_get_coeffs(kx, i)) res |= 1 << i;
  }

  return res;
}

static int table_to_exp[256];
static int table_to_256mod[256];

static bool init_tables(void) {
  // 初回
  // 最初は-1にしておく (あと0は特例で-1のまま)
  for (int i = 0; i < 256; i++) table_to_exp[i] = -1;

  // 最初は x^0 からスタート
  struct kx curr;
  int curr_exp = 0;
  if (!kx_alloc(ft_gf2, &curr, 0)) {
    return false;
  }
  curr.coeffs[0] = 1;

  // かけていく x
  struct kx x;
  if (!kx_alloc(ft_gf2, &x, 1)) {
    kx_free(&curr);
    return false;
  }
  x.coeffs[1] = 1;

  // 同一視する多項式
  struct kx mod;
  if (!kx_alloc(ft_gf2, &mod, 8)) {
    kx_free(&curr);
    kx_free(&x);
    return false;
  }
  mod.coeffs[8] = 1;
  mod.coeffs[4] = 1;
  mod.coeffs[3] = 1;
  mod.coeffs[2] = 1;
  mod.coeffs[0] = 1;

  // ループ
  // 数学わからんので同じ数字にぶち当たったら終わりとする
  int as_256mod;
  while (table_to_exp[(as_256mod = to_256mod(&curr))] == -1) {
    table_to_exp[as_256mod] = curr_exp;
    table_to_256mod[curr_exp] = as_256mod;

    // 次を計算
    // まずは単純にxをかけた多項式を計算する
    struct kx next_before_wrapping;
    if (!kx_mul_alloc(&next_before_wrapping, &curr, &x)) {
      kx_free(&curr);
      kx_free(&x);
      kx_free(&mod);
      return false;
    }

    // 次に、next_before_wrapping に x^8 が含まれていた場合は
    // x^8 を x^4 + x^3 + x^2 + x^0 にする
    // (係数が GF(2) なので x^8 + x^4 + x^3 + x^2 + x^0 で剰余をとるのと同じ)
    assert(next_before_wrapping.dim <= 8);
    struct kx next;
    if (kx_get_coeffs(&next_before_wrapping, 8) == 1) {
      // x^8 をより低次の多項式にする
      if (!kx_add_alloc(&next, &next_before_wrapping, &mod)) {
        kx_free(&curr);
        kx_free(&x);
        kx_free(&mod);
        kx_free(&next_before_wrapping);
        return false;
      }

      kx_free(&next_before_wrapping);
    } else {
      next = next_before_wrapping;
    }

    // 置き換え
    kx_free(&curr);
    curr = next;
    curr_exp++;
  }

  // ちゃんとできてることをチェック
  for (int i = 1; i < 256; i++) {
    assert(table_to_exp[i] >= 0);
  }
  // あと x^255 ≡ 1 になるはず。つまり、上のループは curr_exp == 255 で止まっ
  // ているはず。
  // (指数部分は特殊ケースの0多項式があるので、ループ自体は0～254の255通りになる)
  assert(curr_exp == 255);
  table_to_256mod[255] = 1;

  // 一時変数たちを解放する
  kx_free(&curr);
  kx_free(&x);
  kx_free(&mod);

  return true;
}

// 多項式をシンプルにする
// e.g. x^4 + x^3 + x^2 + x^0 を x^8 のような単項の形にする
static bool simplify_alloc(struct kx *res, struct kx *orig) {
  assert(orig->dim < 8);

  if (table_to_exp[0] == 0) {
    if (!init_tables()) return false;
  }

  int res_exp = table_to_exp[to_256mod(orig)];
  if (res_exp == -1) {
    // 0の場合なので0多項式を返す
    kx_alloc(ft_gf2, res, 0);
    res->coeffs[0] = 0;
    return true;
  } else {
    // x^res_exp が1になっている多項式を作る
    kx_alloc(ft_gf2, res, res_exp);
    res->coeffs[res_exp] = 1;
    return true;
  }
}

int gf256_from_exp(int exp) {
  assert(exp < 255);
  if (table_to_256mod[exp] == 0) {
    assert(init_tables());
    assert(table_to_256mod[exp] > 0);
  }

  return table_to_256mod[exp];
}

int gf256_add(int a, int b) {
  struct kx kxa, kxb, res;
  assert(to_gf2xmod_alloc(&kxa, a));
  assert(to_gf2xmod_alloc(&kxb, b));
  kx_add_alloc(&res, &kxa, &kxb);
  int n = to_256mod(&res);
  kx_free(&kxa);
  kx_free(&kxb);
  kx_free(&res);
  return n;
}

int gf256_sub(int a, int b) {
  struct kx kxa, kxb, res;
  assert(to_gf2xmod_alloc(&kxa, a));
  assert(to_gf2xmod_alloc(&kxb, b));
  assert(kx_sub_alloc(&res, &kxa, &kxb));
  int n = to_256mod(&res);
  kx_free(&kxa);
  kx_free(&kxb);
  kx_free(&res);
  return n;
}

int gf256_mul(int a, int b) {
  struct kx kxa, kxb, minpoly, res, quot, rem;

  // GF(2^8)の原始多項式
  assert(kx_alloc(ft_gf2, &minpoly, 8));
  minpoly.coeffs[8] = 1;
  minpoly.coeffs[4] = 1;
  minpoly.coeffs[3] = 1;
  minpoly.coeffs[2] = 1;
  minpoly.coeffs[0] = 1;

  assert(to_gf2xmod_alloc(&kxa, a));
  assert(to_gf2xmod_alloc(&kxb, b));
  assert(kx_mul_alloc(&res, &kxa, &kxb));

  // 剰余をとる
  assert(kx_div_alloc(&quot, &rem, &res, &minpoly));
  int n = to_256mod(&rem);

  kx_free(&minpoly);
  kx_free(&kxa);
  kx_free(&kxb);
  kx_free(&res);
  kx_free(&quot);
  kx_free(&rem);

  return n;
}

int gf256_div(int a, int b) {
  assert(b != 0);

  struct kx kxa, kxb;
  assert(to_gf2xmod_alloc(&kxa, a));
  assert(to_gf2xmod_alloc(&kxb, b));

  // 単純にする
  struct kx kxsa, kxsb;
  assert(simplify_alloc(&kxsa, &kxa));
  assert(simplify_alloc(&kxsb, &kxb));

  // もし kxsa のほうが次元が低い場合は、kxsa を x^255 倍する
  // (x^255 ≡ 1 なので問題ない)
  if (kxsa.dim < kxsb.dim) {
    struct kx x255;
    assert(kx_alloc(ft_gf2, &x255, 255));
    x255.coeffs[255] = 1;
    struct kx kxsae;
    assert(kx_mul_alloc(&kxsae, &kxsa, &x255));
    kx_free(&kxsa);
    kxsa = kxsae;
  }

  // 割る
  struct kx q, r;
  assert(kx_div_alloc(&q, &r, &kxsa, &kxsb));
  assert(r.dim == 0 && r.coeffs[0] == 0);

  // q = x^? みたいになっているはず。チェック。
  for (int i = 0; i < q.dim; i++) assert(q.coeffs[i] == 0);
  assert(q.coeffs[q.dim] == 1);

  // x^? の ? を 256mod に直す。
  int res = gf256_from_exp(q.dim);

  kx_free(&q);
  kx_free(&r);
  kx_free(&kxsa);
  kx_free(&kxsb);

  return res;
}
