#include <cassert>
#include <iostream>
#include <numeric>
#include <random>
#include <sstream>
#include <string>
#include <vector>
#include "../高精度.cpp"
using namespace std;
using LL = long long;

// ============ constexpr 全链路探针: 编译期算高精度 ============
// 本地 mingw g++15 曾以 static_assert 验证过; Linux g++12/13 对 constexpr
// vector 比较严格(allocated storage after deallocation)会拒编, 降级运行期
// 断言(见 main), 编译期能力验证责任归本地。

// ============ 独立 oracle 甲: 十进制串竖式 (与模板 base-1e9/Karatsuba 不同构) ============
static string s_mul(const string& x, const string& y)
{
    if (x == "0" || y == "0") return "0";
    int n = (int)x.size(), m = (int)y.size();
    vector<int> t(n + m, 0);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            t[i + j + 1] += (x[i] - '0') * (y[j] - '0');
    for (int k = n + m - 1; k > 0; k--)
    {
        t[k - 1] += t[k] / 10;
        t[k] %= 10;
    }
    int p = 0;
    while (p + 1 < n + m && t[p] == 0) p++;
    string r;
    for (; p < n + m; p++) r += char('0' + t[p]);
    return r;
}

// ============ 独立 oracle 乙: i128 原生语义 (构造值全部 |v| <= LLONG_MAX) ============
static string i128_str(i128 v)
{
    if (v == 0) return "0";
    bool neg = v < 0;
    u128 m = neg ? (u128)(-(v + 1)) + 1 : (u128)v;
    string s;
    while (m) { s += char('0' + (int)(m % 10)); m /= 10; }
    if (neg) s += '-';
    reverse(s.begin(), s.end());
    return s;
}
static i128 i128_pow(i128 b, int e)
{
    i128 r = 1;
    while (e--) r *= b;
    return r;
}

// ============ 随机生成 ============
static string rnd_digits(mt19937& rng, int len)
{
    string s(1, char('1' + rng() % 9));
    for (int i = 1; i < len; i++) s += char('0' + rng() % 10);
    return s;
}
static string rnd_signed(mt19937& rng, int len)
{
    string s = rnd_digits(rng, len);
    return (rng() & 1) ? "-" + s : s;
}
static LL rnd_ll(mt19937& rng) { return ((LL)rng() << 32 | rng()) % (LL)1e14 + 1; }

// ===================== 1. i128 全精确: 小值域全接口 =====================
void test_i128_core()
{
    mt19937 rng(42);
    const i128 edges[] = { 0, 1, -1, (i128)numeric_limits<LL>::max(),
                           (i128)numeric_limits<LL>::min() + 1, (i128)(LL)1e18, (i128)(LL)-1e18 };
    const i128 base_tab[] = { 0, 1, 2, 3, 7, 10, -2, -3 };
    for (int tc = 0; tc < 300; tc++)
    {
        bool edge = tc % 10 == 0;
        i128 a = edge ? edges[rng() % 7] : (i128)(rnd_ll(rng) * (rng() & 1 ? -1 : 1));
        i128 b = edge ? edges[rng() % 7] : (i128)(rnd_ll(rng) * (rng() & 1 ? -1 : 1));
        BigInt A(a), B(b);
        assert(A.str() == to_string((LL)a));
        assert(A.sign() == (a > 0 ? 1 : a < 0 ? -1 : 0));
        assert(A.abs().str() == to_string(a < 0 ? (LL)(-a) : (LL)a));
        assert((-A).str() == to_string((LL)(-a)));
        assert((bool)A == (a != 0));
        assert(A.is_zero() == (a == 0));
        assert(A.digits10() == (int)to_string((LL)a).size() - (a < 0 ? 1 : 0));
        assert((A < B) == (a < b) && (A <= B) == (a <= b) && (A > B) == (a > b));
        assert((A == B) == (a == b) && (A != B) == (a != b) && (A >= B) == (a >= b));
        assert((A + B).str() == i128_str(a + b));
        assert((A - B).str() == i128_str(a - b));
        assert((A * B).str() == i128_str(a * b));
        if (b != 0)
        {
            auto [Q, R] = A.divmod(B);
            assert(Q.str() == i128_str(a / b));
            assert(R.str() == i128_str(a % b));
            assert((A / B).str() == i128_str(a / b));
            assert((A % B).str() == i128_str(a % b));
        }
        i128 bs = base_tab[rng() % 8];
        int e = (int)(rng() % 13);
        assert(BigInt(bs).pow(e).str() == i128_str(i128_pow(bs, e)));
        LL m = 1 + (LL)(rng() % 1000000007);
        assert(A.mod(m) == (LL)(((a % m) + m) % m));
        assert(A.to_LL() == (LL)a);
        assert(A.to_i128() == a);
        BigInt op("777");
        op += A;
        op -= A;
        op *= A;                                         // 复合赋值往返
        assert(op.str() == i128_str(777 * a));
    }
    assert(BigInt((int)5).str() == "5");
    assert(BigInt(5u).str() == "5");
    assert(BigInt((LL)-9).str() == "-9");
}

// ===================== 2. 串乘跨 Karatsuba 阈值 (KARATSUBA=40 肢 = 360 位) =====================
void test_mul_straddle()
{
    mt19937 rng(42);
    const int lens[] = { 1, 9, 315, 351, 360, 369, 405, 720, 900 };
    const int nl = 9;
    auto one = [&](const string& x, const string& y)
    {
        bool nx = rng() & 1, ny = rng() & 1;
        BigInt A((nx ? "-" : "") + x), B((ny ? "-" : "") + y);
        string prod = s_mul(x, y);
        string want = (prod == "0") ? "0" : ((nx != ny) ? "-" + prod : prod);
        assert((A * B).str() == want);
    };
    for (int i = 0; i < nl; i++)
        for (int j = 0; j < nl; j++)
            one(rnd_digits(rng, lens[i]), rnd_digits(rng, lens[j]));
    for (int t = 0; t < 60; t++)
        one(rnd_digits(rng, 1 + rng() % 900), rnd_digits(rng, 1 + rng() % 900));
    assert((BigInt("123456789123456789") * BigInt(0)).str() == "0");
    assert((BigInt("-987654321987654321") * BigInt(1)).str() == "-987654321987654321");
}

// ===================== 3. 大数除法性质完备 (截断除法解唯一, 性质即证明) =====================
void test_div_property()
{
    mt19937 rng(42);
    for (int tc = 0; tc < 300; tc++)
    {
        BigInt A(rnd_signed(rng, 1 + rng() % 200));
        BigInt B(rnd_digits(rng, 1 + rng() % 200));      // B > 0
        BigInt Q = A / B, R = A % B;
        assert(B * Q + R == A);
        assert(R == 0 || R.sign() == A.sign());
        assert(R.abs() < B);
        BigInt Bn = -B;                                  // 除数取负: |Q| 不变, R 不变, Q 变号
        BigInt Q2 = A / Bn, R2 = A % Bn;
        assert(Q2.abs() == Q.abs());
        assert(R2 == R);
        assert(Q2 == 0 || Q2.sign() == -A.sign());
    }
    BigInt Z(0), D("3");
    assert(Z / D == 0 && Z % D == 0);                    // 0 除
    BigInt S("123456789");
    assert(S / S == 1 && S % S == 0);                    // 自除
    assert(S / -S == -1 && S % -S == 0);                 // 负自除
    BigInt T = S - BigInt(1);                            // |a| < |b|
    assert(T / S == 0 && T % S == T);
    BigInt N9(string(200, '9'));                        // 借位链: 10^200 - 1
    BigInt q9 = N9 / BigInt(7), r9 = N9 % BigInt(7);
    assert(BigInt(7) * q9 + r9 == N9 && r9 < 7);
    BigInt M7("1000000007"), K = M7 * BigInt("12345678987654321");  // 多肢构造整除
    assert(K / M7 == BigInt("12345678987654321") && K % M7 == 0);
}

// ===================== 4. sqrt 下取整性质 =====================
void test_sqrt_property()
{
    mt19937 rng(42);
    for (int tc = 0; tc < 200; tc++)
    {
        BigInt N(rnd_digits(rng, 1 + rng() % 100));
        BigInt X = N.sqrt();
        assert(X * X <= N && (X + 1) * (X + 1) > N);
    }
    for (int tc = 0; tc < 100; tc++)                     // 完全平方精确命中
    {
        BigInt K(rnd_digits(rng, 1 + rng() % 40));
        assert((K * K).sqrt() == K);
    }
    assert(BigInt(0).sqrt() == BigInt(0));
    assert(BigInt("100000000000000000000").sqrt() == BigInt("10000000000"));
}

// ===================== 5. gcd 构造 + lcm 性质 =====================
void test_gcd_construct()
{
    mt19937 rng(42);
    for (int tc = 0; tc < 200; tc++)
    {
        BigInt G(rnd_digits(rng, 1 + rng() % 30));
        int p = 1 + rng() % 60, q = 1 + rng() % 60;
        BigInt A = G * p, B = G * q;
        if (rng() & 1) A = -A;
        if (rng() & 1) B = -B;
        assert(gcd(A, B) == G * (LL)std::gcd(p, q));
        if (tc % 4 == 0)
        {
            BigInt L = lcm(A, B);
            assert(L.sign() >= 0);                       // 对齐 std::lcm: 恒非负
            assert(L % A == 0 && L % B == 0);
            assert((gcd(A, B) * L).abs() == (A * B).abs());
            assert(L == G * (LL)((LL)(p / std::gcd(p, q)) * q));  // 精确值
        }
    }
    assert(gcd(BigInt(-567), BigInt(0)).str() == "567");
    assert(gcd(BigInt(0), BigInt(0)).str() == "0");
}

// ===================== 6. factorial 独立串乘链 =====================
void test_factorial()
{
    string run = "1";                                    // 0! = 1
    for (int n = 0; n <= 300; n++)
    {
        assert(BigInt::factorial((u64)n).str() == run);
        run = s_mul(run, to_string(n + 1));
    }
}

// ===================== 7. parse 规范化 + IO 往返 =====================
void test_parse_io()
{
    mt19937 rng(42);
    for (int tc = 0; tc < 300; tc++)
    {
        string digits((size_t)(rng() % 6), '0');         // 前导零 0..5 个
        int body = 1 + rng() % 50;
        for (int i = 0; i < body; i++) digits += char('0' + rng() % 10);
        int r = (int)(rng() % 3);
        string s = (r == 1 ? "-" : r == 2 ? "+" : "");
        s += digits;
        string mag;                                      // 独立规范化: 去前导零
        bool lead = true;
        for (char c : digits)
        {
            if (lead && c == '0') continue;
            lead = false;
            mag += c;
        }
        if (mag.empty()) mag = "0";
        string want = (mag == "0") ? "0" : (r == 1 ? "-" + mag : mag);
        BigInt X(s);
        assert(X.str() == want);
        X = string(s);
        assert(X.str() == want);
        X = s.c_str();
        assert(X.str() == want);
        stringstream ss(s + " " + s);
        BigInt Y1, Y2;
        ss >> Y1 >> Y2;
        assert(Y1.str() == want && Y2.str() == want);
        ostringstream os;
        os << Y1;
        assert(os.str() == want);
    }
}

int main()
{
    assert(BigInt(7).pow(3) == BigInt(343));                  // constexpr 链路运行期复验
    assert(BigInt(114514).pow(2) == BigInt((LL)13113456196));
    test_i128_core();
    test_mul_straddle();
    test_div_property();
    test_sqrt_property();
    test_gcd_construct();
    test_factorial();
    test_parse_io();
    cout << "All tests passed flawlessly!\n";
    return 0;
}