// zoi: divisorSums
#ifndef Z_OI_DIVISOR_SUMS
#define Z_OI_DIVISOR_SUMS

#include "../../../杂项/utils/utils.cpp"

// 正 int x 的 {约数个数,约数和}, x=1 返回 {1,1}. 时间 O(sqrt(x)), 空间 O(1)
inline PIL deprime(int x)
{
    assert(x >= 1);
    LL sum = 1;
    int cnt = 1;
    for (int i = 2; i <= x / i; i += (i == 2 ? 1 : 2))
    {
        if (x % i == 0)
        {
            int a = 0;
            while (x % i == 0)
            {
                a++;
                x /= i;
            }
            cnt *= a + 1;
            LL ret = 1;
            while (a--)
            {
                ret = ret * i + 1;
            }
            sum *= ret;
        }
    }
    if (x > 1)
    {
        cnt *= 1 + 1;
        sum *= (LL)x + 1;
    }
    return {cnt, sum};
}
#endif
/* Usage
#include <divisorSums.h>
int main()
{
    auto [cnt, sum] = deprime(12);
    cout << cnt << ' ' << sum << '\n'; // 6 28
    auto one = deprime(1);
    cout << one.first << ' ' << one.second << '\n'; // 1 1
    cout << cnt << ' ' << sum << '\n'; // 6 28

}
*/
