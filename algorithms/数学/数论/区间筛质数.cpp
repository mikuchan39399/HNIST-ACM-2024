// zoi: segSieve
#ifndef Z_OI_SEG_SIEVE
#define Z_OI_SEG_SIEVE

#include "整数开方.cpp"

// 返回闭区间 [l,r] 内的全部质数, 按从小到大排列; 空区间返回空 vector
// 时间: O(sqrt(r) + (r-l+1) log log r) | 空间: O(sqrt(r) + r-l+1)
// 两张筛表约 sqrt(r)+r-l+2 字节, 另需质数表和返回数组各 8B/数
vector<LL> segmented_sieve(LL l, LL r)
{
    l = max(l, 2LL);
    if (l > r) return {};
    LL lim = floor_isqrt(r);
    vector<char> st(lim + 1, 0);
    vector<LL> p;
    for (LL i = 2; i <= lim; i++)
    {
        if (!st[i]) p.push_back(i);
        for (size_t j = 0; j < p.size() && p[j] <= lim / i; j++)
        {
            st[p[j] * i] = 1;
            if (i % p[j] == 0) break;
        }
    }
    LL len = r - l + 1;
    vector<char> ret(len, 0);
    for (LL x : p)
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
/*
 * Usage:
 * auto primes = segmented_sieve(2, 10); // 返回 {2,3,5,7}, 从下标 0 开始存
 */
