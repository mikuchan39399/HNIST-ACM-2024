// zoi: mint
#ifndef Z_OI_MODLL
#define Z_OI_MODLL

#include "../../杂项/128位整数/128int.cpp"
#include "../../杂项/utils/utils.cpp"

template <class T>
concept Mintable = (is_integral_v<T> && sizeof(T) <= 8) || is_same_v<T, i128>;
// 注: 严格 -std=c++20 下 libstdc++ 的 is_integral_v<__int128> 为 false, 故 i128 单列

// 编译期固定模数 1 < MOD < 2^62, 每个对象只存 [0, MOD) 内的一个 LL。
// 接收至多 64 位整数及有符号 i128; 四则与比较两侧均可混用整数。
// 加减乘 O(1), 除法/逆元 O(log MOD); 求逆与负指数要求 gcd(x, MOD)=1。
// 阶乘表与筛表按模数共享, 不调用 init_fact 就不分配; 不同模数互不影响。
template <LL MOD>
class ModLL
{
    static_assert(MOD > 1,           "模数必须 >= 2");
    static_assert(MOD < (1LL << 62), "模数过大, 无法保证加减不溢出 LL");
public:
    static constexpr bool PRIME = []
    {
        auto mul = [](LL a, LL b) { return (LL)((i128)a * b % MOD); };
        auto mpow = [&](LL a, LL e)
        {
            LL r = 1;
            for (; e; e >>= 1, a = mul(a, a))
                if (e & 1) r = mul(r, a);
            return r;
        };
        for (LL p : {2LL, 3LL, 5LL, 7LL, 11LL, 13LL, 17LL, 19LL, 23LL, 29LL, 31LL, 37LL})
            if (MOD % p == 0) return MOD == p;
        LL d = MOD - 1; int s = 0;
        while (!(d & 1)) d >>= 1, ++s;
        for (LL a : {2LL, 3LL, 5LL, 7LL, 11LL, 13LL, 17LL, 19LL, 23LL, 29LL, 31LL, 37LL})
        {
            LL x = mpow(a, d);
            if (x == 1 || x == MOD - 1) continue;
            bool comp = true;
            for (int i = 1; i < s; i++)
            {
                x = mul(x, x);
                if (x == MOD - 1) { comp = false; break; }
            }
            if (comp) return false;
        }
        return true;
    }();
    constexpr ModLL() : x(0) {}
    constexpr ModLL(Mintable auto v) : x(norm(v)) {}
    constexpr LL val() const { return x; }
    constexpr ModLL& operator+=(const ModLL& r)
    {
        x += r.x - MOD;
        if (x < 0) x += MOD;
        return *this;
    }
    constexpr ModLL& operator-=(const ModLL& r)
    {
        x -= r.x;
        if (x < 0) x += MOD;
        return *this;
    }
    constexpr ModLL& operator*=(const ModLL& r) { x = mulmod(x, r.x); return *this; }
    constexpr ModLL& operator/=(const ModLL& r) { return *this *= r.inv(); }
    friend constexpr ModLL operator+(ModLL l, const ModLL& r) { return l += r; }
    friend constexpr ModLL operator-(ModLL l, const ModLL& r) { return l -= r; }
    friend constexpr ModLL operator*(ModLL l, const ModLL& r) { return l *= r; }
    friend constexpr ModLL operator/(ModLL l, const ModLL& r) { return l /= r; }
    constexpr ModLL operator-() const { return ModLL(-x); }
    friend constexpr bool operator==(const ModLL& l, const ModLL& r) { return l.x == r.x; }
    friend constexpr bool operator!=(const ModLL& l, const ModLL& r) { return l.x != r.x; }
    // O(log(|n|+1)), n<0 另求逆; 约定 0^0=1, 支持 i128 全范围。
    constexpr ModLL pow(i128 n) const
    {
        ModLL r(1), a = *this;
        u128 e = n; // 在无符号域取绝对值, 避免最小 i128 取负溢出
        if (n < 0) a = inv(), e = -e;
        for (; e; e >>= 1)
        {
            if (e & 1) r *= a;
            a *= a;
        }
        return r;
    }
    // 返回乘法逆元; 不可逆时断言失败, 合数模下不能直接用费马小定理。
    constexpr ModLL inv() const
    {
        assert(x != 0);
        if constexpr (PRIME) return pow(MOD - 2);
        else
        {
            LL s, t;
            LL g = exgcd(x, MOD, s, t);
            assert(g == 1 && "inv(): gcd(x, MOD) != 1, 逆元不存在");
            return ModLL(s);
        }
    }
    // 读合法十进制整数, 可带正负号; d 位耗时/临时空间 O(d), 输出标准余数。
    friend istream& operator>>(istream& is, ModLL& o)
    {
        string s;
        if (!(is >> s)) return is;
        bool neg = (s[0] == '-');
        size_t i = (s[0] == '-') || (s[0] == '+');
        LL r = 0;
        for (; i < s.size(); i++) r = (mulmod(r, 10) + (s[i] - '0')) % MOD;
        o = ModLL(neg ? MOD - r : r);
        return is;
    }
    friend ostream& operator<<(ostream& os, const ModLL& o) { return os << o.x; }
    // 素数模只填 fact/inv_fact, 合数模只填最小质因子 spf。
    static inline vector<ModLL> fact, inv_fact;
    static inline VI spf;
    // 重建 [0,n] 的共享表, n>=0; 素数模另要求 n<MOD, 不支持跨模 Lucas。
    // 素数模 O(n+log MOD), 合数模 O(n), 空间 O(n); 缩表保留 vector 容量。
    static void init_fact(int n)
    {
        if constexpr (PRIME)
        {
            fact.resize(n + 1); inv_fact.resize(n + 1);
            fact[0] = 1;
            for (int i = 1; i <= n; i++) fact[i] = fact[i - 1] * i;
            inv_fact[n] = fact[n].inv();
            for (int i = n - 1; i >= 0; i--) inv_fact[i] = inv_fact[i + 1] * (i + 1);
        }
        else
        {
            spf.assign(n + 1, 0);
            VI pr;
            for (int i = 2; i <= n; i++)
            {
                if (!spf[i]) spf[i] = i, pr.push_back(i);
                for (int p : pr)
                {
                    if ((LL)p * i > n) break;
                    spf[p * i] = p;
                    if (i % p == 0) break;
                }
            }
        }
    }
    // C(n,k) mod MOD; n>=0 且已预处理到 n, k 越界返回 0。
    // 素数模 O(1); 合数模逐因子相消, k'=min(k,n-k), O(k' log^2(n+1))。
    // 合数模每次用 O(k' log(n+1)) 临时空间, 适合少量查询, 不作 O(1) 查表。
    static ModLL comb(int n, int k)
    {
        if (k < 0 || k > n) return ModLL(0);
        if constexpr (PRIME)
        {
            assert(n < (int)fact.size());
            return fact[n] * inv_fact[k] * inv_fact[n - k];
        }
        else
        {
            assert(n < (int)spf.size());
            k = min(k, n - k);
            map<int, int> e;
            auto add = [&](int v, int d)
            {
                while (v > 1)
                {
                    int p = spf[v];
                    do { e[p] += d; v /= p; } while (v % p == 0);
                }
            };
            for (int i = 1; i <= k; i++)
            {
                add(n - k + i, +1);
                add(i, -1);
            }
            ModLL r(1);
            for (auto& [p, c] : e)
                if (c) r *= ModLL(p).pow(c);
            return r;
        }
    }
private:
    template <class T>
    static constexpr LL norm(T v)
    {
        if constexpr (is_same_v<T, i128> || is_signed_v<T>)
        { v %= MOD; return (LL)(v < 0 ? v + MOD : v); }
        else return (LL)(v % MOD);
    }
    static constexpr bool DIRECT_MUL = MOD <= 3037000499LL;
    static constexpr LL mulmod(LL a, LL b)
    {
        if constexpr (DIRECT_MUL) return a * b % MOD;
        else                      return (LL)((i128)a * b % MOD);
    }
    static constexpr LL exgcd(LL a, LL b, LL& s, LL& t)
    {
        if (!b) { s = 1; t = 0; return a; }
        LL g = exgcd(b, a % b, t, s);
        t -= a / b * s;
        return g;
    }
    LL x;
};
#endif


/* Usage:
#include "mint.h"

using mint = ModLL<1000000007>;
using mint12 = ModLL<12>;
int main()
{
    mint a = -1, b = 3;
    cout << a + b << ' ' << 5 - b << '\n'; // 2 2
    cout << b / 3 << ' ' << (3 == b) << '\n'; // 1 1
    cout << (b.pow(-2) * b * b).val() << '\n'; // 1
    static_assert(mint(6) / 3 == 2);

    mint::init_fact(200000); // 素数模: 最大 n 必须小于 MOD
    cout << mint::comb(10, 3) << ' ' << mint::fact[5] << '\n'; // 120 120
    mint12::init_fact(100); // 合数模: 不除阶乘, 单次组合数查询较慢
    cout << mint12::comb(10, 3) << ' ' << mint12(5).inv() << '\n'; // 0 5
    // cin >> a; // 可直接读取任意位数的合法十进制整数
}
*/
