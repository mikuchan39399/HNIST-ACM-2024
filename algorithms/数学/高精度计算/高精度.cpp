// zoi: bigint
#ifndef Z_OI_BIGINT
#define Z_OI_BIGINT

#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <limits>
#include <cassert>
#include <algorithm>
#include <type_traits>
#include <compare>
#include "../../杂项/128位整数/128int.cpp"
#include "../../杂项/utils/utils.cpp"

using namespace std;


using u64 = unsigned long long;

template <class T>
concept Intable = (is_integral_v<T> && sizeof(T) <= 8) || is_same_v<T, i128>;

class BigInt
{
    static_assert(sizeof(unsigned) >= 4, "肢体需至少 32 位");
    using limb = unsigned;
    static constexpr limb B = 1000000000;
    static constexpr int BW = 9;
    vector<limb> a;
    bool neg = false;
    static constexpr void trim(vector<limb>& v)
    {
        while (!v.empty() && v.back() == 0) v.pop_back();
    }
    static constexpr int cmp_mag(const vector<limb>& x, const vector<limb>& y)
    {
        if (x.size() != y.size()) return x.size() < y.size() ? -1 : 1;
        for (size_t i = x.size(); i-- > 0; )
            if (x[i] != y[i]) return x[i] < y[i] ? -1 : 1;
        return 0;
    }
    static constexpr vector<limb> add_mag(const vector<limb>& x, const vector<limb>& y)
    {
        const vector<limb>& L = x.size() >= y.size() ? x : y;
        const vector<limb>& S = &L == &x ? y : x;
        vector<limb> r; r.reserve(L.size() + 1);
        limb c = 0;
        for (size_t i = 0; i < L.size(); i++)
        {
            limb t = L[i] + c;
            if (i < S.size()) t += S[i];
            r.push_back(t % B);
            c = t / B;
        }
        if (c) r.push_back(c);
        return r;
    }
    static constexpr vector<limb> sub_mag(const vector<limb>& x, const vector<limb>& y) // 需 x >= y
    {
        vector<limb> r; r.reserve(x.size());
        int bor = 0;
        for (size_t i = 0; i < x.size(); i++)
        {
            LL t = (LL)x[i] - bor;
            if (i < y.size()) t -= y[i];
            if (t < 0) { t += B; bor = 1; } else bor = 0;
            r.push_back((limb)t);
        }
        assert(bor == 0 && "sub_mag: 内部约束 x >= y 被破坏");
        trim(r);
        return r;
    }
    static constexpr vector<limb> mul_school(const vector<limb>& x, const vector<limb>& y)
    {
        if (x.empty() || y.empty()) return {};
        vector<limb> r(x.size() + y.size(), 0);
        for (size_t i = 0; i < x.size(); i++)
        {
            u64 c = 0;
            for (size_t j = 0; j < y.size(); j++)
            {
                u64 t = (u64)x[i] * y[j] + r[i + j] + c;    // < B^2 + 2B < 2^63
                r[i + j] = (limb)(t % B);
                c = t / B;
            }
            for (size_t k = i + y.size(); c; k++)
            {
                u64 t = r[k] + c;
                r[k] = (limb)(t % B);
                c = t / B;
            }
        }
        trim(r);
        return r;
    }
    static constexpr void add_at(vector<limb>& r, const vector<limb>& x, size_t off)
    {
        if (x.empty()) return;
        if (r.size() < off + x.size()) r.resize(off + x.size());
        limb c = 0;
        for (size_t i = 0; i < x.size(); i++)
        {
            limb t = r[off + i] + x[i]; t += c;
            r[off + i] = t % B;
            c = t / B;
        }
        for (size_t k = off + x.size(); c; k++)
        {
            if (k == r.size()) r.push_back(0);
            limb t = r[k] + c;
            r[k] = t % B;
            c = t / B;
        }
    }
    static constexpr vector<limb> mul_small(const vector<limb>& x, limb m)   // x * 单肢
    {
        if (x.empty() || m == 0) return {};
        vector<limb> r; r.reserve(x.size() + 1);
        u64 c = 0;
        for (limb t : x) { u64 v = (u64)t * m + c; r.push_back((limb)(v % B)); c = v / B; }
        if (c) r.push_back((limb)c);
        return r;
    }
    static constexpr pair<vector<limb>, limb> div_small(const vector<limb>& x, limb d)
    {
        assert(d >= 1);
        vector<limb> q(x.size());
        u64 r = 0;
        for (size_t i = x.size(); i-- > 0; )
        {
            u64 cur = r * B + x[i];
            q[i] = (limb)(cur / d);
            r = cur % d;
        }
        trim(q);
        return {move(q), (limb)r};
    }
    static constexpr size_t KARATSUBA = 40;
    static constexpr vector<limb> mul_mag(const vector<limb>& x, const vector<limb>& y)
    {
        if (x.empty() || y.empty()) return {};
        if (x.size() < KARATSUBA || y.size() < KARATSUBA) return mul_school(x, y);
        size_t h  = (max(x.size(), y.size()) + 1) / 2;
        size_t sx = min(h, x.size()), sy = min(h, y.size());
        vector<limb> x0(x.begin(), x.begin() + sx), x1(x.begin() + sx, x.end());
        vector<limb> y0(y.begin(), y.begin() + sy), y1(y.begin() + sy, y.end());
        vector<limb> z0 = mul_mag(x0, y0);
        vector<limb> z2 = mul_mag(x1, y1);
        vector<limb> z1 = mul_mag(add_mag(x0, x1), add_mag(y0, y1));
        z1 = sub_mag(z1, z0);
        z1 = sub_mag(z1, z2);
        add_at(z0, z1, h);
        add_at(z0, z2, 2 * h);
        trim(z0);
        return z0;
    }
    static constexpr pair<vector<limb>, vector<limb>> divmod_mag(const vector<limb>& X, const vector<limb>& Y)
    {
        assert(!Y.empty() && "divmod_mag: 除数为零");
        if (cmp_mag(X, Y) < 0) return {{}, X};
        if (Y.size() == 1)
        {
            auto [q, r] = div_small(X, Y[0]);
            vector<limb> rem = r ? vector<limb>{r} : vector<limb>{};
            return {move(q), move(rem)};
        }
        limb s = B / (Y.back() + 1);
        vector<limb> u = mul_small(X, s);
        if (u.size() == X.size()) u.push_back(0);
        vector<limb> v = mul_small(Y, s);
        size_t n = v.size(), m = u.size() - n;
        vector<limb> q(m);
        for (size_t j = m; j-- > 0; )
        {
            u64 num = (u64)u[j + n] * B + u[j + n - 1];
            u64 qh  = num / v[n - 1], rh = num % v[n - 1];
            if (qh == B) { qh = B - 1; rh = num - qh * v[n - 1]; }
            while (rh < B && qh * (u64)v[n - 2] > rh * B + u[j + n - 2])
            { qh--; rh += v[n - 1]; }
            LL c = 0;
            for (size_t i = 0; i < n; i++)
            {
                LL t  = (LL)u[j + i] + c - (LL)(qh * (u64)v[i]);
                LL dg = t % B;
                if (dg < 0) dg += B;
                u[j + i] = (limb)dg;
                c = (t - dg) / B;
            }
            LL top = (LL)u[j + n] + c;
            if (top < 0)
            {
                qh--;
                limb carry = 0;
                for (size_t i = 0; i < n; i++)
                {
                    limb t = u[j + i] + v[i] + carry;
                    u[j + i] = t % B;
                    carry = t / B;
                }
                top += (LL)carry;
                assert(top == 0 && "divmod_mag: add-back 后未归零 (内部错误)");
            }
            assert(top == 0 && "divmod_mag: 窗口未归零 (内部错误)");
            u[j + n] = 0;
            q[j] = (limb)qh;
        }
        u.resize(n);
        auto [r, rr] = div_small(u, s);
        assert(rr == 0 && "divmod_mag: 去规范化不整除 (内部错误)");
        return {move(q), move(r)};
    }
    void parse(const string& s)
    {
        size_t p = 0, n = s.size();
        neg = false;
        if (p < n && (s[p] == '+' || s[p] == '-')) { neg = s[p] == '-'; p++; }
        size_t d0 = p;
        while (p < n && s[p] >= '0' && s[p] <= '9') p++;
        assert(p == n && p > d0 && "BigInt 串解析: 非法字符或空串");
        while (d0 < p && s[d0] == '0') d0++;
        a.clear(); a.reserve((p - d0 + BW - 1) / BW);
        for (size_t e = p; e > d0; )
        {
            size_t st = (e - d0 > (size_t)BW) ? e - BW : d0;
            limb v = 0;
            for (size_t i = st; i < e; i++) v = v * 10 + (s[i] - '0');
            a.push_back(v);
            e = st;
        }
        if (a.empty()) neg = false;
    }
    constexpr void from_i128(i128 val)
    {
        a.clear();
        neg = val < 0;
        u128 m = neg ? (u128)(-(val + 1)) + 1 : (u128)val;
        while (m) { a.push_back((limb)(m % B)); m /= B; }
    }
public:
    constexpr BigInt() = default;
    constexpr BigInt(Intable auto v) { from_i128(v); }
    BigInt(const string& s) { parse(s); }
    BigInt(const char* s)   { parse(s); }
    constexpr BigInt& operator=(Intable auto v) { return *this = BigInt(v); }
    BigInt& operator=(string s)      { return *this = BigInt(move(s)); }
    BigInt& operator=(const char* s) { return *this = BigInt(string(s)); }
    constexpr bool is_zero()  const  { return a.empty(); }
    constexpr int  sign()     const  { return a.empty() ? 0 : (neg ? -1 : 1); }
    constexpr int  digits10() const
    {
        if (a.empty()) return 1;
        int d = (int)(a.size() - 1) * BW;
        for (limb t = a.back(); t; t /= 10) d++;
        return d;
    }
    constexpr BigInt abs() const { BigInt t = *this; t.neg = false; return t; }
    constexpr BigInt operator-() const { BigInt t = *this; if (!t.a.empty()) t.neg = !t.neg; return t; }
    explicit constexpr operator bool() const { return !a.empty(); }
    friend constexpr strong_ordering operator<=>(const BigInt& l, const BigInt& r)
    {
        if (l.neg != r.neg) return l.neg ? strong_ordering::less : strong_ordering::greater;
        int c = cmp_mag(l.a, r.a);
        return (l.neg ? -c : c) <=> 0;
    }
    friend constexpr bool operator==(const BigInt& l, const BigInt& r)
    {
        return l.neg == r.neg && l.a == r.a;
    }
    constexpr BigInt& operator+=(const BigInt& r)
    {
        if (r.a.empty()) return *this;
        if (a.empty()) return *this = r;
        if (neg == r.neg) { a = add_mag(a, r.a); return *this; }
        int c = cmp_mag(a, r.a);
        if (c == 0) { a.clear(); neg = false; }
        else if (c > 0) a = sub_mag(a, r.a);
        else { a = sub_mag(r.a, a); neg = r.neg; }
        return *this;
    }
    constexpr BigInt& operator-=(const BigInt& r)
    {
        if (r.a.empty()) return *this;
        if (a.empty()) { a = r.a; neg = !r.neg; return *this; }
        if (neg != r.neg) { a = add_mag(a, r.a); return *this; }
        int c = cmp_mag(a, r.a);
        if (c == 0) { a.clear(); neg = false; }
        else if (c > 0) a = sub_mag(a, r.a);
        else { a = sub_mag(r.a, a); neg = !neg; }
        return *this;
    }
    constexpr BigInt& operator*=(const BigInt& r)
    {
        bool nn = (a.empty() || r.a.empty()) ? false : (neg != r.neg);
        a = (r.a.size() == 1) ? mul_small(a, r.a[0])
                              : mul_mag(a, r.a);
        neg = nn;
        return *this;
    }
    constexpr pair<BigInt, BigInt> divmod(const BigInt& d) const
    {
        auto [q, r] = divmod_mag(a, d.a);
        BigInt Q, R;
        Q.a = move(q); R.a = move(r);
        Q.neg = !Q.a.empty() && (neg != d.neg);
        R.neg = !R.a.empty() && neg;
        return {move(Q), move(R)};
    }
    constexpr BigInt& operator/=(const BigInt& d) { return *this = divmod(d).first; }
    constexpr BigInt& operator%=(const BigInt& d) { return *this = divmod(d).second; }
    friend constexpr BigInt operator+(BigInt l, const BigInt& r) { return l += r; }
    friend constexpr BigInt operator-(BigInt l, const BigInt& r) { return l -= r; }
    friend constexpr BigInt operator*(BigInt l, const BigInt& r) { return l *= r; }
    friend constexpr BigInt operator/(BigInt l, const BigInt& r) { return l /= r; }
    friend constexpr BigInt operator%(BigInt l, const BigInt& r) { return l %= r; }
    // ==================== 幂 / 开方 / gcd ====================
    constexpr BigInt pow(Intable auto e) const    // 快速幂, 0^0 = 1
    {
        i128 k(e);
        assert(k >= 0 && "pow(): 负指数无整数结果");
        BigInt r(1), b = *this;
        for (; k; k >>= 1)
        {
            if (k & 1) r *= b;
            if (k > 1) b *= b;
        }
        return r;
    }
    constexpr BigInt sqrt() const
    {
        assert(!neg && "sqrt(): 负数无整数平方根");
        if (a.empty()) return {};
        size_t L = a.size();
        BigInt x;
        if (L <= 2)
        {
            u128 v = (u128)a[0];
            if (L == 2) v += (u128)a[1] * B;
            u128 r = isqrt_u128(v);
            if (r) x.a.push_back((limb)r);
            return x;
        }
        u128 T = (u128)a[L - 1] * B + a[L - 2];
        u128 t = ((L - 2) % 2 == 0) ? T : T * B;
        u128 k = isqrt_u128(t) + 2;
        size_t h = (L - 2) / 2;
        x.a.assign(h, 0);
        x.a.push_back((limb)(k % B));
        if (k / B) x.a.push_back((limb)(k / B));
        while (true)
        {
            BigInt y = *this / x + x;
            y /= BigInt(2);
            if (y >= x) break;
            x = move(y);
        }
        while (x * x > *this) x -= BigInt(1);
        return x;
    }
    friend constexpr BigInt gcd(BigInt a, BigInt b)
    {
        a.neg = b.neg = false;
        while (!b.a.empty()) { a %= b; swap(a, b); }
        return a;
    }
    friend constexpr BigInt lcm(const BigInt& a, const BigInt& b)
    {
        if (a.a.empty() || b.a.empty()) return {};
        BigInt r = a / gcd(a, b) * b;
        r.neg = false;
        return r;
    }
    constexpr LL mod(LL m) const
    {
        assert(m >= 1 && "mod(): 模数需 >= 1");
        i128 r = 0;
        for (size_t i = a.size(); i-- > 0; ) r = (r * B + a[i]) % m;
        return (LL)(neg && r ? m - r : r);
    }
    constexpr i128 to_i128() const
    {
        constexpr i128 MX = numeric_limits<i128>::max();
        i128 r = 0;
        for (size_t i = a.size(); i-- > 0; )
        {
            assert(r <= (MX - a[i]) / B && "to_i128(): 超出可表示范围");
            r = r * B + a[i];
        }
        return neg ? -r : r;
    }
    constexpr LL to_LL() const
    {
        i128 v = to_i128();
        assert(v >= numeric_limits<LL>::min() && v <= numeric_limits<LL>::max() && "to_LL(): 溢出");
        return (LL)v;
    }
    // ==================== 字符串 / IO ====================
    string str() const
    {
        if (a.empty()) return "0";
        string s; s.reserve(a.size() * BW + 2);
        if (neg) s.push_back('-');
        s += to_string(a.back());
        char buf[BW];
        for (size_t i = a.size() - 1; i-- > 0; )
        {
            limb t = a[i];
            for (int k = BW - 1; k >= 0; k--) { buf[k] = char('0' + t % 10); t /= 10; }
            s.append(buf, sizeof(buf));
        }
        return s;
    }
    friend string to_string(const BigInt& x) { return x.str(); }
    friend istream& operator>>(istream& is, BigInt& x)
    {
        string s;
        if (is >> s) x = move(s);
        return is;
    }
    friend ostream& operator<<(ostream& os, const BigInt& x) { return os << x.str(); }
    static constexpr BigInt factorial(u64 n)
    {
        return n < 2 ? BigInt(1) : prod_range(2, n);
    }
private:
    static constexpr u128 isqrt_u128(u128 v)
    {
        if (v == 0) return 0;
        u128 r = 1;
        while (r * r <= v) r <<= 1;
        while (true)
        {
            u128 t = (r + v / r) / 2;
            if (t >= r) break;
            r = t;
        }
        while (r * r > v) r--;
        return r;
    }
    static constexpr BigInt prod_range(u64 lo, u64 hi)
    {
        if (hi - lo <= 64)
        {
            BigInt r((u64)lo);
            for (u64 k = lo + 1; k <= hi; k++) r *= (u64)k;
            return r;
        }
        u64 mid = lo + (hi - lo) / 2;
        return prod_range(lo, mid) * prod_range(mid + 1, hi);
    }
};
#endif

/* Usage:
 * =====================================================================
 * 1. 构造与赋值 (通吃所有类型)
 * =====================================================================
 *   BigInt a;                               // 默认构造为 0
 *   BigInt b = 5, c = -7LL;                 // 直接接收原生整数 (隐式构造)
 *   BigInt d = (i128)1 << 100;              // 完美接收 __int128 超大基元
 *   BigInt e("1234567890123456789012345");  // 支持任意长度字符串构造
 *   a = "-000987";                          // 安全过滤前导零和冗余负号
 *
 * =====================================================================
 * 2. 基础算术与比较 (完美融入 C++ 运算符)
 * =====================================================================
 *   a = b + c;  a += b;
 *   a = b - c;  a -= b;
 *   a = b * c;  a *= 2;                     // 整数放右侧直接混算
 *   a = 100 - b;                            // 整数放左侧直接混算
 *
 *   // 极其好用的强三路比较 (基于 C++20 <=>)
 *   if (a < b) ...;  if (a >= 0) ...;  if (a == "100000") ...;
 *
 *   // 带余除法
 *   auto [q, r] = a.divmod(b);
 *   a = b / c;  a /= b;                     // 截断除法 (商向零取整)
 *   a = b % c;  a %= b;                     // 余数符号永远与被除数(b)一致
 *
 * =====================================================================
 * 3. 高级数学运算
 * =====================================================================
 *   BigInt p = BigInt(2).pow(1000);         // 快速幂, 指数支持 i128/LL, 0^0 = 1
 *   BigInt s = a.sqrt();                    // 下取整平方根; ≤2e4 位毫秒级, 1e5 位≈秒级 (牛顿步数=位数对数级)
 *   BigInt g = gcd(a, b);                   // 最大公约数 (内置辗转相除)
 *   BigInt l = lcm(a, b);                   // 最小公倍数 (恒非负, 对齐标准库 lcm)
 *   BigInt f = BigInt::factorial(100000);   // 阶乘 (乘积树算法, 算10万阶乘仅需几秒!)
 *
 * =====================================================================
 * 4. 辅助状态与 IO
 * =====================================================================
 *   if (a) ...;                             // 判非 0
 *   if (a.is_zero()) ...;                   // 判断是否为 0
 *   int sgn = a.sign();                     // 返回符号: 1(正), -1(负), 0(零)
 *   int len = a.digits10();                 // O(1) 预估十进制位数 (不必转成字符串就能知道多长)
 *   BigInt abs_a = a.abs();                 // 取绝对值
 *
 *   cin >> a;  cout << a << '\n';           // 无缝接管标准 IO 流
 *   string s = a.str();                     // 转为 string
 *   string s2 = to_string(a);               // 兼容 STL 习惯
 *
 * =====================================================================
 * 5. 生态联动
 * =====================================================================
 *   // 桥梁 1: BigInt 降维到原生类型
 *   LL val = a.mod(998244353);              // [重要] 获取大数对某 LL 取模的值
 *   i128 v = a.to_i128();                   // 强转 i128 (若溢出会触发 assert)
 *   LL v2  = a.to_LL();                     // 强转 LL   (若溢出会触发 assert)
 *
 *   // 桥梁 2: 注入 ModLL
 *   mint m = mint(a.mod(MOD));              // 先用 mod() 降维, 再无缝塞给 mint
 *
 *   // 桥梁 3: 注入 Graph
 *   Graph<false, BigInt> g(n, m);           // 将边权直接设为 BigInt，跑大数最短路！
 *
 * =====================================================================
 * 6. 编译期元编程 (C++20 Constexpr)
 * =====================================================================
 *   // 只要 GCC >= 12, 你可以直接在编译期算高精度
 *   constexpr BigInt MAGIC = BigInt(114514).pow(100);
 *   static_assert(MAGIC % 10 == 6);
 * =====================================================================
 */
