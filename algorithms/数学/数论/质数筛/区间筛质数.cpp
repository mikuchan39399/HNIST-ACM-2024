// zoi: segSieve
#ifndef Z_OI_SEG_SIEVE
#define Z_OI_SEG_SIEVE

#include "../../../杂项/utils/utils.cpp"
#include "埃氏筛.cpp"

// 返回 [l,r] 的升序质数, 空区间为空; 需容纳 sqrt(r) 及区间长度两张表, sqrt(r)<INT_MAX
// 时间 O((sqrt(r)+r-l+1)log log r), 空间 O(sqrt(r)+r-l+1)
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
