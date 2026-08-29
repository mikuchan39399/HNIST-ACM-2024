#include <algorithm>
using namespace std;

using LL = long long;
const LL INF = 0x3f3f3f3f3f3f3f3f;

// |======================================================|
// 区间最值
struct Tag
{
    LL max_lmt = INF;
    void apply(const Tag& t) 
    { 
        max_lmt = min(max_lmt, t.max_lmt);   
    }
    void clear() 
    { 
        max_lmt = INF;
    }
    bool has_tag() 
    { 
        return max_lmt != INF;
    }
};
struct Info
{
    LL mx = -INF;
    LL se = -INF;
    LL mx_cnt = 0;
    LL sum = 0;
    bool break_cond(const Tag& t) 
    { 
        return t.max_lmt >= mx; 
    }
    bool tag_cond(const Tag& t) 
    { 
        return t.max_lmt > se;
    }
    void apply(const Tag& t) 
    {   
        sum -= (mx - t.max_lmt) * mx_cnt;
        mx = t.max_lmt;
    }
    friend Info operator+(const Info& a, const Info& b)
    {
        Info c;
        c.sum = a.sum + b.sum;
        if (a.mx == b.mx)
        {
            c.mx = a.mx;
            c.se = max(a.se, b.se);
            c.mx_cnt = a.mx_cnt + b.mx_cnt;
        }
        else if (a.mx > b.mx)
        {
            c.mx = a.mx;
            c.se = max(a.se, b.mx);
            c.mx_cnt = a.mx_cnt;
        }
        else
        {
            c.mx = b.mx;
            c.se = max(a.mx, b.se);
            c.mx_cnt = b.mx_cnt;
        }
        return c;
    }
};

// |=======================================================================================|
// 区间加法 + 区间最值 (SegTree<InfoA, TagA> 直接组装)
struct TagA
{
    LL add = 0;
    LL max_lmt = INF;
    void apply(const TagA& t)
    {
        if (max_lmt != INF) max_lmt += t.add;
        max_lmt = min(max_lmt, t.max_lmt);
        add += t.add;
    }
    void clear()
    {
        add = 0;
        max_lmt = INF;
    }
    bool has_tag()
    {
        return add != 0 || max_lmt != INF;
    }
};
struct InfoA
{
    LL len = 0;
    LL mx = -INF;
    LL se = -INF;
    LL mx_cnt = 0;
    LL sum = 0;
    bool break_cond(const TagA& t)
    {
        if (t.max_lmt == INF) return false;
        return t.max_lmt >= mx;
    }
    bool tag_cond(const TagA& t)
    {
        return t.max_lmt > se;
    }
    void apply(const TagA& t)
    {
        if (t.add != 0)
        {
            sum += len * t.add;
            mx += t.add;
            if (se != -INF) se += t.add;
        }
        if (t.max_lmt < mx)
        {
            sum -= (mx - t.max_lmt) * mx_cnt;
            mx = t.max_lmt;
        }
    }
    friend InfoA operator+(const InfoA& a, const InfoA& b)
    {
        InfoA c;
        if (a.len == 0) return b;
        if (b.len == 0) return a;
        c.sum = a.sum + b.sum;
        c.len = a.len + b.len;
        if (a.mx == b.mx)
        {
            c.mx = a.mx;
            c.se = max(a.se, b.se);
            c.mx_cnt = a.mx_cnt + b.mx_cnt;
        }
        else if (a.mx > b.mx)
        {
            c.mx = a.mx;
            c.se = max(a.se, b.mx);
            c.mx_cnt = a.mx_cnt;
        }
        else
        {
            c.mx = b.mx;
            c.se = max(a.mx, b.se);
            c.mx_cnt = b.mx_cnt;
        }
        return c;
    }
};

// |=======================================================================================|
// 区间加法 + 区间最值 + 历史区间最值 (SegTree<InfoB, TagB> 直接组装)
struct TagB
{
    LL add = 0;
    LL max_lmt = INF;
    LL mx_add = 0;
    LL other_add = 0;
    LL pre_mx_addmx = 0;
    LL pre_other_addmx = 0;

    void apply(const TagB& t)
    {
        pre_mx_addmx = max(pre_mx_addmx, mx_add + t.pre_mx_addmx);
        pre_other_addmx = max(pre_other_addmx, other_add + t.pre_other_addmx);
        mx_add += t.mx_add;
        other_add += t.other_add;
    }
    void clear()
    {
        add = mx_add = other_add = pre_mx_addmx = pre_other_addmx = 0;
        max_lmt = INF;
    }
    bool has_tag() const
    {
        return mx_add != 0 || other_add != 0 || pre_mx_addmx != 0 || pre_other_addmx != 0;
    }
};

struct InfoB
{
    LL len = 0;
    LL mx = -INF;
    LL mx_hist = -INF;
    LL se = -INF;
    LL mx_cnt = 0;
    LL sum = 0;
    bool break_cond(const TagB& t) const
    {
        if (t.max_lmt == INF) return false;
        return t.max_lmt >= mx;
    }
    bool tag_cond(const TagB& t) const
    {
        if (t.max_lmt == INF) return true;
        return t.max_lmt > se;
    }
    // 萃取懒标记含义, 用于复杂的初始化懒标记场景
    TagB get_real_tag(const TagB& ext) const
    {
        TagB res;
        if (ext.max_lmt != INF) // 区间最值
        {
            LL k = ext.max_lmt - mx;
            res.mx_add = k;
            res.pre_mx_addmx = k;
        }
        else // 区间加法
        {
            res.mx_add = res.other_add = ext.add;
            res.pre_mx_addmx = res.pre_other_addmx = ext.add;
        }
        return res;
    }
    // 子区间下放懒标记需求不同的分流机制
    pair<TagB, TagB> split_tag(const TagB& p_tag, const InfoB& lc_info, const InfoB& rc_info) const
    {
        LL mx_val = max(lc_info.mx, rc_info.mx);
        TagB tl = p_tag;
        TagB tr = p_tag;
        if (lc_info.mx != mx_val)
        {
            tl.mx_add = tl.other_add;
            tl.pre_mx_addmx = tl.pre_other_addmx;
        }
        if (rc_info.mx != mx_val)
        {
            tr.mx_add = tr.other_add;
            tr.pre_mx_addmx = tr.pre_other_addmx;
        }
        return {tl, tr};
    }
    void apply(const TagB& t)
    {
        sum += t.mx_add * mx_cnt + t.other_add * (len - mx_cnt);
        mx_hist = max(mx_hist, mx + t.pre_mx_addmx);
        mx += t.mx_add;
        if (se != -INF) se += t.other_add;
    }
    friend InfoB operator+(const InfoB& a, const InfoB& b)
    {
        if (a.len == 0) return b;
        if (b.len == 0) return a;
        InfoB c;
        c.sum = a.sum + b.sum;
        c.len = a.len + b.len;
        c.mx_hist = max(a.mx_hist, b.mx_hist);

        if (a.mx == b.mx)
        {
            c.mx = a.mx;
            c.se = max(a.se, b.se);
            c.mx_cnt = a.mx_cnt + b.mx_cnt;
        }
        else if (a.mx > b.mx)
        {
            c.mx = a.mx;
            c.se = max(a.se, b.mx);
            c.mx_cnt = a.mx_cnt;
        }
        else
        {
            c.mx = b.mx;
            c.se = max(a.mx, b.se);
            c.mx_cnt = b.mx_cnt;
        }
        return c;
    }
};