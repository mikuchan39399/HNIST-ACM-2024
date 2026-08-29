#include <cassert>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>
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
    auto rll = [&]() { return ((LL)rng() << 32) | (LL)rng(); }; // 64 位随机
    for (int t = 0; t < 40; t++)
    {
        LL a = (LL)(rng() % (unsigned long long)M);
        LL b = (LL)(rng() % (unsigned long long)M);
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
        i128 big = ((i128)rll() << 64) | (unsigned long long)rll();
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
    cout << "All tests passed flawlessly!\n";
    return 0;
}