// zoi: rnd
#ifndef Z_OI_ZRND
#define Z_OI_ZRND

#include "../utils/utils.cpp"

// 整数双闭 [lo,hi], 浮点有限半开 [lo,hi), 两端相等返回该值; 上界在前
// 原生至多 64 位整数含 bool/字符, 浮点支持有限端点; 期望 O(1), O(1) 额外空间
template <class T, uniform_random_bit_generator G>
    requires ((integral<T> && sizeof(T) <= 8) || floating_point<T>)
T z_rnd(T hi, T lo, G& rng)
{
    assert(lo <= hi);
    if constexpr (integral<T>)
    {
        using W = conditional_t<is_signed_v<T>, LL, ULL>;
        return (T)uniform_int_distribution<W>((W)lo, (W)hi)(rng);
    }
    else
    {
        assert(isfinite(lo) && isfinite(hi));
        if (lo == hi) return lo;
        T u = generate_canonical<T, numeric_limits<T>::digits>(rng);
        T x = lerp(lo, hi, u); // 不直接计算 hi-lo, 避免相反大端点溢出
        return x < hi ? x : nextafter(hi, lo);
    }
}
// 缺省下界为 0, 按类型复用 mt19937_64; 仅用于算法随机化, 不生成安全密钥
template <class T = int> requires ((integral<T> && sizeof(T) <= 8) || floating_point<T>)
T z_rnd(T hi, T lo = 0)
{
    static mt19937_64 rng(chrono::steady_clock::now().time_since_epoch().count());
    return z_rnd(hi, lo, rng);
}
#endif

/* Usage
#include "rnd.h"

int main()
{
    int a = z_rnd(10);                  // [0,10]
    LL b = z_rnd(LLONG_MAX, LLONG_MIN); // 同类型参数, 全 LL 区间合法
    double c = z_rnd(1.0);              // [0,1)
    cout << (0 <= a && a <= 10) << ' ' << (0 <= c && c < 1) << '\n'; // 1 1
    cout << (b >= LLONG_MIN) << '\n';   // 1
    mt19937_64 fixed(42);
    int d = z_rnd(9, 0, fixed);         // 自传引擎可复现, 不改变默认引擎的状态
    cout << (0 <= d && d <= 9) << '\n'; // 1, 不承诺不同标准库的分布输出序列相同
}
*/
