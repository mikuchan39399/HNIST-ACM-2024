#include <iostream>
using namespace std;

int sum, cnt;
// 利用约数个数及约数和定理
void deprime(int x) // 质因数分解
{
    sum = cnt = 1;
    for(int i = 2; i * i <= x; i++)
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
            int ret = 1;
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
        ret *= x + 1;
    }
}