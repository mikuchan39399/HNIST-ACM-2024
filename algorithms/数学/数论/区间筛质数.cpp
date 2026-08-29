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
    vector<char> st(lim + 1, 0);
    vector<LL> p;
    p.reserve(lim / log(lim) + 10); 
    for (LL i = 2; i <= lim; i++) 
    {
        if (!st[i]) p.push_back(i);
        for (int j = 0; j < (int)p.size() && p[j] * i <= lim; j++) 
        {
            st[p[j] * i] = 1;
            if (i % p[j] == 0) break;
        }
    }
    LL len = r - l + 1;
    vector<char> ret(len, 0);
    for (LL x : p) 
    {
        LL start = max(x * 2, (l + x - 1) / x * x);
        for (LL j = start; j <= r; j += x) 
            ret[j - l] = 1; 
    }
    vector<LL> res;
    res.reserve(len / log(l + 2) + 10); 
    for (LL i = 0; i < len; i++) 
        if (!ret[i]) res.push_back(i + l);
    return res;
}