// zoi: mint
#ifndef Z_OI_MODLL
#define Z_OI_MODLL

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cassert>
#include <algorithm>
#include <type_traits>
#include <utility>
#include <tuple>
#include "../../杂项/128位整数/128int.cpp"
#include "../../杂项/utils/utils.cpp"

using namespace std;


template <class T>
concept Mintable = (is_integral_v<T> && sizeof(T) <= 8) || is_same_v<T, i128>;
// 注: 严格 -std=c++20 下 libstdc++ 的 is_integral_v<__int128> 为 false, 故 i128 单列

template <LL MOD>
class ModLL
{
    static_assert(MOD > 1,           "模数必须 >= 2");
    static_assert(MOD < (1LL << 62), "模数过大, 无法保证加减不溢出 LL");
    static constexpr LL mul(LL a, LL b, LL m) { return (LL)((i128)a * b % m); }
    static constexpr LL mpow(LL a, LL e, LL m)
    {
        LL r = 1;
        for (; e; e >>= 1, a = mul(a, a, m))
            if (e & 1) r = mul(r, a, m);
        return r;
    }
    static constexpr bool is_prime(LL n)
    {
        if (n < 2) return false;
        for (LL p : {2LL,3LL,5LL,7LL,11LL,13LL,17LL,19LL,23LL,29LL,31LL,37LL})
            if (n % p == 0) return n == p;
        LL d = n - 1; int s = 0;
        while (!(d & 1)) d >>= 1, ++s;
        for (LL a : {2LL,3LL,5LL,7LL,11LL,13LL,17LL,19LL,23LL,29LL,31LL,37LL})
        {
            LL x = mpow(a, d, n);
            if (x == 1 || x == n - 1) continue;
            bool comp = true;
            for (int i = 1; i < s; i++)
            {
                x = mul(x, x, n);
                if (x == n - 1) { comp = false; break; }
            }
            if (comp) return false;
        }
        return true;
    }
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
public:
    static constexpr bool PRIME = is_prime(MOD);
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
    ModLL& operator/=(const ModLL& r) { return *this *= r.inv(); }
    friend constexpr ModLL operator+(ModLL l, const ModLL& r) { return l += r; }
    friend constexpr ModLL operator-(ModLL l, const ModLL& r) { return l -= r; }
    friend constexpr ModLL operator*(ModLL l, const ModLL& r) { return l *= r; }
    friend ModLL operator/(ModLL l, const ModLL& r) { return l /= r; }
    constexpr ModLL operator-() const { return ModLL(-x); }
    constexpr bool operator==(const ModLL& o) const { return x == o.x; }
    constexpr bool operator!=(const ModLL& o) const { return x != o.x; }
    // ---------------- 幂 / 逆元 ----------------
    constexpr ModLL pow(i128 n) const
    {
        if (n < 0) return inv().pow(-n);
        ModLL r(1), a = *this;
        for (; n; n >>= 1)
        {
            if (n & 1) r *= a;
            a *= a;
        }
        return r;
    }
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
    // ---------------- IO ----------------
    friend istream& operator>>(istream& is, ModLL& o)
    {
        string s;                                   // 逐位折模, 不经 i128, 任意位数不溢出
        if (!(is >> s)) return is;
        bool neg = (s[0] == '-');
        size_t i = (s[0] == '-') || (s[0] == '+');
        LL r = 0;
        for (; i < s.size(); i++) r = (mulmod(r, 10) + (s[i] - '0')) % MOD;
        o = ModLL(neg ? MOD - r : r);
        return is;
    }
    friend ostream& operator<<(ostream& os, const ModLL& o) { return os << o.x; }
    // ---------------- 阶乘 & 组合数 (自动路由) ----------------
    static inline vector<ModLL> fact, inv_fact;
    static inline VI spf;
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
};
#endif

/* Usage:
 * =====================================================================
 * 1. 引入与起别名
 * =====================================================================
 *   using LL = long long;
 *   using VI = vector<int>;
 *   const LL MOD = 1e9 + 7;        // 任意模数, 质数/非质数均可 (1 < MOD < 2^62)
 *   using mint = ModLL<MOD>;       // 统一别名，多模数可再起 using mint2 = ModLL<998244353>;
 *
 * =====================================================================
 * 2. 基础构造与四则运算
 * =====================================================================
 *   mint a = 10, b = 20;           // 自动取模, 支持 int/LL/u64/i128
 *   mint c = -1;                   // 极其好用：自动转换为 MOD - 1
 *   mint d = a + b;                // 加法
 *   mint e = a - b;                // 减法
 *   mint f = a * b;                // 乘法 (内部根据 MOD^2 大小自动路由 LL 或 i128)
 *   mint g = a / b;                // 除法 (等价于 a * b.inv())
 *   mint h = -a;                   // 一元负号: 返回 MOD - a
 *
 *   // 混合隐式转换: 字面量或普通变量放两边都可以直接算
 *   a = a + 3;  b = 5 - a;
 *   LL y = 10;  a = a * y;
 *   if (a == 3 && b != y) ...      // 比较运算完美支持隐式转换
 *
 * =====================================================================
 * 3. 幂与逆元
 * =====================================================================
 *   mint p1 = a.pow(100);          // 快速幂
 *   mint p2 = a.pow(-5);           // 【隐藏绝招】支持负指数: 自动计算逆元的正数次幂
 *   mint p3 = a.pow((i128)1e30);   // 指数完全支持超大 i128 类型
 *
 *   mint inv = a.inv();            // 逆元 (质数: 费马小定理 O(logM); 非质数: exgcd O(logM))
 *                                  // 注意: 非质数模数下, 若 gcd(a, MOD) != 1 则触发 assert 断言
 *
 * =====================================================================
 * 4. 阶乘与组合数 (极其智能的内部路由!!!)
 * =====================================================================
 *   // [初始化] 必须在 main 开头调用一次预处理 (以最大可能的 N 为准)
 *   mint::init_fact(200000);
 *   // 注意: 质数模数下 N 必须 < MOD (n! ≥ MOD 时不可逆, init 即 assert);
 *   //       需查 n ≥ MOD 的组合数请改用 Lucas, 勿扩表
 *
 *   // [求组合数] 全模数通用
 *   mint ans = mint::comb(10, 3);  // 求 C(10, 3), 若 k < 0 或 k > n 会安全返回 0
 *     -> 如果 MOD 是质数:   直接查表, 单次 O(1)
 *     -> 如果 MOD 是非质数: 触发 spf 线性筛+质因数追踪算法, 单次 O(min(k, n-k) * logN)
 *
 *   // [访问阶乘] 仅限 PRIME = true 时可用!
 *   // 非质数下 init_fact 为了效率根本没有计算 fact 数组，越界必报错。
 *   mint f5   = mint::fact[5];     // 5!
 *   mint inv5 = mint::inv_fact[5]; // 1 / 5!
 *
 * =====================================================================
 * 5. 输入输出与取值
 * =====================================================================
 *   cin >> a;                      // 逐位折模读入, 任意位数十进制(负号支持)不溢出
 *   cout << a << '\n';             // 直接输出
 *   LL val = a.val();              // 脱离 mint 外壳，获取最原始的 LL 值
 *
 * =====================================================================
 * 6. 编译期与元编程支持
 * =====================================================================
 *   static_assert(mint::PRIME);                          // 编译期获取模数是否为素数
 *   static_assert(mint(5).inv() == mint(400000003));     // 全链路 constexpr，可写编译期单测
 *   constexpr mint MAGIC = mint(114514).pow(1919810);    // 在编译期直接算出结果，运行时 0 开销！
 * =====================================================================
 */
