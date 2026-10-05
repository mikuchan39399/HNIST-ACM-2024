// zoi: binomFactInv
#ifndef Z_OI_BINOM_FACT_INV
#define Z_OI_BINOM_FACT_INV

#include "../../杂项/128位整数/128int.cpp"

// 运行时素数模组合数表; fact[i]=i!, inv_fact[i]=1/i!, 每对象独立. 空间 O(n)
struct PrimeComb
{
    LL mod = 0;
    VLL fact, inv_fact;

    // 先 init(n,p), p 为素数, 0<=n<p 且 n<INT_MAX. 时间 O(n+log p), 空间 O(n)
    void init(int n, LL p)
    {
        assert(n >= 0 && n < INT_MAX && p >= 2 && n < p);
        mod = p;
        fact.resize(n + 1);
        inv_fact.resize(n + 1);
        fact[0] = 1;
        for (int i = 1; i <= n; i++) fact[i] = (i128)fact[i - 1] * i % mod;
        LL inv = 1, a = fact[n];
        for (LL e = mod - 2; e; e >>= 1, a = (i128)a * a % mod)
            if (e & 1) inv = (i128)inv * a % mod;
        inv_fact[n] = inv;
        for (int i = n; i; i--) inv_fact[i - 1] = (i128)inv_fact[i] * i % mod;
    }
    // C(n,k) mod p, n 在表内, k 越界为 0. O(1)
    LL comb(int n, int k) const
    {
        assert(n >= 0 && n < (int)fact.size());
        if (k < 0 || k > n) return 0;
        return (i128)fact[n] * inv_fact[k] % mod * inv_fact[n - k] % mod;
    }
};
#endif

/* Usage
#include "binomFactInv.h"

int main()
{
    int limit, q;
    LL p;
    if (!(cin >> limit >> q >> p)) return 0; // 输入示例: 10 2 1000000007
    PrimeComb c;
    c.init(limit, p);                       // p 为素数, limit 必须小于 p
    while (q--)
    {
        int n, k;
        cin >> n >> k;                      // 后接: 10 3 / 5 2
        cout << c.comb(n, k) << '\n';        // 120 / 10
    }
    cout << c.fact[0] << ' ' << c.inv_fact[0] << '\n'; // 1 1
    c.init(4, 5);                           // 多测换模数, 旧表被覆盖
    cout << c.comb(4, 2) << '\n';            // 1

}
*/
