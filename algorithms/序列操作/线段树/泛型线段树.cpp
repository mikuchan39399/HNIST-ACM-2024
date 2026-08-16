#include <vector>
#include <algorithm>

using namespace std;
using LL = long long;
const LL INF = 0x3f3f3f3f3f3f3f3f;

#define lc (p << 1)
#define rc (p << 1 | 1)
template<class Info, class Tag>
struct SegTree
{
    int n;
    vector<Info> info;
    vector<Tag> tag;
    SegTree(int max_n) : 
        n(max_n), info(4 * max_n + 10), tag(4 * max_n + 10) 
    {}
    void init(int _n)
    {
        n = _n;
        for (int i = 0; i <= 4 * n; i++)
        {
            info[i] = Info{};
            tag[i] = Tag{};
        }
    }
    void pushup(int p) { info[p] = info[lc] + info[rc]; }
    void lazy(int p, const Tag& v)
    {
        info[p].apply(v);
        tag[p].apply(v);
    }
    void pushdown(int p)
    {
        if (tag[p].has_tag())
        {
            if constexpr (requires { info[p].split_tag(tag[p], info[lc], info[rc]); })
            {
                auto [tagl, tagr] = info[p].split_tag(tag[p], info[lc], info[rc]);
                lazy(lc, tagl);
                lazy(rc, tagr);
            }
            else
            {
                lazy(lc, tag[p]);
                lazy(rc, tag[p]);
            }
            tag[p].clear();
        }
    }
    void build(int p, int l, int r, const vector<Info>& a)
    {
        if (l == r)
        {
            info[p] = a[l];
            info[p].len = 1;
            return;
        }
        int mid = (l + r) >> 1;
        build(lc, l, mid, a);
        build(rc, mid + 1, r, a);
        pushup(p);
    }
    void modify(int p, int l, int r, int x, int y, const Tag& v)
    {
        if (l > y || r < x || info[p].break_cond(v)) return;
        if (l >= x && r <= y && info[p].tag_cond(v))
        {
            if constexpr (requires { info[p].get_real_tag(v); })
            {
                lazy(p, info[p].get_real_tag(v));
            }
            else
            {
                lazy(p, v);
            }
            return;
        }
        pushdown(p);
        int mid = (l + r) >> 1;
        modify(lc, l, mid, x, y, v);
        modify(rc, mid + 1, r, x, y, v);
        pushup(p);
    }
    Info query(int p, int l, int r, int x, int y)
    {
        if (l > y || r < x) return Info{};
        if (l >= x && r <= y) return info[p];
        pushdown(p);
        int mid = (l + r) >> 1;
        return query(lc, l, mid, x, y) + query(rc, mid + 1, r, x, y);
    }
    template<class Pred>
    int find_first(int p, int l, int r, int start, Pred pred)
    {
        if (r < start || !pred(info[p])) return -1;
        if (l == r) return l;
        pushdown(p);
        int mid = (l + r) >> 1;
        int res = find_first(lc, l, mid, start, pred);
        if (res == -1)
            res = find_first(rc, mid + 1, r, start, pred);
        return res;
    }
    template<class Pred>
    int find_last(int p, int l, int r, int end, Pred pred)
    {
        if (l > end || !pred(info[p])) return -1;
        if (l == r) return l;
        pushdown(p);
        int mid = (l + r) >> 1;
        int res = find_last(rc, mid + 1, r, end, pred);
        if (res == -1)
            res = find_last(lc, l, mid, end, pred);
        return res;
    }
    // 对外接口
    void modify(int x, int y, const Tag& v) { modify(1, 1, n, x, y, v); }
    Info query(int x, int y) { return query(1, 1, n, x, y); }
    void build(const vector<Info>& a) { build(1, 1, n, a); }
    template<class Pred>
    int find_first(int start, Pred pred)
    {
        if (start < 1 || start > n) return -1;
        return find_first(1, 1, n, start, pred);
    }
    template<class Pred>
    int find_last(int end, Pred pred)
    {
        if (end < 1 || end > n) return -1;
        return find_last(1, 1, n, end, pred);
    }
};

// ==================== 示例 1：区间加法区间求和 ====================
struct Tag
{
    LL add = 0;
    void apply(const Tag& t) 
    { 
        add += t.add; 
    }
    void clear() 
    { 
        add = 0; 
    }
    bool has_tag() 
    { 
        return add != 0; 
    }
};

struct Info
{
    LL sum = 0;
    int len = 0;
    bool break_cond(const Tag& t) 
    { 
        return false; 
    }
    bool tag_cond(const Tag& t) 
    { 
        return true; 
    }
    void apply(const Tag& t) 
    { 
        sum += t.add * len; 
    }
    friend Info operator+(const Info& a, const Info& b)
    {
        if (a.len == 0) return b;
        if (b.len == 0) return a;
        Info c;
        c.sum = a.sum + b.sum;
        c.len = a.len + b.len;
        return c;
    }
};

// ==================== 示例 2：区间取模区间求和 ====================
/*
struct Tag
{
    LL op_mod = 0;
    void apply(const Tag& t)        // 只在叶子节点应用懒标记，永远不会叠加
    {}     
    void clear() 
    {}                              // 不下放就不会清空
    bool has_tag() const            // 永不下放懒标记
    { 
        return false; 
    }
};

struct Info
{
    LL sum = 0;
    LL max_val = 0;
    int len = 0;
    Info() {}
    bool break_cond(const Tag& t) const      // 区间内Info是否已无势能
    {
        return max_val < t.op_mod;
    }
    bool tag_cond(const Tag& t) const        // 区间是否能使用懒标记
    {
        return len == 1; 
    }
    void apply(const Tag& t)                 // 应用懒标记
    {
        max_val %= t.op_mod;
        sum = max_val;
    }
    friend Info operator+(const Info& a, const Info& b)
    {
        if (a.len == 0) return b;
        if (b.len == 0) return a;
        Info c;
        c.sum = a.sum + b.sum;
        c.max_val = max(a.max_val, b.max_val);
        c.len = a.len + b.len;
        return c;
    }
};
*/

/**
 * Usage:
 * vector<Info> init_info(n + 1);
 * for (int i = 1; i <= n; i++)
 * {
 *     init_info[i].sum = a[i];
 *     init_info[i].max_val = a[i]; 
 *     init_info[i].len = 1;
 * }
 * 
 * SegTree<Info, Tag> seg(n);
 * seg.build(init_info);
 * 
 * seg.modify(L, R, {k});
 * 
 * LL ans = seg.query(L, R).sum;
 */