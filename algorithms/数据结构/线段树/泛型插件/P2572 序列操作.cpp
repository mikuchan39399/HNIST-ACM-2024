#include <algorithm>
using namespace std;

using LL = long long;
const LL INF = 0x3f3f3f3f3f3f3f3f;

struct Tag
{
    LL ow = -1;
    int rev = 0;
    void apply(const Tag& t) 
    { 
        if (t.ow != -1)
        {
            ow = t.ow;
            rev = 0;
        }
        else
        {
            if (ow != -1) ow ^= 1;
            else rev ^= 1;
        }
    }
    void clear() 
    { 
        ow = -1;
        rev = 0;
    }
    bool has_tag() 
    { 
        return ow != -1 || rev != 0; 
    }
};
struct Info
{
    LL sum0 = 0, sum1 = 0;
    LL pre0 = 0, suf0 = 0;
    LL pre1 = 0, suf1 = 0;
    LL mx0 = 0, mx1 = 0;
    int len = 0;
    bool break_cond(const Tag&)
    { 
        return false; 
    }
    bool tag_cond(const Tag&)
    { 
        return true; 
    }
    void apply(const Tag& t) 
    {
        if (t.ow != -1)
        {
            if (t.ow == 0)
            {
                sum0 = pre0 = suf0 = mx0 = len;
                sum1 = pre1 = suf1 = mx1 = 0;
            }
            else
            {
                sum1 = pre1 = suf1 = mx1 = len;
                sum0 = pre0 = suf0 = mx0 = 0;
            }
        }
        else
        {
            swap(sum0, sum1); swap(pre0, pre1);
            swap(suf0, suf1); swap(mx0, mx1);
        }
    }
    friend Info operator+(const Info& a, const Info& b)
    {
        Info c;
        c.sum0 = a.sum0 + b.sum0;
        c.sum1 = a.sum1 + b.sum1;
        c.len = a.len + b.len;
        c.pre0 = a.pre0 + (a.pre0 == a.len ? b.pre0 : 0);
        c.pre1 = a.pre1 + (a.pre1 == a.len ? b.pre1 : 0);
        c.suf0 = b.suf0 + (b.suf0 == b.len ? a.suf0 : 0);
        c.suf1 = b.suf1 + (b.suf1 == b.len ? a.suf1 : 0);
        c.mx0 = max({a.mx0, b.mx0, a.suf0 + b.pre0});
        c.mx1 = max({a.mx1, b.mx1, a.suf1 + b.pre1});
        return c;
    }
};