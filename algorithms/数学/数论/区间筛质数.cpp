#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

using namespace std;
using LL = long long;

vector<LL> segmented_sieve(LL l, LL r) 
{
    if (l < 2) l = 2;     
    if (l > r) return {}; 

    LL lim = sqrt(r);
    // st: 记录 <= sqrt(r) 的合数，false为质数，true为合数
    vector<bool> st(lim + 1, false);
    vector<LL> p;
    
    for (LL i = 2; i <= lim; i++) 
    {
        if (!st[i]) 
        {
            p.push_back(i);
            for (LL j = i * i; j <= lim; j += i) 
            {
                st[j] = true;
            }
        }
    }

    LL len = r - l + 1;
    // ret: 记录 [l, r] 区间的合数，false为质数，true为合数
    vector<bool> ret(len, false);
    for (LL x : p) 
    {
        LL start = max(2LL, (l + x - 1) / x) * x;
        for (LL j = start; j <= r; j += x) 
        {
            ret[j - l] = true; 
        }
    }
    vector<LL> res;
    for (LL i = 0; i < len; i++) 
    {
        if (!ret[i]) 
        {
            res.push_back(i + l);
        }
    }
    return res;
}