#include "../循环求组合数.cpp"
#include "../打表杨辉三角求组合数.cpp"
#include "../阶乘表及阶乘逆元表求组合数.cpp"
#include "../卢卡斯定理求组合数.cpp"
#include "../循环求组合数.cpp"
#include "../打表杨辉三角求组合数.cpp"
#include "../阶乘表及阶乘逆元表求组合数.cpp"
#include "../卢卡斯定理求组合数.cpp"

// 独立参照使用倍加模乘、精确小组合数、整数因子约分与阶乘质因数赋值
static mt19937_64 rng(20260908);
static unsigned long long checks = 0;

static void eq(LL got, LL want)
{
    ++checks;
    if (got != want)
    {
        cerr << "comb mismatch: " << got << " != " << want << '\n';
        abort();
    }
}
static LL mul_ref(LL a, LL b, LL m)
{
    ULL x = a % m, y = b, r = 0, mod = m;
    while (y)
    {
        if (y & 1) r = (r + x) % mod;
        x = (x + x) % mod;
        y >>= 1;
    }
    return r;
}
static LL pow_ref(LL a, LL e, LL m)
{
    LL r = 1 % m;
    for (int bit = 62; bit >= 0; bit--)
    {
        r = mul_ref(r, r, m);
        if ((e >> bit) & 1) r = mul_ref(r, a, m);
    }
    return r;
}
static LL choose_cancel(LL n, int k, LL mod)
{
    if (k < 0 || k > n) return 0;
    k = (int)min<LL>(k, n - k);
    VLL factors;
    for (int j = 0; j < k; j++) factors.push_back(n - j);
    for (int d = 2; d <= k; d++)
    {
        LL left = d;
        for (LL& x : factors)
        {
            LL g = gcd(x, left);
            x /= g;
            left /= g;
            if (left == 1) break;
        }
        assert(left == 1);
    }
    LL r = 1 % mod;
    for (LL x : factors) r = mul_ref(r, x, mod);
    return r;
}
static VI primes_to(int n)
{
    vector<bool> composite(n + 1);
    VI primes;
    for (int i = 2; i <= n; i++)
        if (!composite[i])
        {
            primes.push_back(i);
            if ((LL)i * i <= n)
                for (int j = i * i; j <= n; j += i) composite[j] = true;
        }
    return primes;
}
static LL valuation(LL n, int p)
{
    LL r = 0;
    while (n) r += n /= p;
    return r;
}
static LL choose_factors(int n, int k, LL mod, const VI& primes)
{
    LL r = 1 % mod;
    for (int p : primes)
    {
        if (p > n) break;
        LL e = valuation(n, p) - valuation(k, p) - valuation(n - k, p);
        if (e) r = mul_ref(r, pow_ref(p, e, mod), mod);
    }
    return r;
}
static LL inverse_ref(LL x, LL mod)
{
    LL a = x, b = mod, s = 1, t = 0;
    while (b)
    {
        LL q = a / b;
        LL na = a - q * b, ns = s - q * t;
        a = b; b = na; s = t; t = ns;
    }
    assert(a == 1);
    s %= mod;
    return s < 0 ? s + mod : s;
}
// 去掉阶乘中全部 p 因子, 用 p 进赋值判断答案是否为零; 不调用 Lucas 数位组合
struct PadicOracle
{
    int p;
    VLL unit;
    explicit PadicOracle(int mod) : p(mod), unit(p, 1)
    {
        for (int i = 1; i < p; i++) unit[i] = mul_ref(unit[i - 1], i, p);
    }
    LL factorial_unit(LL n) const
    {
        LL r = 1;
        while (n)
        {
            if ((n / p) & 1) r = mul_ref(r, p - 1, p);
            r = mul_ref(r, unit[n % p], p);
            n /= p;
        }
        return r;
    }
    LL comb(LL n, LL k) const
    {
        if (k < 0 || k > n) return 0;
        if (valuation(n, p) - valuation(k, p) - valuation(n - k, p)) return 0;
        LL den = mul_ref(factorial_unit(k), factorial_unit(n - k), p);
        return mul_ref(factorial_unit(n), inverse_ref(den, p), p);
    }
};

static void small_exact()
{
    const VLL mods{1, 2, 3, 4, 5, 7, 12, 97, 101, 998244353, 1000000007,
                   2147483647, 2147483648LL, 2305843009213693951LL, LLONG_MAX};
    const VI small_primes = primes_to(121);
    for (LL mod : mods)
    {
        PascalComb pascal;
        pascal.init(120, mod);
        bool prime = find(small_primes.begin(), small_primes.end(), mod) != small_primes.end()
                  || mod == 998244353 || mod == 1000000007 || mod == 2147483647
                  || mod == 2305843009213693951LL;
        PrimeComb fact;
        Lucas lucas;
        if (prime) fact.init((int)min<LL>(120, mod - 1), mod);
        if (prime && mod <= 121) lucas.init((int)mod);
        for (int n = 0; n <= 120; n++)
        {
            u128 exact = 1;
            for (int k = 0; k <= n; k++)
            {
                if (k) exact = exact * (n - k + 1) / k;
                LL want = exact % (u128)mod;
                eq(pascal.comb(n, k), want);
                if (prime && min(k, n - k) < mod) eq(binom_loop(n, k, mod), want);
                if (prime && n < mod) eq(fact.comb(n, k), want);
                if (prime && mod <= 121) eq(lucas.comb(n, k), want);
            }
            eq(pascal.comb(n, -1), 0);
            eq(pascal.comb(n, n + 1), 0);
            if (prime)
            {
                eq(binom_loop(n, -1, mod), 0);
                eq(binom_loop(n, n + 1, mod), 0);
                if (n < mod) { eq(fact.comb(n, -1), 0); eq(fact.comb(n, n + 1), 0); }
            }
        }
    }
}
static void loops()
{
    for (LL p : {2LL, 3LL, 97LL, 1000000007LL, 2305843009213693951LL})
    {
        for (int rep = 0; rep < 160; rep++)
        {
            LL n = rep < 80 ? LLONG_MAX - rep : (LL)(rng() & LLONG_MAX);
            int k = (int)(rng() % min<LL>(p, 81));
            LL want = choose_cancel(n, k, p);
            eq(binom_loop(n, k, p), want);
            eq(binom_loop(n, n - k, p), want);
        }
        eq(binom_loop(p, 1, p), 0);
        eq(binom_loop(p + 1, 1, p), 1);
    }
    VI primes = primes_to(400000);
    for (LL p : {1000000007LL, 2305843009213693951LL})
        eq(binom_loop(400000, 200000, p), choose_factors(400000, 200000, p, primes));
}
static void pascal_rebuild()
{
    PascalComb c, other;
    other.init(100, 12);
    const VI primes = primes_to(2000);
    for (LL mod : {1LL, 2LL, 12LL, 2147483647LL, 2147483648LL, LLONG_MAX})
        for (int limit : {2000, 0, 1, 257, 2000})
        {
            c.init(limit, mod);
            eq(c.mod, mod);
            eq(c.c.size(), limit + 1);
            LL sum_expected = 1 % mod;
            for (int n = 0; n <= limit; n++)
            {
                eq(c.c[n].size(), n + 1);
                LL sum = 0;
                for (int k = 0; k <= n; k++)
                {
                    LL v = c.comb(n, k);
                    assert(v >= 0 && v < mod);
                    sum = ((u128)sum + v) % mod;
                    eq(v, c.comb(n, n - k));
                }
                eq(sum, sum_expected);
                sum_expected = ((u128)sum_expected * 2) % mod;
                for (int k = 0; k <= min(n, 2); k++)
                    eq(c.comb(n, k), choose_cancel(n, k, mod));
            }
            for (int rep = 0; rep < 40; rep++)
            {
                int n = (int)(rng() % (limit + 1)), k = (int)(rng() % (n + 1));
                eq(c.comb(n, k), choose_factors(n, k, mod, primes));
            }
            eq(other.comb(100, 50), choose_factors(100, 50, 12, primes));
        }
}
static void factorial_rebuild()
{
    PrimeComb c, other;
    other.init(4, 5);
    VI primes = primes_to(5000000);
    for (LL mod : {1000000007LL, 2305843009213693951LL})
        for (int limit : {200000, 0, 1, 257, 200000})
        {
            c.init(limit, mod);
            eq(c.mod, mod);
            eq(c.fact.size(), limit + 1);
            eq(c.inv_fact.size(), limit + 1);
            LL fact = 1;
            for (int n = 0; n <= limit; n++)
            {
                if (n) fact = mul_ref(fact, n, mod);
                eq(c.fact[n], fact);
                eq(mul_ref(c.fact[n], c.inv_fact[n], mod), 1);
                eq(c.comb(n, 0), 1);
                eq(c.comb(n, n), 1);
                if (n) eq(c.comb(n, 1), n);
            }
            for (int rep = 0; rep < 6; rep++)
            {
                int n = (int)(rng() % (limit + 1)), k = (int)(rng() % (n + 1));
                eq(c.comb(n, k), choose_factors(n, k, mod, primes));
            }
            eq(other.comb(4, 2), 1);
        }
    c.init(5000000, 1000000007);
    LL fact = 1;
    for (int n = 0; n <= 5000000; n++)
    {
        if (n) fact = fact * n % c.mod; // 小模数独立 64 位直乘, 不复用模板 i128 运算
        eq(c.fact[n], fact);
        eq(c.fact[n] * c.inv_fact[n] % c.mod, 1);
    }
    eq(c.comb(5000000, 2500000), choose_factors(5000000, 2500000, c.mod, primes));
    c.init(0, 2);
    eq(c.comb(0, 0), 1);
}
static void lucas_rebuild()
{
    Lucas c, other;
    other.init(7);
    const VI checked_primes = primes_to(200003);
    for (int p : {200003, 2, 3, 5, 97, 257, 100003, 2, 200003})
    {
        assert(binary_search(checked_primes.begin(), checked_primes.end(), p));
        c.init(p);
        PadicOracle ref(p);
        for (int n = 0; n < p; n++)
        {
            eq(c.table.fact[n], ref.unit[n]);
            eq(mul_ref(c.table.fact[n], c.table.inv_fact[n], p), 1);
        }
        VLL ns{0, 1, p - 1, p, p + 1, (LL)p * p - 1, (LL)p * p,
               (LL)p * p + 1, LLONG_MAX - 1, LLONG_MAX};
        for (LL n : ns)
        {
            for (LL k : {0LL, 1LL, 2LL, n / 2, n - 1, n})
                eq(c.comb(n, k), ref.comb(n, k));
            eq(c.comb(n, -1), 0);
            if (n != LLONG_MAX) eq(c.comb(n, n + 1), 0);
        }
        for (int rep = 0; rep < 2000; rep++)
        {
            LL n = (LL)(rng() & LLONG_MAX);
            LL k = (LL)(rng() % ((ULL)n + 1));
            eq(c.comb(n, k), ref.comb(n, k));
            // 随机 k 容易只产生零, 再构造每个 p 进位均不超过 n 的非零答案
            LL rest = n, place = 1;
            k = 0;
            while (rest)
            {
                k += (LL)(rng() % (rest % p + 1)) * place;
                rest /= p;
                if (rest) place *= p;
            }
            eq(c.comb(n, k), ref.comb(n, k));
            eq(c.comb(n, n - k), ref.comb(n, k));
        }
        eq(other.comb(100, 50), PadicOracle(7).comb(100, 50));
    }
    c.init(2);
    for (int rep = 0; rep < 200000; rep++)
    {
        LL n = (LL)(rng() & LLONG_MAX);
        LL k = rep & 1 ? (LL)(rng() & n) : (LL)(rng() & LLONG_MAX);
        eq(c.comb(n, k), (k & ~n) == 0);
    }
}
int main()
{
    small_exact();
    loops();
    pascal_rebuild();
    factorial_rebuild();
    lucas_rebuild();
    cout << "comb_check passed: " << checks << " comparisons, seed 20260908\n";
}
