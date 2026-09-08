// zoi: divCeilFloor
#ifndef Z_OI_DIV_CEIL_FLOOR
#define Z_OI_DIV_CEIL_FLOOR

#include "../../杂项/utils/utils.cpp"

// 返回 a/b 向负无穷取整; b!=0 且不能是 LLONG_MIN/-1 (结果超出 LL)
// 时间: O(1) | 空间: O(1)
inline LL floor_div(LL a, LL b)
{
    assert(b != 0 && !(a == LLONG_MIN && b == -1));
    LL res = a / b;
    LL rem = a % b;
    if (rem != 0 && ((a < 0) ^ (b < 0)))
    {
        res--;
    }
    return res;
}

// 返回 a/b 向正无穷取整; 契约同 floor_div, 支持正负分子和分母
// 时间: O(1) | 空间: O(1)
inline LL ceil_div(LL a, LL b)
{
    assert(b != 0 && !(a == LLONG_MIN && b == -1));
    LL res = a / b;
    LL rem = a % b;
    if (rem != 0 && ((a > 0) == (b > 0)))
    {
        res++;
    }
    return res;
}
#endif

/* Usage
#include <divCeilFloor.h>
int main()
{
    cout << floor_div(-7, 3) << ' ' << ceil_div(-7, 3) << '\n'; // -3 -2
    cout << floor_div(7, -3) << ' ' << ceil_div(7, -3) << '\n'; // -3 -2
    cout << floor_div(-7, -3) << ' ' << ceil_div(-7, -3) << '\n'; // 2 3
    cout << ceil_div(LLONG_MAX, 2) << '\n'; // 4611686018427387904
    // 不用 (a+b-1)/b: 负数会取错方向, 大正数还可能在加法处溢出
}
*/
