// zoi: binomLoop
#ifndef Z_OI_BINOM_LOOP
#define Z_OI_BINOM_LOOP

#include "../../杂项/128位整数/128int.cpp"

// 返回 C(n,k) mod p, k 越界返回 0; n>=0, p 为素数, k'=min(k,n-k)<p
// O(k'+log p) 时间, O(1) 空间; n/p 可至 LL 上界, 不预处理到 n, 适合大 n 小 k
inline LL binom_loop(LL n, LL k, LL p)
{
    assert(n >= 0 && p >= 2);
    if (k < 0 || k > n) return 0;
    k = min(k, n - k);
    assert(k < p);
    LL up = 1, down = 1;
    for (LL i = 1; i <= k; i++)
    {
        up = (i128)up * (n - i + 1) % p;
        down = (i128)down * i % p;
    }
    LL inv = 1;
    for (LL e = p - 2; e; e >>= 1, down = (i128)down * down % p)
        if (e & 1) inv = (i128)inv * down % p;
    return (i128)up * inv % p;
}
#endif

/* Usage
#include "binomLoop.h"

int main()
{
    LL n, k, p;
    if (!(cin >> n >> k >> p)) return 0; // 输入示例: 1000000000000 2 1000000007
    cout << binom_loop(n, k, p) << '\n'; // 24503500
    cout << binom_loop(8, 1, 7) << '\n'; // 1, n 可以达到或超过 p
    cout << binom_loop(8, 7, 7) << '\n'; // 1, 对称后只循环 1 次
    // 每次重新循环; n 可预处理且查询很多时, 用 mint::comb 或 PrimeComb
    // 若 min(k,n-k)>=p, 分母可能含 p 因子, 不能调用此函数, 改用 Lucas 等方法
}
*/
