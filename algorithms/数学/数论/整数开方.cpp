// zoi: intSqrt
#ifndef Z_OI_INT_SQRT
#define Z_OI_INT_SQRT

#include "../../杂项/utils/utils.cpp"

// 返回 sqrt(x) 向下取整的整数; x <= 0 时返回 0
// 时间: O(1) | 空间: O(1)
inline LL floor_isqrt(LL x)
{
    if (x <= 0) return 0;
    LL r = sqrt(x);
    while (r + 1 <= x / (r + 1)) r++;
    while (r > x / r) r--;
    return r;
}
// 返回 sqrt(x) 向上取整的整数; x <= 0 时返回 0
// 时间: O(1) | 空间: O(1)
inline LL ceil_isqrt(LL x)
{
    if (x <= 0) return 0;
    LL r = floor_isqrt(x);
    return r + (r * r != x);
}
#endif
/* Usage
#include <intSqrt.h>
int main()
{
    for (LL x : {0LL, 9LL, 10LL, LLONG_MAX})
        cout << floor_isqrt(x) << ' ' << ceil_isqrt(x) << '\n';
    // 0 0; 3 3; 3 4; 3037000499 3037000500
    // 直接强转 sqrt(x) 在平方数附近可能舍入错, 接口会用整数除法校正
}
*/
