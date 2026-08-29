#include <algorithm>
#include <cmath>
using LL = long long;

LL floor_isqrt(LL x)
{
    if (x <= 0) return 0;
    LL r = sqrt(x);
    while ((r + 1) * (r + 1) <= x) r++;
    while (r * r > x) r--;
    return r;
}
LL ceil_isqrt(LL x)
{
    if (x <= 0) return 0;
    LL r = sqrt(x);
    while ((r - 1) * (r - 1) >= x) r--;
    while (r * r < x) r++;
    return r;
}