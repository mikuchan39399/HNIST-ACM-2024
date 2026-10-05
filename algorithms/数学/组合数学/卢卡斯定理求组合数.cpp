// zoi: lucas
#ifndef Z_OI_LUCAS
#define Z_OI_LUCAS

#include "阶乘表及阶乘逆元表求组合数.cpp"

// Lucas, 素数模 p; 按 p 建表, 须容纳 O(p) 空间
struct Lucas
{
    PrimeComb table;

    // 先 init(p), p>=2 为素数. 时间/空间 O(p)
    void init(int p)
    {
        assert(p >= 2);
        table.init(p - 1, p);
    }
    // C(n,k) mod p, n>=0, k 越界为 0. 时间 O(1+log_p(n+1)), 空间 O(1)
    LL comb(LL n, LL k) const
    {
        assert(n >= 0 && table.mod >= 2);
        if (k < 0 || k > n) return 0;
        LL ans = 1, p = table.mod;
        while (k)
        {
            int a = n % p, b = k % p;
            if (b > a) return 0;
            ans = (i128)ans * table.comb(a, b) % p;
            n /= p;
            k /= p;
        }
        return ans;
    }
};
#endif

/* Usage
#include "lucas.h"

int main()
{
    int p, q;
    if (!(cin >> p >> q)) return 0;      // 输入示例: 7 2
    Lucas c;
    c.init(p);                         // p 必须为素数, 按 p 而非 n 分配空间
    while (q--)
    {
        LL n, k;
        cin >> n >> k;                  // 后接: 100 50 / 8 1
        cout << c.comb(n, k) << '\n';    // 4 / 1
    }
    c.init(2);                         // 多测换模数, 2 同样合法
    cout << c.comb(8, 1) << '\n';        // 0

}
*/
