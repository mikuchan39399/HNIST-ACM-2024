// zoi: hld
#ifndef Z_OI_HLD
#define Z_OI_HLD

#include <algorithm>
#include "../../图的存储/Graph.cpp"
#include "../../../杂项/utils/utils.cpp"

using namespace std;

// ============ HLD 重链剖分 + 树链线段树 ============
// 1-based, 与 图的存储/Graph.cpp 配套(无向加边, g[u] 邻接迭代)
// 功能: 两遍 dfs 剖链 O(n); 路径/子树 加与求和 O(log^2 n)/O(log n)
// 约定: 点编号 [1, n]; build(g) 扫全森林(dfn==0 起剖), build(g, root)
//   单树; SegTree 按剖出的 dfn 序建树: build(1, 1, n, a, hld.seg),
//   a 为 1-based 原点权, seg 为 dfn->原编号映射
// 内存账: HLD 七个 int 数组 28B/n + SegTree 结点 24B × 4n ≈ 124B/n,
//   n = 1e5 约 13MB
// 注意: dfs1/dfs2 为递归, 深链退化 O(n) 栈深, 小栈环境需改迭代
struct SegTree
{
    struct Node
    {
        int l, r;
        LL add, sum;
    };
    vector<Node> tr;
    // 构造: 预算 max_n 规模的 4n 结点
    // 时间: O(n) | 空间: 24B × 4n (账目见类头)
    SegTree(int max_n = 0) :
        tr((max_n + 10) << 2, {0, 0, 0, 0})
    {}
    // 从 a[1..n] 按 dfn->seg 映射建树 (a 1-based 原点权, seg 为 hld.seg)
    // 时间: O(n) | 空间: O(1)
    void build(int p, int l, int r, const VI& a, const VI& seg)
    {
        tr[p] = {l, r, 0, 0};
        if (l == r)
        {
            tr[p].sum = a[seg[l]];
            return;
        }
        int mid = (l + r) >> 1;
        build(p << 1, l, mid, a, seg);
        build(p << 1 | 1, mid + 1, r, a, seg);
        push_up(p);
    }
    // 区间 [x, y] 整体加 k
    // 时间: O(log n) | 空间: O(1)
    void modify(int p, int x, int y, LL k)
    {
        int l = tr[p].l, r = tr[p].r;
        if (l >= x && r <= y)
        {
            lazy(p, k);
            return;
        }
        push_down(p);
        int mid = (l + r) >> 1;
        if (x <= mid) modify(p << 1, x, y, k);
        if (y > mid) modify(p << 1 | 1, x, y, k);
        push_up(p);
    }
    // 区间 [x, y] 求和
    // 时间: O(log n) | 空间: O(1)
    LL query(int p, int x, int y)
    {
        int l = tr[p].l, r = tr[p].r;
        if (l >= x && r <= y) return tr[p].sum;
        push_down(p);
        LL sum = 0;
        int mid = (l + r) >> 1;
        if (x <= mid) sum += query(p << 1, x, y);
        if (y > mid) sum += query(p << 1 | 1, x, y);
        return sum;
    }
private:
    void push_up(int p)
    {
        tr[p].sum = tr[p << 1].sum + tr[p << 1 | 1].sum;
    }
    void lazy(int p, LL add)
    {
        tr[p].add += add;
        tr[p].sum += add * (tr[p].r - tr[p].l + 1);
    }
    void push_down(int p)
    {
        if (tr[p].add == 0 || tr[p].l == tr[p].r) return;
        lazy(p << 1, tr[p].add);
        lazy(p << 1 | 1, tr[p].add);
        tr[p].add = 0;
    }
};

struct HLD
{
    int n, dfn_idx;
    VI fa, dep, sz, son, top, dfn, seg;
    // 构造: 预算 max_n 的七个剖链数组
    // 时间: O(n) | 空间: 28B/n (账目见类头)
    HLD(int max_n = 0) : n(max_n), dfn_idx(0),
        fa(max_n + 10, 0), dep(max_n + 10, 0), sz(max_n + 10, 0),
        son(max_n + 10, 0), top(max_n + 10, 0),
        dfn(max_n + 10, 0), seg(max_n + 10, 0)
    {}
    // 多测复位: 七个数组清零, n 更新为 _n
    // 时间: O(n) | 空间: O(1)
    void init(int _n)
    {
        n = _n;
        dfn_idx = 0;
        z_fill_n(n, 0, fa, dep, sz, son, top, dfn, seg);
    }
public:
    // 跑剖链: root 指定则单树, 默认扫全森林(dfn==0 起剖)
    // 时间: O(n) | 空间: O(1)
    template <class G>
    void build(G& g, int root = -1)
    {
        if (root != -1)
        {
            dfs1(root, 0, g);
            dfs2(root, root, g);
            return;
        }
        for (int i = 1; i <= n; i++)
        {
            if (!dfn[i])
            {
                dfs1(i, 0, g);
                dfs2(i, i, g);
            }
        }
    }
private:
    template <class G>
    void dfs1(int u, int f, G& g)
    {
        fa[u] = f;
        dep[u] = dep[f] + 1;
        sz[u] = 1;
        son[u] = 0;
        for (auto& e : g[u])
        {
            int v = e.v;
            if (v == f) continue;
            dfs1(v, u, g);
            sz[u] += sz[v];
            if (sz[v] > sz[son[u]]) son[u] = v;
        }
    }
    template <class G>
    void dfs2(int u, int t, G& g)
    {
        top[u] = t;
        dfn[u] = ++dfn_idx;
        seg[dfn_idx] = u;
        if (son[u]) dfs2(son[u], t, g);
        for (auto& e : g[u])
        {
            int v = e.v;
            if (v == fa[u] || v == son[u]) continue;
            dfs2(v, v, g);
        }
    }
};

// 树上 u-v 路径整体加 k
// 时间: O(log^2 n) | 空间: O(1)
void modify_path(HLD& h, SegTree& t, int u, int v, LL k)
{
    while (h.top[u] != h.top[v])
    {
        if (h.dep[h.top[u]] < h.dep[h.top[v]]) swap(u, v);
        t.modify(1, h.dfn[h.top[u]], h.dfn[u], k);
        u = h.fa[h.top[u]];
    }
    if (h.dep[u] > h.dep[v]) swap(u, v);
    t.modify(1, h.dfn[u], h.dfn[v], k);
}

// 树上 u-v 路径和
// 时间: O(log^2 n) | 空间: O(1)
LL query_path(HLD& h, SegTree& t, int u, int v)
{
    LL res = 0;
    while (h.top[u] != h.top[v])
    {
        if (h.dep[h.top[u]] < h.dep[h.top[v]]) swap(u, v);
        res += t.query(1, h.dfn[h.top[u]], h.dfn[u]);
        u = h.fa[h.top[u]];
    }
    if (h.dep[u] > h.dep[v]) swap(u, v);
    res += t.query(1, h.dfn[u], h.dfn[v]);
    return res;
}

// u 的子树整体加 k
// 时间: O(log n) | 空间: O(1)
void modify_subtree(HLD& h, SegTree& t, int u, LL k)
{
    t.modify(1, h.dfn[u], h.dfn[u] + h.sz[u] - 1, k);
}

// u 的子树和
// 时间: O(log n) | 空间: O(1)
LL query_subtree(HLD& h, SegTree& t, int u)
{
    return t.query(1, h.dfn[u], h.dfn[u] + h.sz[u] - 1);
}
#endif
/* Usage:
    int n, m; cin >> n >> m;
    Graph<false, Empty> g{n, n};
    HLD hld{n};
    SegTree tr{n};
    VI a(n + 1);                     // 1-based 原点权
    hld.init(n);
    for (int i = 1; i <= n; i++) cin >> a[i];
    for (int i = 1; i < n; i++) { int u, v; cin >> u >> v; g.add(u, v); }
    hld.build(g);                    // 剖链(森林全扫; 单树传 hld.build(g, root))
    tr.build(1, 1, n, a, hld.seg);   // 按 dfn 序建线段树
    modify_path(hld, tr, u, v, k);   // 路径加
    query_path(hld, tr, u, v);       // 路径和
    modify_subtree(hld, tr, u, k);   // 子树加
    query_subtree(hld, tr, u);       // 子树和
    // 多测: g.clear(); hld.init(n); 重跑 build
*/
