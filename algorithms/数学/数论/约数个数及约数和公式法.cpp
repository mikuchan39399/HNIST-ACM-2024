// zoi: divisorSums
#ifndef Z_OI_DIVISOR_SUMS
#define Z_OI_DIVISOR_SUMS

#include "../../杂项/utils/utils.cpp"

LL sum;
int cnt;
// 分解正整数 x, 把约数和写入 sum、约数个数写入 cnt; 每次调用覆盖旧结果
// 时间: O(sqrt(x)) | 空间: O(1)
void deprime(int x)
{
    sum = cnt = 1;
    for(int i = 2; i <= x / i; i++)
    {
        if(x % i == 0)
        {
            int a = 0; // 计算这个质数的次方
            while(x % i == 0)
            {
                a++;
                x /= i;
            }
            cnt *= a + 1;
            LL ret = 1;
            while(a--) // 秦九韶算法算多项式
            {
                ret = ret * i + 1;
            }
            sum *= ret;
        }
    }
    if(x > 1)
    {
        cnt *= 1 + 1;
        sum *= (LL)x + 1;
    }
}
#endif
/*
 * Usage:
 * deprime(12); // cnt = 6, sum = 28
 */
