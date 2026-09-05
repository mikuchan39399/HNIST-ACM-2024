// zoi: intSqrt
#ifndef Z_OI_INT_SQRT
#define Z_OI_INT_SQRT

#include "../../杂项/utils/utils.cpp"

// 返回 sqrt(x) 向下取整的整数; x <= 0 时返回 0
// 时间: O(1) | 空间: O(1)
LL floor_isqrt(LL x)
{
    if (x <= 0) return 0;
    LL r = sqrt(x);
    while (r + 1 <= x / (r + 1)) r++;
    while (r > x / r) r--;
    return r;
}
// 返回 sqrt(x) 向上取整的整数; x <= 0 时返回 0
// 时间: O(1) | 空间: O(1)
LL ceil_isqrt(LL x)
{
    if (x <= 0) return 0;
    LL r = floor_isqrt(x);
    return r + (r * r != x);
}
#endif
/*
 * Usage:
 * floor_isqrt(10); // 返回 3
 * ceil_isqrt(10);  // 返回 4
 */
