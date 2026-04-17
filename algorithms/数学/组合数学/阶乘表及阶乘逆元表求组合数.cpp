#include <iostream>

using namespace std;

using LL = long long;

const int N = 5e6 + 10;
LL f[N]; // 阶乘表 f[i] = i! % p
LL g[N]; // 阶乘逆元表 g[i] = (i!)^(-1) % p

// 快速幂
LL qpow(LL a, LL x, LL p)
{
    LL ret = 1;
    while (x)
    {
        if (x & 1) 
        {
            ret = (ret * a) % p;
        }
        x >>= 1;
        a = (a * a) % p;
    }
    return ret;
}

// 预处理阶乘及其逆元
// 核心：逆元递推公式 g[i] = g[i+1] * (i+1) % p，实现 O(N) 预处理
// 前置条件：p 必须为质数，且 limit < N
void init_fact(int limit, LL p)
{
    f[0] = 1;
    for (int i = 1; i <= limit; i++)
    {
        f[i] = (f[i - 1] * i) % p;
    }
    
    // 费马小定理求 limit! 的乘法逆元
    g[limit] = qpow(f[limit], p - 2, p);
    
    // 线性倒推预处理所有逆元
    for (int i = limit - 1; i >= 0; i--)
    {
        g[i] = (g[i + 1] * (i + 1)) % p; 
    }
}

// O(1) 查询组合数 C(n, m) % p
LL get_C(int n, int m, LL p)
{
    if (n < m || m < 0) 
    {
        return 0;
    }
    return (((f[n] * g[n - m]) % p) * g[m]) % p;
}