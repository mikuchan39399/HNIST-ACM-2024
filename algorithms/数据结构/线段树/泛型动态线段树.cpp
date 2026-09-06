// zoi: dynamicSeg
#ifndef Z_OI_DYSEG
#define Z_OI_DYSEG

#include "../../杂项/utils/utils.cpp"

// 动态线段树维护 [1, n], 0 号为空指针; 虚区间由 Info{} 补 len 表示, 须对应初始零值区间
// root 为当前根, idx 为已用结点数; 查询和 find 下传标记也会开点, clear/build 后旧编号失效
// 每结点含两个 int、Info 和 Tag 并按类型对齐; 40 B/结点时预算 4e6 约 160 MB
// 预算按两次复位间累计开点计算, build 用 2n-1 个, 普通修改/查询/查找每次另计 O(log n)
// 区间非空且在 [1, n] 内; find 的 pred 必须等价于区间内存在满足条件的点, 不累积前缀
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
    // 设置值域上界 _n 并预留 _budget 个实结点, 初始全域取 Info 的零值
    // 时间: O(1) | 空间: O(_budget) 预留
    DySegTree(LL _n = 1e9, int _budget = 4000010) : n(_n), root(0), idx(0), budget(_budget)
    {
        tr.reserve(budget + 1);
        tr.push_back(Node{});
    }
    // 清空结点并恢复哨兵, 保留值域 n 和池容量
    // 时间: O(idx) | 空间: O(1)
    void clear()
    {
        root = idx = 0;
        tr.clear();
        tr.push_back(Node{});
    }
    // 清空旧树并把值域改为 [1, _n], _n >= 1, 保留池容量
    // 时间: O(idx) | 空间: O(1)
    void init(LL _n)
    {
        clear();
        n = _n;
    }
    // 把标记 v 应用到闭区间 [x, y], 单点传 x == y
    // 时间: 普通懒标记 O(log n), 势能修改按题分析 | 额外空间: 普通修改 O(log n)
    void modify(LL x, LL y, const Tag& v) { modify(root, 1, n, x, y, v); }
    // 返回闭区间 [x, y] 的 Info, 未开点区间按初始零值参与合并
    // 时间: O(log n) | 额外空间: O(log n)
    Info query(LL x, LL y) { return query(root, 1, n, x, y); }
    // 清空旧树并用 a[1..m] 建树, 值域改为 [1, m], m >= 1
    // 时间: O(idx + m) | 额外空间: O(m)
    void build(const vector<Info>& a) { init(a.size() - 1); build(root, 1, n, a); }
    // 返回 [start, n] 内满足 pred 的最左位置, 不存在或 start 越界返回 -1
    // 时间: O(log n) | 额外空间: O(log n)
    template<class Pred>
    LL find_first(LL start, Pred pred)
    {
        if (start < 1 || start > n) return -1;
        return find_first(root, 1, n, start, pred);
    }
    // 返回 [1, end] 内满足 pred 的最右位置, 不存在或 end 越界返回 -1
    // 时间: O(log n) | 额外空间: O(log n)
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
#endif

/* Usage: 区间加、区间和与最大值, 按题改 Info/Tag
struct Tag
{
    LL add = 0;
    void apply(const Tag& t) { add += t.add; }
    void clear() { add = 0; }
    bool has_tag() const { return add != 0; }
};
struct Info
{
    LL len = 0, sum = 0, mx = 0;
    bool break_cond(const Tag&) const { return false; }
    bool tag_cond(const Tag&) const { return true; }
    void apply(const Tag& t) { sum += len * t.add; mx += t.add; }
    friend Info operator+(const Info& a, const Info& b)
    {
        if (!a.len) return b;
        if (!b.len) return a;
        return {a.len + b.len, a.sum + b.sum, max(a.mx, b.mx)};
    }
};
int main()
{
    DySegTree<Info, Tag> seg(1000000000LL, 256);
    seg.modify(2, 4, {3});
    cout << seg.query(1, 5).sum << "\n"; // 9
    auto pred = [](const Info& v) { return v.mx > 0; };
    cout << seg.find_first(1, pred) << " " << seg.find_last(5, pred) << "\n"; // 2 4
    seg.clear();                     // 保留值域, 回到全零树
    seg.init(10);                    // 清空并改为 [1, 10]
    vector<Info> a = {{}, {1, 2, 2}, {1, -1, -1}, {1, 4, 4}};
    seg.build(a);                    // 自动清空并改为 [1, 3]
    cout << seg.query(1, 3).sum << "\n"; // 5
}
*/
