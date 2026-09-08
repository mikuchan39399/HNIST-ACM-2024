// zoi: binomPascal
#ifndef Z_OI_BINOM_PASCAL
#define Z_OI_BINOM_PASCAL

#include "../../杂项/128位整数/128int.cpp"

// 杨辉三角 C(n,k)=C(n-1,k-1)+C(n-1,k), 无除法, 模数可为合数或 1
// 保存下三角, 有效数据共 4(n+1)(n+2) B 加行容器, n=2000 约 16 MB
struct PascalComb
{
    LL mod = 0;
    vector<VLL> c;

    // 重建 [0,n] 所有组合数, 0<=n<INT_MAX, 1<=p<=LLONG_MAX; O(n^2) 时间/空间
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
    // 返回 C(n,k) mod mod, k 越界返回 0; n 在已建范围内, O(1) 时间/额外空间
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
    // 合数模且小 n 多查询时适用; mint 的合数模 comb 不提供 O(1) 查询
}
*/
