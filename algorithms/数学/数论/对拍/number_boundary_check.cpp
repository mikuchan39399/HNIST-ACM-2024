#include <cassert>
#include <climits>
#include <iostream>
#include <random>
#include "../整数开方.cpp"
#include "../区间筛质数.cpp"
#include "../约数个数及约数和公式法.cpp"
#include "../除法上下取整.cpp"
#include "../整数开方.cpp"
#include "../区间筛质数.cpp"
#include "../约数个数及约数和公式法.cpp"
#include "../除法上下取整.cpp"

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
void test_division()
{
    auto check = [](LL a, LL b)
    {
        if (!b || (a == LLONG_MIN && b == -1)) return;
        __int128 x = a, y = b;
        if (y < 0) { x = -x; y = -y; }
        auto floor_ref = [](__int128 p, __int128 q)
        { return p >= 0 ? p / q : -((-p + q - 1) / q); };
        assert(floor_div(a, b) == floor_ref(x, y));
        assert(ceil_div(a, b) == -floor_ref(-x, y));
    };
    for (LL a = -200; a <= 200; ++a)
        for (LL b = -50; b <= 50; ++b) check(a, b);
    VLL edge{LLONG_MIN, LLONG_MIN + 1, -3037000500LL, -2, -1, 0, 1, 2, 3037000500LL, LLONG_MAX - 1, LLONG_MAX};
    for (LL a : edge) for (LL b : edge) check(a, b);
    mt19937_64 rng(20260908);
    for (int t = 0; t < 200000; ++t) check(bit_cast<LL>(rng()), bit_cast<LL>(rng()));
}
void test_scale()
{
    mt19937_64 rng(20260908);
    for (int t = 0; t < 100000; ++t)
    {
        check_sqrt(rng() & 0x7fffffffffffffffULL);
        LL root = 1 + rng() % 3037000499LL, square = root * root;
        check_sqrt(square - 1); check_sqrt(square); check_sqrt(square + 1);
    }
    for (int d = 0; d <= 10000; ++d) check_sqrt(LLONG_MAX - d);
    constexpr int n = 200000;
    VI counts(n + 1);
    VLL sums(n + 1);
    // 独立枚举每个约数的倍数, 不复用质因数分解/乘法公式。
    for (int d = 1; d <= n; ++d)
        for (int x = d; x <= n; x += d) { ++counts[x]; sums[x] += d; }
    for (int x = 1; x <= n; ++x)
    {
        auto [count, sum] = deprime(x);
        assert(count == counts[x] && sum == sums[x]);
    }
    for (int t = 0; t < 300; ++t)
    {
        int x = (t < 100) ? INT_MAX - t : 1 + rng() % INT_MAX;
        int count = 0; LL sum = 0;
        for (LL d = 1; d * d <= x; ++d)
            if (x % d == 0)
            {
                ++count; sum += d;
                if (d != x / d) { ++count; sum += x / d; }
            }
        assert((deprime(x) == pair<int, LL>{count, sum}));
    }
    auto saved = deprime(12);
    assert((deprime(1) == pair<int, LL>{1, 1}));
    assert((saved == pair<int, LL>{6, 28}));
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
        auto [cnt, sum] = deprime(n);
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
        auto [cnt, sum] = deprime(n);
        assert(sum == s && cnt == c);
    }
    test_division();
    test_scale();
    std::cout << "number boundaries: PASS (LL division/sqrt, 200000 divisor results)\n";
}
