#include "../质数筛/欧拉筛.cpp"
#include "../质数筛/埃氏筛.cpp"
#include "../质数筛/区间筛质数.cpp"
#include "../质数筛/欧拉筛.cpp"
#include "../质数筛/埃氏筛.cpp"

// 独立 Sundaram 枚举奇合数: (2i+1)(2j+1)=2(i+j+2ij)+1, 不使用两份筛的循环。
vector<char> reference_primes(int n)
{
    int m = (n - 1) / 2;
    vector<char> removed(max(m + 1, 1), false), result(n + 1, false);
    if (n >= 2) result[2] = true;
    for (LL i = 1; 2 * i * (i + 1) <= m; ++i)
        for (LL j = i; i + j + 2 * i * j <= m; ++j) removed[i + j + 2 * i * j] = true;
    for (int i = 1; i <= m; ++i) if (!removed[i]) result[2 * i + 1] = true;
    return result;
}
bool trial_prime(LL x)
{
    if (x < 2) return false;
    for (LL d = 2; d <= x / d; ++d) if (x % d == 0) return false;
    return true;
}
vector<PII> trial_factors(int x)
{
    vector<PII> ans;
    for (int d = 2; d <= x / d; ++d)
    {
        int count = 0;
        while (x % d == 0) { x /= d; ++count; }
        if (count) ans.push_back({d, count});
    }
    if (x > 1) ans.push_back({x, 1});
    return ans;
}
void verify(const LinearSieve& a, const Eratosthenes& b, const vector<char>& expected)
{
    int n = a.n;
    assert(b.n == n && a.lp.size() == (size_t)n + 1 && b.prime.size() == (size_t)n + 1);
    size_t count = 0;
    for (int i = 0; i <= n; ++i)
    {
        assert(a.is_prime(i) == (bool)expected[i] && b.is_prime(i) == (bool)expected[i]);
        if (expected[i])
        {
            assert(count < a.primes.size() && a.primes[count] == i);
            assert(count < b.primes.size() && b.primes[count] == i);
            ++count;
        }
        if (i < 2) { assert(a.lp[i] == 0); continue; }
        int p = a.lp[i], q = i / p;
        assert(p >= 2 && expected[p] && i % p == 0);
        // 质因子序列非降 + 相乘为原数, 用唯一分解验证 lp 确为最小值。
        assert(q == 1 || p <= a.lp[q]);
    }
    assert(a.primes.size() == count && b.primes.size() == count);
}
void test_sieves()
{
    mt19937 rng(42);
    LinearSieve a, untouched(100);
    Eratosthenes b, stable(100);
    auto old_lp = untouched.lp, old_primes = stable.primes;
    for (int tc = 0; tc < 300; ++tc)
    {
        int n = rng() % 1001;
        a.init(n); b.init(n);
        vector<char> ref(n + 1);
        for (int i = 0; i <= n; ++i) ref[i] = trial_prime(i);
        assert(reference_primes(n) == ref); // 先以试除逐点验独立大表参照
        verify(a, b, ref);
        for (int i = 1; i <= n; ++i) assert(a.factorize(i) == trial_factors(i));
    }
    for (int n : {10000000, 0, 1, 2, 257, 1000000, 10000000})
    {
        a.init(n); b.init(n);
        auto ref = reference_primes(n);
        verify(a, b, ref);
        for (int q = 0; q < min(n, 20000); ++q)
        {
            int x = 1 + rng() % n;
            auto fac = a.factorize(x);
            LL product = 1;
            int prev = 1;
            for (auto [p, e] : fac)
            {
                assert(p > prev && ref[p] && e > 0);
                for (int k = 0; k < e; ++k) product *= p;
                prev = p;
            }
            assert(product == x);
        }
        assert(untouched.lp == old_lp && stable.primes == old_primes);
    }
}
void test_segments()
{
    auto ref = reference_primes(1000000);
    vector<LL> all;
    for (int i = 2; i <= 1000000; ++i) if (ref[i]) all.push_back(i);
    assert(segmented_sieve(0, 1000000) == all);
    assert(segmented_sieve(500000, 1000000) == vector<LL>(lower_bound(all.begin(), all.end(), 500000), all.end()));
    assert(segmented_sieve(LLONG_MIN, 1).empty());
    assert(segmented_sieve(LLONG_MAX, 0).empty());
    for (LL p : {2, 3, 5, 97, 997, 999983})
    {
        LL l = p * p - 128, r = p * p + 128;
        vector<LL> expected;
        for (LL x = max(2LL, l); x <= r; ++x) if (trial_prime(x)) expected.push_back(x);
        assert(segmented_sieve(l, r) == expected);
        assert(segmented_sieve(p * p, p * p).empty());
    }
    mt19937_64 rng(42);
    for (int t = 0; t < 300; ++t)
    {
        LL l = rng() % 10000000000LL, r = l + rng() % 7;
        vector<LL> expected;
        for (LL x = l; x <= r; ++x) if (trial_prime(x)) expected.push_back(x);
        assert(segmented_sieve(l, r) == expected);
    }
}
int main()
{
    test_sieves();
    test_segments();
    cout << "number sieves: PASS (10000000 table, rebuild, 1000000 interval, high squares)\n";
}
