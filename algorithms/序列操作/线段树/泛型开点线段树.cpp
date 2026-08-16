#include <cassert>
#include <vector>
using namespace std;

// 操作数 * log(值域大小)  -- M * logV
template<class Info, class Tag>
struct DySegTree
{
    struct Node 
    {
        int lc = 0, rc = 0;
        Info info;
        Tag tag;
    };
    LL n; 
    int root;
    int idx;
    vector<Node> tr;
    DySegTree(LL _n = 1e9, int max_nodes = 3000010) 
    {
        n = _n;
        root = 0;
        idx = 0;
        tr.resize(max_nodes);
    }
    void clear() 
    {
        for (int i = 0; i <= idx; i++) tr[i] = Node{};
        root = 0;
        idx = 0;
    }
    void set_range(LL _n) { n = _n; }
    int new_node(LL len) 
    {
        tr[++idx].info.len = len;
        return idx;
    }
    Info get_info(int p, LL l, LL r) 
    {
        if (p) return tr[p].info;
        Info ret;
        ret.len = r - l + 1;
        return ret;
    }
    void pushup(int p, LL l, LL r) 
    {
        LL mid = l + (r - l) / 2;
        tr[p].info = get_info(tr[p].lc, l, mid) + get_info(tr[p].rc, mid + 1, r);
    }
    void build(int& p, LL l, LL r, const vector<Info>& a) 
    {
        p = new_node(r - l + 1);
        if (l == r) 
        {
            tr[p].info = a[l];
            tr[p].info.len = 1;
            return;
        }
        LL mid = l + (r - l) / 2;
        build(tr[p].lc, l, mid, a);
        build(tr[p].rc, mid + 1, r, a);
        pushup(p, l, r);
    }
    void lazy(int& p, LL l, LL r, const Tag& v) 
    {
        if (!p) p = new_node(r - l + 1);
        tr[p].info.apply(v);
        tr[p].tag.apply(v);
    }
    void pushdown(int p, LL l, LL r) 
    {
        if (l == r || !tr[p].tag.has_tag()) return;
        LL mid = l + (r - l) / 2;
        int& lc = tr[p].lc;
        int& rc = tr[p].rc;
        if (!lc) lc = new_node(mid - l + 1);
        if (!rc) rc = new_node(r - mid);
        if constexpr (requires { tr[p].info.split_tag(tr[p].tag, tr[lc].info, tr[rc].info); }) 
        {
            auto [tl, tr_tag] = tr[p].info.split_tag(tr[p].tag, tr[lc].info, tr[rc].info);
            lazy(lc, l, mid, tl);
            lazy(rc, mid + 1, r, tr_tag);
        }
        else 
        {
            lazy(lc, l, mid, tr[p].tag);
            lazy(rc, mid + 1, r, tr[p].tag);
        }
        tr[p].tag.clear();
    }
    void modify(int& p, LL l, LL r, LL x, LL y, const Tag& v) 
    {
        if (l > y || r < x) return;
        if (!p) p = new_node(r - l + 1);
        if (tr[p].info.break_cond(v)) return;
        if (l >= x && r <= y && tr[p].info.tag_cond(v)) 
        {
            if constexpr (requires { tr[p].info.get_real_tag(v); }) 
                lazy(p, l, r, tr[p].info.get_real_tag(v));
            else 
                lazy(p, l, r, v);
            return;
        }
        pushdown(p, l, r);
        LL mid = l + (r - l) / 2;
        if (x <= mid) modify(tr[p].lc, l, mid, x, y, v);
        if (y > mid) modify(tr[p].rc, mid + 1, r, x, y, v);
        pushup(p, l, r);
    }
    Info query(int p, LL l, LL r, LL x, LL y) 
    {
        if (l > y || r < x) return Info{}; 
        if (l >= x && r <= y) return get_info(p, l, r);
        if (p) pushdown(p, l, r);
        LL mid = l + (r - l) / 2;
        return query(tr[p].lc, l, mid, x, y) + query(tr[p].rc, mid + 1, r, x, y);
    }
    // 对外接口
    void modify(LL x, LL y, const Tag& v) { modify(root, 1, n, x, y, v); }
    Info query(LL x, LL y) { return query(root, 1, n, x, y); }
    void build(const vector<Info>& a) { n = a.size() - 1; build(root, 1, n, a); }
};

// 区间加法区间和（未改动）
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
    bool has_tag() const 
    { 
        return add != 0; 
    }
};
struct Info
{
    LL len = 0;
    LL sum = 0;
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
        sum += len * t.add;
    }
    friend Info operator+(const Info& a, const Info& b)
    {
        if (a.len == 0) return b;
        if (b.len == 0) return a;
        Info c;
        c.len = a.len + b.len;
        c.sum = a.sum + b.sum;
        return c;
    }
};