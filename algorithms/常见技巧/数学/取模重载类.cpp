#include <iostream>
#include <vector>
#include <cassert>

using namespace std;
using LL = long long;

// 需要 c++17 支持 inline 静态成员
template <LL MOD>
class ModLL 
{
    LL x;  // 0 <= x < MOD
public:
    // 构造函数
    constexpr ModLL(LL v = 0) : x((v % MOD + MOD) % MOD) {}
    constexpr ModLL(int v) : ModLL(static_cast<LL>(v)) {}
    constexpr LL val() const { return x; }
    // 运算
    ModLL& operator+=(const ModLL& rhs) {
        x += rhs.x;
        if (x >= MOD) x -= MOD;
        return *this;
    }
    ModLL& operator-=(const ModLL& rhs) {
        x -= rhs.x;
        if (x < 0) x += MOD;
        return *this;
    }
    ModLL& operator*=(const ModLL& rhs) {
        x = (x * rhs.x) % MOD;
        return *this;
    }
    ModLL& operator/=(const ModLL& rhs) {
        *this *= rhs.inv();
        return *this;
    }
    friend ModLL operator+(ModLL l, const ModLL& r) { return l += r; }
    friend ModLL operator-(ModLL l, const ModLL& r) { return l -= r; }
    friend ModLL operator*(ModLL l, const ModLL& r) { return l *= r; }
    friend ModLL operator/(ModLL l, const ModLL& r) { return l /= r; }
    ModLL operator-() const { return ModLL(-x); }
    bool operator==(const ModLL& o) const { return x == o.x; }
    bool operator!=(const ModLL& o) const { return x != o.x; }
    ModLL pow(LL n) const {
        ModLL res(1), a = *this;
        while (n) {
            if (n & 1) res *= a;
            a *= a;
            n >>= 1;
        }
        return res;
    }
    ModLL inv() const 
    { 
        assert(x != 0);
        return pow(MOD - 2); 
    }
    friend istream& operator>>(istream& is, ModLL& obj) {
        LL v; is >> v;
        obj = ModLL(v);
        return is;
    }
    friend ostream& operator<<(ostream& os, const ModLL& obj) {
        return os << obj.x;
    }
    // 阶乘 & 组合数
    static inline vector<ModLL> fact, inv_fact;
    static void init_fact(int n) {
        fact.resize(n + 1);
        inv_fact.resize(n + 1);
        fact[0] = 1;
        for (int i = 1; i <= n; i++) fact[i] = fact[i - 1] * i;
        inv_fact[n] = fact[n].inv();
        for (int i = n - 1; i >= 0; i--) inv_fact[i] = inv_fact[i + 1] * (i + 1);
    }
    static ModLL comb(int n, int k) {
        if (k < 0 || k > n) return ModLL(0);
        assert(n < fact.size());
        return fact[n] * inv_fact[k] * inv_fact[n - k];
    }
};

/* Usage: 
 * ===============================================================
 * 在模板中定义：
 *   using LL = long long;
 *   const LL MOD = 1e9 + 7;    // 任意大素数模数
 * 请添加：
 *   using mint = ModLL<MOD>;   // 统一别名
 *
 * ========== 基本使用 ==========
 *   mint a = 10, b = 20;          // 自动取模
 *   mint c = a + b;               // 加减乘除
 *   mint d = a - b;
 *   mint e = a * b;
 *   mint f = a / b;               // 等价于 a * b.inv()
 *   mint g = a.pow(100);          // a^100 mod MOD
 *   mint h = a.inv();             // 逆元 (MOD必须为素数)
 *   mint arr[100];                // 数组全部初始化为 0
 *
 *   // 打印/输入 直接用
 *   cout << a << endl;
 *   cin >> a;
 *
 * ========== 组合数与阶乘 ==========
 *   一次性预计算阶乘 (main 开头)：
 *     mint::init_fact(200000);    // 根据题目最大 n
 *   之后任意处调用：
 *     mint ans = mint::comb(10, 3);  // C(10,3)
 *
 * ========== 注意事项 ==========
 * 1. MOD 必须为素数 (逆元用费马小定理)。
 * 2. init_fact(n) 里 n 不要超过 MOD-1，且只在 main 前调用一次。
 * 3. mint 可以和字面量 (如 a+5) 混合运算，但与 LL 变量运算需显式构造：
 *      LL x = 10; mint ans = mint(x) + a;
 * 4. 需要原始 LL 值时用 .val()，例：cout << a.val()。
 * 5. 若需多模数，另起别名即可：using mint2 = ModLL<998244353>;。
 * ================================================================
 */