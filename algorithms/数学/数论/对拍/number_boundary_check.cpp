#include <cassert>
#include <climits>
#include <iostream>
#include <random>
#include "../整数开方.cpp"
#include "../区间筛质数.cpp"
#include "../约数个数及约数和公式法.cpp"
#include "../整数开方.cpp"
#include "../区间筛质数.cpp"
#include "../约数个数及约数和公式法.cpp"

// 用整数二分求平方根, 避开模板的浮点估计和修正过程
LL brute_sqrt(LL x)
{
    if (x <= 0) return 0;
    LL l = 0, r = 3037000500LL;
    while (l < r)
    {
        LL m = (l + r + 1) / 2;
        if ((__int128)m * m <= x) l = m;
        else r = m - 1;
    }
    return l;
}
bool prime(LL x)
{
    if (x < 2) return false;
    for (LL d = 2; d <= x / d; d++)
        if (x % d == 0) return false;
    return true;
}
void check_sqrt(LL x)
{
    LL f = brute_sqrt(x);
    assert(floor_isqrt(x) == f);
    assert(ceil_isqrt(x) == f + (x > 0 && (__int128)f * f != x));
}
int main()
{
    for (LL x : {LLONG_MIN, -1LL, 0LL, 1LL, 2LL, 3LL, 4LL,
                 9223372030926249000LL, 9223372030926249001LL,
                 9223372030926249002LL, LLONG_MAX}) check_sqrt(x);
    std::mt19937 rng(42);
    for (int t = 0; t < 1000; t++)
    {
        LL x = (LL)((((unsigned long long)rng() << 32) | rng()) >> 1);
        check_sqrt(x);
        LL l = (int)(rng() % 10000) - 20, r = l + rng() % 100;
        std::vector<LL> ref;
        for (LL i = l; i <= r; i++) if (prime(i)) ref.push_back(i);
        assert(segmented_sieve(l, r) == ref);
        int n = rng() % 10000 + 1;
        LL s = 0;
        int c = 0;
        for (int d = 1; d <= n; d++) if (n % d == 0) { s += d; c++; }
        deprime(n);
        assert(sum == s && cnt == c);
    }
    assert((segmented_sieve(2, 3) == std::vector<LL>{2, 3}));
    assert(segmented_sieve(3, 2).empty());
    assert(segmented_sieve(-10, 1).empty());
    // 独立枚举因子对, 覆盖 int 上界的循环条件和约数和溢出
    for (int n : {INT_MAX, 2000000000, 1800000000})
    {
        LL s = 0;
        int c = 0;
        for (LL d = 1; d * d <= n; d++)
            if (n % d == 0)
            {
                s += d; c++;
                if (d != n / d) { s += n / d; c++; }
            }
        deprime(n);
        assert(sum == s && cnt == c);
    }
    std::cout << "number boundaries: PASS\n";
}
