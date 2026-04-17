#include <iostream>

using namespace std;

using LL = long long;

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
        a = (a * a) % p;
        x >>= 1;
    }
    return ret;
}

// 组合数 C(n, m) % p
// 核心：基于费马小定理求分母的乘法逆元
// 前置条件：p 必须为质数，且 p > m
LL get_C(LL n, LL m, LL p) 
{
    if (n < m || m < 0) 
    {
        return 0;
    }
    
    if (m > n - m) 
    {
        m = n - m;
    }

    LL up = 1;
    LL down = 1;
    
    for (LL i = 1; i <= m; i++) 
    {
        up = (up * (n - i + 1)) % p;
        down = (down * i) % p;
    }
    
    return up * qpow(down, p - 2, p) % p;
}