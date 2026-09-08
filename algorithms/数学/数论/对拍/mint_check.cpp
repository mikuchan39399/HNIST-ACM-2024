#include <cassert>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>
#include "../mint.cpp"
#include "../mint.cpp"
using namespace std;
using LL = long long;

// ===================== 独立暴力底座 (与模板零共享) =====================
// 模归化: 负数也归到 [0, m)
static LL norm_val(LL v, LL m) { return ((v % m) + m) % m; }
// 模乘: 俄罗斯农民倍加, 与模板的直乘/i128 路由不同构
static LL b_mul(LL a, LL b, LL m)
{
    LL r = 0;
    a %= m;
    while (b)
    {
        if (b & 1) r = (r + a) % m;
        a = (a + a) % m;
        b >>= 1;
    }
    return r;
}
// 小指数幂: 逐次乘 (大指数锚定用)
static LL b_pow(LL a, LL e, LL m)
{
    LL r = 1 % m;
    while (e--) r = b_mul(r, a, m);
    return r;
}
// 辗转相除
static LL b_gcd(LL a, LL b) { while (b) { LL t = a % b; a = b; b = t; } return a; }
// 素性: 试除到 sqrt
static bool b_prime(LL n)
{
    if (n < 2) return false;
    for (LL i = 2; i * i <= n; i++)
        if (n % i == 0) return false;
    return true;
}
// 十进制大串取模: 逐位 Horner
static LL b_parse(const string& s, LL m)
{
    LL r = 0;
    bool neg = (s[0] == '-');
    for (char c : s)
        if (c >= '0' && c <= '9') r = (b_mul(r, 10, m) + (c - '0')) % m;
    return neg ? norm_val(-r, m) : r;
}
// 组合数: 杨辉三角滚动行, 与模板两条路由都不同构
static LL b_comb(int n, int k, LL m)
{
    if (k < 0 || k > n) return 0;
    vector<LL> C(k + 1);
    C[0] = 1 % m;
    for (int i = 1; i <= n; i++)
        for (int j = min(i, k); j >= 1; j--)
            C[j] = (C[j] + C[j - 1]) % m;
    return C[k];
}

// ===================== 单模数全套 =====================
template <LL M>
void run_mod(mt19937& rng)
{
    using mint = ModLL<M>;
    static bool inited = false;
    if (!inited)
    {
        if (M < (LL)4e12) assert(mint::PRIME == b_prime(M));
        else assert(M % 3 == 0 && !mint::PRIME); // 2^62-1 含因子 3, 构造性合数
        if constexpr (mint::PRIME) mint::init_fact((int)min(210LL, M - 1)); // 质数: n! 须 < M 才可逆
        else mint::init_fact(210);                                          // 合数: spf 全域有效
        inited = true;
    }
    auto rll = [&]() { return (LL)(((unsigned long long)rng() << 32) | rng()); }; // 64 位随机
    for (int t = 0; t < 40; t++)
    {
        LL a = t == 0 ? M - 1 : (LL)((unsigned long long)rll() % (unsigned long long)M);
        LL b = t == 0 ? M - 1 : (LL)((unsigned long long)rll() % (unsigned long long)M);
        mint ma(a), mb(b);
        assert(ma.val() == a && mb.val() == b);
        // 四则与负号
        assert((ma + mb).val() == (a + b) % M);
        assert((ma - mb).val() == norm_val(a - b, M));
        assert((ma * mb).val() == b_mul(a, b, M));
        assert((-ma).val() == norm_val(-a, M));
        // 混合隐式转换与比较
        assert((ma + 3).val() == (a + 3) % M);
        assert((5 - ma).val() == norm_val(5 - a, M));
        assert((ma * 7).val() == b_mul(a, 7, M));
        mint probe(3);
        assert(probe == 3 && probe != 4);
        // 大原值构造: 负 LL / i128 / u64 无符号分支
        LL raw = (rll() >> 2) - (1LL << 60);
        mint mraw(raw);
        assert(mraw.val() == norm_val(raw, M));
        i128 big = (i128)((((unsigned __int128)(unsigned long long)rll() << 64) | (unsigned long long)rll()) >> 1);
        if (rng() & 1) big = -big;
        mint mbig(big);
        i128 vb = big % M;
        if (vb < 0) vb += M;
        assert(mbig.val() == (LL)vb);
        unsigned long long u = (unsigned long long)rll();
        mint mu(u);
        assert(mu.val() == (LL)(u % M));
        // 幂: 小指数逐乘锚定 + 大指数分裂一致性 + 负指数
        LL e = rng() % 41;
        assert(ma.pow(e).val() == b_pow(a, e, M));
        assert(ma.pow(0).val() == 1 % M && ma.pow(1).val() == a);
        i128 E = ((i128)((unsigned long long)rll() >> 1) << 64) | (unsigned long long)rll(); // 逻辑右移保非负
        assert(ma.pow(E) == ma.pow(E >> 1) * ma.pow(E - (E >> 1)));
        if (a != 0 && b_gcd(a, M) == 1)
        {
            assert(ma.pow(-e) == ma.pow(e).inv());
            assert(ma.pow(-e) * ma.pow(e) == mint(1));
        }
        // 逆元与除法: 解唯一, 性质即完全证明
        if (a != 0 && b_gcd(a, M) == 1)
        {
            assert((ma * ma.inv()).val() == 1 % M);
            if (b != 0 && b_gcd(b, M) == 1)
            {
                assert((ma / mb) == ma * mb.inv());
                assert((ma / mb) * mb == ma);
            }
        }
    }
    // 组合数: 全 k 扫描 + 锚点 + 偶发大 n (质数模数约束 n < M)
    if (!mint::PRIME || 10 < M) assert(mint::comb(10, 3).val() == 120 % M);
    int lim = mint::PRIME ? (int)min(61LL, M - 1) : 61;
    int n = (int)(rng() % lim);
    for (int k = -1; k <= n + 1; k++)
        assert(mint::comb(n, k).val() == b_comb(n, k, M));
    int hi = mint::PRIME ? (int)min(210LL, M - 1) : 210;
    if (hi >= 61 && rng() % 10 == 0)
    {
        int N = 61 + (int)(rng() % (hi - 60));
        int K = (int)(rng() % (N + 1));
        assert(mint::comb(N, K).val() == b_comb(N, K, M));
    }
    // 质数模数: fact / inv_fact 表 (n! < M 约束同上)
    if constexpr (mint::PRIME)
    {
        int fi = (int)min(200LL, M - 1);
        LL run = 1 % M;
        for (int i = 0; i <= fi; i++)
        {
            assert(mint::fact[i].val() == run);
            assert((mint::fact[i] * mint::inv_fact[i]).val() == 1 % M);
            run = b_mul(run, i + 1, M);
        }
    }
    // 任意位数大数 IO 往返 (含负号)
    int len = 1 + (int)(rng() % 60);
    string s(1, char('1' + rng() % 9));
    for (int i = 1; i < len; i++) s += char('0' + rng() % 10);
    if (rng() % 5 == 0) s = "-" + s;
    LL want = b_parse(s, M);
    mint mio;
    istringstream iss(s);
    iss >> mio;
    assert(mio.val() == want);
    ostringstream oss;
    oss << mio;
    assert(oss.str() == to_string(want));
}


// ============ 最终覆盖: 类型极值, 独立幂参照, 静态表复位与目标规模 ============
static LL unsigned_mod(u128 value, LL m)
{
    LL result = 0;
    for (int bit = 127; bit >= 0; bit--)
        result = (result + result + LL((value >> bit) & 1)) % m;
    return result;
}
static LL signed_mod(i128 value, LL m)
{
    u128 magnitude = value;
    if (value < 0) magnitude = -magnitude;
    LL r = unsigned_mod(magnitude, m);
    return value < 0 && r ? m - r : r;
}
// 从最高位开始扫描, 模乘用倍加, 不复用模板的右移幂与直乘
static LL big_pow(LL base, u128 exponent, LL m)
{
    LL result = 1;
    for (int bit = 127; bit >= 0; bit--)
    {
        result = b_mul(result, result, m);
        if ((exponent >> bit) & 1) result = b_mul(result, base, m);
    }
    return result;
}

// 埃氏筛与 Legendre 阶乘赋值, 不使用模板的 SPF 或逐个分解分子/分母
static VI oracle_primes(int n)
{
    VI primes;
    vector<bool> composite(n + 1);
    for (int p = 2; p <= n; p++) if (!composite[p])
    {
        primes.push_back(p);
        for (LL j = LL(p) * p; j <= n; j += p) composite[j] = true;
    }
    return primes;
}
static LL large_comb(int n, int k, LL m, const VI& primes)
{
    if (k < 0 || k > n) return 0;
    LL result = 1;
    for (int p : primes)
    {
        if (p > n) break;
        int exponent = 0;
        for (LL power = p; power <= n; power *= p)
            exponent += n / power - k / power - (n - k) / power;
        while (exponent--) result = b_mul(result, p, m);
    }
    return result;
}

template <LL M>
static void type_boundaries()
{
    using T = ModLL<M>;
    for (i128 x : {numeric_limits<i128>::min(), numeric_limits<i128>::min() + 1,
                    i128(LLONG_MIN), i128(INT_MIN), i128(-M), i128(-1), i128(0), i128(1),
                    i128(M), i128(INT_MAX), i128(LLONG_MAX), numeric_limits<i128>::max()})
        assert(T(x).val() == signed_mod(x, M));
    for (ULL x : {0ULL, 1ULL, ULL(M - 1), ULL(M), ULL(LLONG_MAX), ULLONG_MAX})
        assert(T(x).val() == unsigned_mod(x, M));
    for (int v = -128; v <= 127; v++) assert(T(static_cast<signed char>(v)).val() == norm_val(v, M));
    for (int v = 0; v <= 255; v++) assert(T(static_cast<unsigned char>(v)).val() == v % M);
    assert(T(numeric_limits<short>::min()).val() == norm_val(numeric_limits<short>::min(), M));
    assert(T(numeric_limits<short>::max()).val() == norm_val(numeric_limits<short>::max(), M));
    assert(T(numeric_limits<unsigned short>::max()).val() == numeric_limits<unsigned short>::max() % M);
    assert(T(INT_MIN).val() == norm_val(INT_MIN, M) && T(INT_MAX).val() == INT_MAX % M);
    assert(T(UINT_MAX).val() == unsigned_mod(UINT_MAX, M));
    assert(T(LONG_MIN).val() == signed_mod(LONG_MIN, M) && T(ULONG_MAX).val() == unsigned_mod(ULONG_MAX, M));
    assert(T(LLONG_MIN).val() == signed_mod(LLONG_MIN, M));
    assert(T(false).val() == 0 && T(true).val() == 1);
    auto character_bounds = []<class C>()
    {
        assert(T(numeric_limits<C>::min()).val() == signed_mod(i128(numeric_limits<C>::min()), M));
        assert(T(numeric_limits<C>::max()).val() == signed_mod(i128(numeric_limits<C>::max()), M));
    };
    character_bounds.template operator()<char>();
    character_bounds.template operator()<wchar_t>();
    character_bounds.template operator()<char8_t>();
    character_bounds.template operator()<char16_t>();
    character_bounds.template operator()<char32_t>();
    static_assert(sizeof(T) == sizeof(LL));
    static_assert(!is_constructible_v<T, double> && !is_constructible_v<T, u128>);
    static_assert(T(6) / T(1) == T(6));
    static_assert(T(1).pow(numeric_limits<i128>::min()).val() == 1);
    constexpr T division = [] { T a(7); a /= 1; return a; }();
    static_assert(division == T(7));
}

template <LL M>
static void deep_mod()
{
    using T = ModLL<M>;
    mt19937_64 rng(42);
    type_boundaries<M>();
    for (int tc = 0; tc < 350; tc++)
    {
        LL a = tc < 3 ? M - 1 - (tc % min(3LL, M)) : LL(rng() % M);
        LL b = LL(rng() % M);
        T x(a), y(b);
        assert((x += y).val() == (a + b) % M);
        x = a; assert((x -= y).val() == norm_val(a - b, M));
        x = a; assert((x *= y).val() == b_mul(a, b, M));
        x = a; x += x; assert(x.val() == (a + a) % M);
        x = a; x -= x; assert(x.val() == 0);
        x = a; x *= x; assert(x.val() == b_mul(a, a, M));
        assert((ULLONG_MAX + T(a)).val() == (unsigned_mod(ULLONG_MAX, M) + a) % M);
        i128 raw = i128((u128(rng()) << 64 | rng()) >> 1);
        if (tc & 1) raw = -raw;
        assert(T(raw).val() == signed_mod(raw, M));
        u128 e = u128(rng()) << 64 | rng();
        e >>= 1;
        assert(T(a).pow(i128(e)).val() == big_pow(a, e, M));
        if (b_gcd(a, M) == 1)
        {
            LL inverse = T(a).inv().val();
            assert(b_mul(a, inverse, M) == 1);
            assert(T(a).pow(-i128(e)).val() == big_pow(inverse, e, M));
            if (tc < 12)
                assert(T(a).pow(numeric_limits<i128>::min()).val() == big_pow(inverse, u128(1) << 127, M));
            x = a; x /= x; assert(x.val() == 1);
            x = b; x /= T(a); assert(x.val() == b_mul(b, inverse, M));
        }
        // 左右操作数隐式构造, 除数仅在可逆域测试
        assert((LLONG_MIN - T(a)).val() == norm_val(signed_mod(LLONG_MIN, M) - a, M));
        assert((T(a) * ULLONG_MAX).val() == b_mul(a, unsigned_mod(ULLONG_MAX, M), M));
        assert((T(a) == a) && (a == T(a)));
        assert((T(a) != a + 1) && (a + 1 != T(a)));
    }
    int limit = T::PRIME ? int(min(180LL, M - 1)) : 180;
    for (int n : {limit, 0, 1, limit / 2, limit})
    {
        T::init_fact(n);
        VLL row(n + 1); row[0] = 1;
        for (int i = 0; i <= n; i++)
        {
            if (i) for (int j = i; j >= 1; j--) row[j] = (row[j] + row[j - 1]) % M;
            for (int k = -1; k <= i + 1; k++)
                assert(T::comb(i, k).val() == (k < 0 || k > i ? 0 : row[k]));
        }
        if constexpr (T::PRIME)
        {
            assert(int(T::fact.size()) == n + 1 && int(T::inv_fact.size()) == n + 1);
            LL fact = 1;
            for (int i = 0; i <= n; i++)
            {
                if (i) fact = b_mul(fact, i, M);
                assert(T::fact[i].val() == fact && b_mul(fact, T::inv_fact[i].val(), M) == 1);
            }
        }
        else
        {
            assert(int(T::spf.size()) == n + 1);
            for (int i = 2; i <= n; i++)
            {
                int p = 2; while (i % p) p++;
                assert(T::spf[i] == p);
            }
        }
    }
    for (string text : {"0", "-0", "+0000", "-00012345678901234567890", "+998244353", "999999999999999999999999999999999999999999"})
    {
        T value(17); istringstream stream(" \t\n" + text); stream >> value;
        assert(value.val() == b_parse(text, M));
        ostringstream out; out << value; assert(out.str() == to_string(value.val()));
        T previous = value; stream >> value; assert(stream.fail() && value == previous);
    }
    istringstream sequence("+12 -34 00056"); T a, b, c; sequence >> a >> b >> c;
    assert(a == 12 && b == -34 && c == 56);
}

template <LL M>
static void throughput()
{
    using T = ModLL<M>;
    mt19937_64 rng(42);
    constexpr int N = 200000;
    vector<T> values(N + 1);
    VLL expected(N + 1);
    for (int i = 1; i <= N; i++)
    {
        LL a = rng() % M, b = rng() % M;
        values[i] = (values[i - 1] + a) * b - i;
        expected[i] = norm_val(b_mul((expected[i - 1] + a) % M, b, M) - i % M, M);
        assert(values[i].val() == expected[i]);
    }
    string token;
    token.reserve(N + 1); token += '-';
    for (int i = 0; i < N; i++) token += char('0' + rng() % 10);
    istringstream in(token); T value; in >> value;
    assert(value.val() == b_parse(token, M));
}

template <LL M>
static void table_scale()
{
    using T = ModLL<M>;
    constexpr int N = 200000;
    auto primes = oracle_primes(N);
    for (int n : {N, 0, 1, 257, N})
    {
        T::init_fact(n);
        for (int k : {0, 1, 2, n / 2, n / 3, n - 1, n, n + 1})
            assert(T::comb(n, k).val() == large_comb(n, k, M, primes));
        if constexpr (T::PRIME)
        {
            LL fact = 1;
            for (int i = 0; i <= n; i++)
            {
                if (i) fact = b_mul(fact, i, M);
                assert(T::fact[i].val() == fact && b_mul(fact, T::inv_fact[i].val(), M) == 1);
                int k = i % 4;
                // 小 k 用精确整数式, i<=20万使 C(i,3) 在 LL 内
                LL choose = k > i ? 0 : k == 0 ? 1 : k == 1 ? i : k == 2 ? LL(i) * (i - 1) / 2 : LL(i) * (i - 1) * (i - 2) / 6;
                assert(T::comb(i, k).val() == choose % M);
            }
        }
        else
        {
            VI reference(n + 1);
            for (int p : primes)
                for (int j = p; j <= n; j += p) if (!reference[j]) reference[j] = p;
            assert(T::spf == reference);
        }
    }
}

static void deep_suite()
{
    deep_mod<2>(); deep_mod<3>(); deep_mod<4>(); deep_mod<12>(); deep_mod<97>();
    deep_mod<998244353>(); deep_mod<1000000007>(); deep_mod<2147483648LL>();
    deep_mod<3037000499LL>(); deep_mod<3037000500LL>(); deep_mod<3037000501LL>();
    deep_mod<4294967291LL>(); deep_mod<2305843009213693951LL>();
    deep_mod<3825123056546413051LL>(); deep_mod<4611686018427387903LL>();
    static_assert(ModLL<2305843009213693951LL>::PRIME);
    static_assert(!ModLL<3825123056546413051LL>::PRIME);
    static_assert(!ModLL<4611686018427387903LL>::PRIME);
    // Lucas-Lehmer 独立检查 2^61-1 的素性, 不用 Miller-Rabin 作参照
    LL lucas = 4;
    for (int i = 0; i < 59; i++) lucas = norm_val(b_mul(lucas, lucas, 2305843009213693951LL) - 2, 2305843009213693951LL);
    assert(lucas == 0);
    assert(149491LL * 747451 * 34233211 == 3825123056546413051LL);
    for (LL n : {3037000499LL, 3037000500LL, 3037000501LL})
    {
        bool actual = n == 3037000499LL ? ModLL<3037000499LL>::PRIME : n == 3037000500LL ? ModLL<3037000500LL>::PRIME : ModLL<3037000501LL>::PRIME;
        assert(actual == b_prime(n));
    }
    throughput<998244353>(); throughput<2305843009213693951LL>(); throughput<4611686018427387903LL>();
    table_scale<998244353>(); table_scale<2305843009213693951LL>();
    table_scale<12>(); table_scale<4611686018427387903LL>();
    // 不同模数的静态表互不污染
    ModLL<998244353>::init_fact(11); ModLL<12>::init_fact(17);
    assert(ModLL<998244353>::comb(11, 5).val() == 462 && ModLL<12>::comb(17, 8).val() == b_comb(17, 8, 12));
    cout << "[PASS] mint deep: 15 moduli x 350 cases, full integer bounds, i128 powers, 200000 operations/digits/tables\n";
}

int main()
{
    mt19937 rng(42);
    using Runner = void (*)(mt19937&);
    Runner rs[] = { run_mod<2>, run_mod<3>, run_mod<5>, run_mod<1000000007LL>,
                    run_mod<998244353LL>, run_mod<4294967291LL>, run_mod<4>,
                    run_mod<6>, run_mod<12>, run_mod<100>, run_mod<2147483648LL>,
                    run_mod<4611686018427387903LL> };
    const int NM = sizeof(rs) / sizeof(rs[0]);
    for (int tc = 0; tc < 300; tc++)
        rs[tc % NM](rng);
    deep_suite();
    cout << "All tests passed flawlessly!\n";
    return 0;
}
