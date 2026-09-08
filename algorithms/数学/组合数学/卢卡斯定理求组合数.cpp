// zoi: lucas
#ifndef Z_OI_LUCAS
#define Z_OI_LUCAS

#include "阶乘表及阶乘逆元表求组合数.cpp"

// 素数模下 C(n,k) 等于各 p 进位组合数的乘积, 任一位 k_i>n_i 即为 0
// 适合 n/k 很大而素数 p 可建表的场景; 复用 PrimeComb, 有效表数据共 16p B
struct Lucas
{
    PrimeComb table;

    // 按素数 p 重建 [0,p-1] 阶乘表, p>=2; O(p) 时间/空间, 非扩展 Lucas
    void init(int p)
    {
        assert(p >= 2);
        table.init(p - 1, p);
    }
    // 返回 C(n,k) mod p, k 越界返回 0; 0<=n<=LLONG_MAX, O(1+log_p(n+1)) 时间
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
    // 即使 p 是素数, p=1e9+7 也不适合整张表; 大 n 小 k 可选 binom_loop
}
*/
