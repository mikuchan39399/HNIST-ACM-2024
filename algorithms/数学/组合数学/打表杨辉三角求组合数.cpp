#include <iostream>

using namespace std;

const int N = 2010;
int C[N][N];

// 杨辉三角递推预处理组合数 C(n, m) % k
// 核心：C(n, m) = C(n-1, m-1) + C(n-1, m)
// 适用场景：n, m 较小（通常 <= 5000），需要 O(1) 频繁查询，且模数 k 即使非质数也适用
void init_C(int limit, int k)
{
    for (int i = 0; i <= limit; i++)
    {
        // C(i, 0) = 1
        C[i][0] = 1; 
        
        for (int j = 1; j <= i; j++)
        {
            C[i][j] = (C[i - 1][j - 1] + C[i - 1][j]) % k;
        }
    }
}