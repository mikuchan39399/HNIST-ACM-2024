// zoi: mint
#ifndef Z_OI_MODLL
#define Z_OI_MODLL

#include "../../杂项/128位整数/128int.cpp"
#include "../../杂项/utils/utils.cpp"

template <class T>
concept Mintable = (is_integral_v<T> && sizeof(T) <= 8) || is_same_v<T, i128>;

// 固定模数 1<MOD<2^62, PRIME 表示是否素数; 输入自动归一化, 不必小于 MOD
// 仅 comb/fact/inv_fact/spf 须先 init_fact; 同模数共享表, 最近一次预处理决定范围
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
    // 构造余数, 默认 0; 支持 <=64 位整数及有符号 i128. O(1)
    constexpr ModLL() : x(0) {}
    constexpr ModLL(Mintable auto v) : x(norm(v)) {}

    // 返回 [0,MOD) 的值. O(1)
    constexpr LL val() const { return x; }

    // 加减乘/取负/比较 O(1), 可混用整数; 除法条件及开销同 inv
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

    // n 次幂, 0^0=1. 时间 O(log(|n|+1)), 空间 O(1); n<0 另需 inv
    constexpr ModLL pow(i128 n) const
    {
        ModLL r(1), a = *this;
        u128 e = n;
        if (n < 0) a = inv(), e = -e;
        for (; e; e >>= 1)
        {
            if (e & 1) r *= a;
            a *= a;
        }
        return r;
    }

    // 逆元: 素数模要求 x!=0, 小费马时间 O(log MOD), 空间 O(1)
    // 合数模要求 gcd(x,MOD)=1, 扩欧时间/栈 O(log(x+1)), 最坏 O(log MOD)
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

    // 读任意位合法十进制整数, 可带正负号. 时空 O(位数)
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

    // 输出标准余数. 时间 O(位数), 空间 O(1)
    friend ostream& operator<<(ostream& os, const ModLL& o) { return os << o.x; }

    static inline vector<ModLL> fact, inv_fact;
    static inline VI spf;

    // 重建 0..n 的表, 0<=n<INT_MAX
    // 素数模须 n<MOD: fact[i]=i!, inv_fact[i]=1/i!, 时间 O(n+log MOD), 空间 O(n)
    // 合数模可 n>=MOD: 仅建 spf 供 comb 用, 时空 O(n)
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

    // 先 init_fact(N), 0<=n<=N; 返回 C(n,k), k 越界为 0
    // 素数模 O(1); 合数模 t=min(k,n-k), 时间 O(t log²(n+1)), 空间 O(t log(n+1))
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
    mint a = -1, b = 3; // 下面的运算均不需要 init_fact
    cout << a + b << ' ' << 5 - b << '\n'; // 2 2
    cout << b / 3 << ' ' << (3 == b) << '\n'; // 1 1
    cout << (b.pow(-2) * b * b).val() << '\n'; // 1
    static_assert(mint(6) / 3 == 2);

    mint::init_fact(200000); // 先预处理, 上限<MOD
    cout << mint::comb(10, 3) << ' ' << mint::fact[5] << '\n'; // 120 120
    mint12::init_fact(100); // 合数模只建 spf, 可超过 MOD
    cout << mint12::comb(10, 3) << ' ' << mint12(5).inv() << '\n'; // 0 5
    // cin >> a;
}
*/
