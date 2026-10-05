// zoi: binomPascal
#ifndef Z_OI_BINOM_PASCAL
#define Z_OI_BINOM_PASCAL

#include "../../杂项/128位整数/128int.cpp"

// 杨辉三角组合数表, 模数可为合数或 1; 空间 O(n²)
struct PascalComb
{
    LL mod = 0;
    vector<VLL> c;

    // 先 init(n,p), 0<=n<INT_MAX, 1<=p<=LLONG_MAX. 重建时空 O(n²)
    void init(int n, LL p)
    {
        assert(n >= 0 && n < INT_MAX && p >= 1);
        mod = p;
        c.resize(n + 1);
        for (int i = 0; i <= n; i++)
        {
            c[i].resize(i + 1);
            c[i][0] = c[i][i] = 1 % mod;
            for (int j = 1; j < i; j++)
                c[i][j] = ((i128)c[i - 1][j - 1] + c[i - 1][j]) % mod;
        }
    }
    // C(n,k) mod p, n 在表内, k 越界为 0. O(1)
    LL comb(int n, int k) const
    {
        assert(n >= 0 && n < (int)c.size());
        return k < 0 || k > n ? 0 : c[n][k];
    }
};
#endif

/* Usage
#include "binomPascal.h"

int main()
{
    int limit, q;
    LL p;
    if (!(cin >> limit >> q >> p)) return 0; // 输入示例: 10 2 12
    PascalComb c;
    c.init(limit, p);
    while (q--)
    {
        int n, k;
        cin >> n >> k;                      // 后接: 10 3 / 6 2
        cout << c.comb(n, k) << '\n';        // 0 / 3
    }
    cout << c.c[0][0] << '\n';              // 1, c[n] 只有下标 0..n
    c.init(3, 1);                           // 多测换模数, 重新生成全部状态
    cout << c.comb(3, 0) << '\n';            // 0, 包括 C(n,0) 也要取模

}
*/
