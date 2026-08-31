// zoi: dynamicSeg
#ifndef Z_OI_DYSEG
#define Z_OI_DYSEG

#include <cassert>
#include <vector>
using namespace std;
using LL = long long;
// 操作数 * logN  -- M * logN
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
    int budget = 0;
    // 兼容 256MB 限制, 禁止中途扩容
    DySegTree(LL _n = 1e9, int _budget = 4000010) : n(_n), root(0), idx(0), budget(_budget)
    {
        tr.reserve(budget + 1);
        tr.push_back(Node{});
    }
    // 对外接口
    void modify(LL x, LL y, const Tag& v) { modify(root, 1, n, x, y, v); }
    Info query(LL x, LL y) { return query(root, 1, n, x, y); }
    void build(const vector<Info>& a) { n = a.size() - 1; build(root, 1, n, a); }
    template<class Pred>
    LL find_first(LL start, Pred pred)
    {
        if (start < 1 || start > n) return -1;
        return find_first(root, 1, n, start, pred);
    }
    template<class Pred>
    LL find_last(LL end, Pred pred)
    {
        if (end < 1 || end > n) return -1;
        return find_last(root, 1, n, end, pred);
    }
private:
    int new_node(LL len)
    {
        assert(idx + 1 <= budget && "max_nodes 开小了");
        tr.push_back(Node{});
        ++idx;
        tr[idx].info.len = len;
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
        if (!tr[p].lc) tr[p].lc = new_node(mid - l + 1);
        if (!tr[p].rc) tr[p].rc = new_node(r - mid);
        if constexpr (requires { tr[p].info.split_tag(tr[p].tag, tr[tr[p].lc].info, tr[tr[p].rc].info); })
        {
            auto [tl, tr_tag] = tr[p].info.split_tag(tr[p].tag, tr[tr[p].lc].info, tr[tr[p].rc].info);
            lazy(tr[p].lc, l, mid, tl);
            lazy(tr[p].rc, mid + 1, r, tr_tag);
        }
        else
        {
            lazy(tr[p].lc, l, mid, tr[p].tag);
            lazy(tr[p].rc, mid + 1, r, tr[p].tag);
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
    template<class Pred>
    LL find_first(int p, LL l, LL r, LL start, Pred pred)
    {
        if (r < start || !pred(get_info(p, l, r))) return -1;
        if (l == r) return l;
        if (p) pushdown(p, l, r);
        LL mid = l + (r - l) / 2;
        LL res = find_first(tr[p].lc, l, mid, start, pred);
        if (res == -1)
            res = find_first(tr[p].rc, mid + 1, r, start, pred);
        return res;
    }
    template<class Pred>
    LL find_last(int p, LL l, LL r, LL end, Pred pred)
    {
        if (l > end || !pred(get_info(p, l, r))) return -1;
        if (l == r) return l;
        if (p) pushdown(p, l, r);
        LL mid = l + (r - l) / 2;
        LL res = find_last(tr[p].rc, mid + 1, r, end, pred);
        if (res == -1)
            res = find_last(tr[p].lc, l, mid, end, pred);
        return res;
    }
};

// 区间加法区间和 (示例 Info/Tag, 守卫隔离; 与 泛型线段树.cpp 示例共守卫, 同场先到者胜)
#endif

#ifndef Z_OI_SEG_DEMO
#define Z_OI_SEG_DEMO
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
#endif
