// ============ z_rnd 随机数回归套件 ============
// 覆盖: 值域包含(300组随机区间) | 覆盖+均匀性([0,9] 打1e6发, ±33σ松界) |
//       缺省下界0生效 | long long / 无符号 / 浮点路径
// 纪律: 改动 z_rnd 模板, 必重跑本套件
// 跑法: g++ -std=c++20 -Wall -Wextra -O2 rnd_check.cpp -o rnd_check && ./rnd_check
#include <cassert>
#include <cstdio>
#include <random>
#include "../z_rnd.cpp"
using namespace std;

template<class T>
static void integer_edges()
{
    mt19937_64 a(42), b(42);
    T lo = numeric_limits<T>::lowest(), hi = numeric_limits<T>::max();
    for (int i = 0; i < 10000; i++)
    {
        T x = z_rnd(hi, lo, a), y = z_rnd(hi, lo, b);
        assert(x == y && x >= lo && x <= hi);
    }
    assert(z_rnd(lo, lo, a) == lo && z_rnd(hi, hi, a) == hi);
    for (int i = 0; i < 1000; i++) assert(z_rnd((T)1, (T)0, a) <= (T)1);
}
struct ZeroEngine
{
    using result_type = uint64_t;
    static constexpr uint64_t min() { return 0; }
    static constexpr uint64_t max() { return UINT64_MAX; }
    uint64_t operator()() { return 0; }
};
struct MaxEngine : ZeroEngine
{
    uint64_t operator()() { return UINT64_MAX; }
};
template<class T>
static void floating_edges()
{
    mt19937_64 a(123), b(123);
    const T max = numeric_limits<T>::max(), tiny = numeric_limits<T>::denorm_min();
    for (auto [lo, hi] : {pair<T,T>{-max, max}, {-max, -max / 2}, {max / 2, max},
                          {-tiny, tiny}, {0, tiny}, {1, nextafter(T(1), T(2))}, {-10, 20}})
    {
        for (int i = 0; i < 10000; i++)
        {
            T x = z_rnd(hi, lo, a), y = z_rnd(hi, lo, b);
            assert(isfinite(x) && x >= lo && x < hi && x == y);
        }
        ZeroEngine z;
        MaxEngine m;
        assert(z_rnd(hi, lo, z) == lo);
        T top = z_rnd(hi, lo, m);
        assert(isfinite(top) && top >= lo && top < hi);
    }
    assert(z_rnd(max, max, a) == max);
    assert(z_rnd(-max, -max, a) == -max);
    assert(z_rnd(T(0), T(0), a) == 0);
    int buckets[10]{};
    for (int i = 0; i < 200000; i++) buckets[(int)(z_rnd(T(1), T(0), a) * 10)]++;
    for (int c : buckets) assert(18000 < c && c < 22000);
}
int main()
{
    integer_edges<bool>(); integer_edges<char>();
    integer_edges<signed char>(); integer_edges<unsigned char>();
    integer_edges<short>(); integer_edges<unsigned short>();
    integer_edges<int>(); integer_edges<unsigned>();
    integer_edges<long>(); integer_edges<unsigned long>();
    integer_edges<long long>(); integer_edges<unsigned long long>();
    integer_edges<wchar_t>(); integer_edges<char8_t>(); integer_edges<char16_t>(); integer_edges<char32_t>();
    floating_edges<float>(); floating_edges<double>(); floating_edges<long double>();
    mt19937 par(42);
    // 300 组随机区间: 值域包含
    for (int t = 0; t < 300; t++)
    {
        int lo = (int)(par() % 2001) - 1000;
        int hi = lo + (int)(par() % 2001);
        for (int i = 0; i < 100; i++)
        {
            int x = z_rnd(hi, lo);
            assert(lo <= x && x <= hi);
        }
    }
    // [0,9] 打 1e6 发: 全值覆盖 + 均匀性 (期望 1e5, ±1e4 为 33σ 松界)
    // 兼验缺省下界 0 生效 (cnt[0] 命中)
    int cnt[10] = {0};
    for (int i = 0; i < 1000000; i++) cnt[z_rnd(9)]++;
    for (int v = 0; v < 10; v++) assert(90000 <= cnt[v] && cnt[v] <= 110000);
    // long long / 无符号 / 浮点路径
    for (int i = 0; i < 100; i++)
    {
        long long x = z_rnd(1LL << 62, -(1LL << 62));
        assert(-(1LL << 62) <= x && x <= (1LL << 62));
        size_t u = z_rnd((size_t)100);
        assert(u <= 100);
        double d = z_rnd(1.0);
        assert(0.0 <= d && d < 1.0);
    }
    puts("z_rnd check passed");
    return 0;
}
