// zoi: segSieve
#ifndef Z_OI_SEG_SIEVE
#define Z_OI_SEG_SIEVE

#include "整数开方.cpp"
#include "埃氏筛.cpp"

// 返回闭区间 [l,r] 内的全部质数, 按从小到大排列; 空区间返回空 vector
// 时间: O((sqrt(r)+r-l+1) log log r) | 空间: O(sqrt(r)+r-l+1)
// sqrt(r)<INT_MAX 且两张表能放下; 位标记约 (sqrt(r)+r-l+2)/8 B, 基础质数4B/数, 返回8B/数
// 大 r 短区间仍需筛到 sqrt(r); 不适合直接处理 LLONG_MAX 附近的区间
inline vector<LL> segmented_sieve(LL l, LL r)
{
    l = max(l, 2LL);
    if (l > r) return {};
    LL lim = floor_isqrt(r);
    assert(lim < INT_MAX);
    Eratosthenes base((int)lim);
    LL len = r - l + 1;
    vector<bool> ret(len, false);
    for (LL x : base.primes)
    {
        LL offset = (x - l % x) % x;
        if (offset > r - l) continue;
        LL start = max(x * x, l + offset);
        for (LL j = start; j <= r;)
        {
            ret[j - l] = 1;
            if (r - j < x) break;
            j += x;
        }
    }
    vector<LL> res;
    for (LL i = 0; i < len; i++)
        if (!ret[i]) res.push_back(i + l);
    return res;
}
#endif
/* Usage
#include <segSieve.h>
int main()
{
    for (LL p : segmented_sieve(90, 110)) cout << p << ' '; // 97 101 103 107 109
    cout << '\n';
    cout << segmented_sieve(-10, 1).size() << '\n'; // 0, 自动排除小于 2 的数
    cout << segmented_sieve(5, 4).size() << '\n';   // 0, 空区间
}
*/
