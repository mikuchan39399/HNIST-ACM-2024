#include <iostream>

using namespace std;

using LL = long long;

const int N = 1e5 + 10; // N 需大于等于可能出现的最大模数 p
LL f[N]; 
LL g[N]; 

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

// 预处理模 p 意义下的阶乘与逆元
// 前置条件：p 必须为质数，且 p <= N
void init_lucas(int p)
{
    int up = p - 1;
    f[0] = 1;
    for (int i = 1; i <= up; i++)
    {
        f[i] = (f[i - 1] * i) % p;
    }
    
    g[up] = qpow(f[up], up - 2, p);
    
    for (int i = up - 1; i >= 0; i--)
    {
        g[i] = (g[i + 1] * (i + 1)) % p;
    }
}

// 计算局部的组合数 C(n, m) % p (n, m < p)
LL get_C(int n, int m, int p)
{
    if (n < m || m < 0) 
    {
        return 0;
    }
    return f[n] * g[m] % p * g[n - m] % p;
}

// 卢卡斯定理核心递归
// 核心机制：C(n, m) ≡ C(n/p, m/p) * C(n%p, m%p) (mod p)
// 适用场景：n, m 极大（如 1e18 级别），但模数 p 较小（如 <= 1e5）且必须为质数
LL lucas(LL n, LL m, int p)
{
    if (m == 0) 
    {
        return 1;
    }
    return (lucas(n / p, m / p, p) * get_C(n % p, m % p, p)) % p;
}