#include <random>
#include <chrono>
#include <cassert>
#include <concepts>

using namespace std;

#ifndef Z_OI_ZRND
#define Z_OI_ZRND
// ============ z_rnd 范围随机数 ============
// 均匀随机数: 整数双闭 [lo,hi] / 浮点半开 [lo,hi), 返回 T; 缺省 lo = 0
template <class T = int> requires integral<T> || floating_point<T>
T z_rnd(T hi, T lo = 0)
{
    assert(lo <= hi);
    static mt19937_64 rng(chrono::steady_clock::now().time_since_epoch().count());
    if constexpr (integral<T>)
        return uniform_int_distribution<T>(lo, hi)(rng);
    else
        return uniform_real_distribution<T>(lo, hi)(rng);
}
#endif
/* Usage:
    int a = z_rnd(n);            // [0, n] 整数, 缺省下界 0
    int b = z_rnd(r, l);         // [l, r] 整数 (上界前, 下界后)
    LL c = z_rnd(w, 1);          // w 为 LL 时自动推导
    double d = z_rnd(1.0);       // [0, 1) 实数, 浮点字面量自动推导
*/
