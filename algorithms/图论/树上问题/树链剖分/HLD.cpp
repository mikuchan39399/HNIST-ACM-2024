// zoi: hld
#ifndef Z_OI_HLD
#define Z_OI_HLD

#include "../../图的存储/Graph.cpp"
#include "../../../数据结构/线段树/泛型线段树.cpp"
#include "../../../杂项/utils/utils.cpp"

// 对 1-based 无向森林作重链剖分, 不读取边权; fa 为父点, dep 为深度, sz 为子树大小
// son 为重儿子, top 为链顶, dfn/seg 为重链优先序的正反表, 子树 u 对应 [dfn[u], dfn[u] + sz[u] - 1]
// 线段树外置并按 dfn 序存点信息, 路径操作要求两点已建表且同树, 包含两端点与 LCA
// query_path 要求 Info 合并可交换, 不保留 u 到 v 的顺序; 子树查询按 dfn 升序合并
// 剖链表每点 28 B; 配套线段树约 4n * (sizeof(Info) + sizeof(Tag)) B
// Info 为 16 B, Tag 为 8 B 时合计约 124n B, n = 1e5 时约 12.4 MB, 递归栈另计
struct HLD
{
    int n, dfn_idx;
    VI fa, dep, sz, son, top, dfn, seg;
    // 分配 max_n 个点的剖链表, 首次可直接 build
    // 时间 O(max_n) | 空间 O(max_n)
    HLD(int max_n = 0) : n(max_n), dfn_idx(0),
        fa(max_n + 10, 0), dep(max_n + 10, 0), sz(max_n + 10, 0),
        son(max_n + 10, 0), top(max_n + 10, 0),
        dfn(max_n + 10, 0), seg(max_n + 10, 0)
    {}
    // 清空剖链状态并设置本次点数 _n, _n 不超过构造容量
    // 时间 O(_n) | 额外空间 O(1)
    void init(int _n)
    {
        n = _n;
        dfn_idx = 0;
        z_fill_n(n, 0, fa, dep, sz, son, top, dfn, seg);
    }
    // 按 root 剖分所在树, -1 表示逐棵取最小编号点为根, 重建前先 init(n)
    // 时间 O(n) | 递归栈 O(h), h 为最大树高
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

// 对 u 到 v 路径上的所有点应用同一标记 k, 不按路径位置平移标记
// 时间 O(log^2 n) | 递归栈 O(log n), 按普通懒标记且单次代数操作 O(1) 计
template <class Info, class Tag>
void modify_path(HLD& h, SegTree<Info, Tag>& t, int u, int v, const Tag& k)
{
    while (h.top[u] != h.top[v])
    {
        if (h.dep[h.top[u]] < h.dep[h.top[v]]) swap(u, v);
        t.modify(h.dfn[h.top[u]], h.dfn[u], k);
        u = h.fa[h.top[u]];
    }
    if (h.dep[u] > h.dep[v]) swap(u, v);
    t.modify(h.dfn[u], h.dfn[v], k);
}

// 合并 u 到 v 路径上的点信息并返回 Info, Info{} 为单位元, 合并须可交换
// 时间 O(log^2 n) | 递归栈 O(log n), 单次代数操作按 O(1) 计
template <class Info, class Tag>
Info query_path(HLD& h, SegTree<Info, Tag>& t, int u, int v)
{
    Info res{};
    while (h.top[u] != h.top[v])
    {
        if (h.dep[h.top[u]] < h.dep[h.top[v]]) swap(u, v);
        res = res + t.query(h.dfn[h.top[u]], h.dfn[u]);
        u = h.fa[h.top[u]];
    }
    if (h.dep[u] > h.dep[v]) swap(u, v);
    res = res + t.query(h.dfn[u], h.dfn[v]);
    return res;
}

// 对 u 的子树内所有点应用标记 k, 包含 u, 子树范围由 build 时的根决定
// 时间 O(log n) | 递归栈 O(log n), 按普通懒标记且单次代数操作 O(1) 计
template <class Info, class Tag>
void modify_subtree(HLD& h, SegTree<Info, Tag>& t, int u, const Tag& k)
{
    t.modify(h.dfn[u], h.dfn[u] + h.sz[u] - 1, k);
}

// 按 dfn 升序合并 u 的子树点信息并返回 Info, 包含 u
// 时间 O(log n) | 递归栈 O(log n), 单次代数操作按 O(1) 计
template <class Info, class Tag>
Info query_subtree(HLD& h, SegTree<Info, Tag>& t, int u)
{
    return t.query(h.dfn[u], h.dfn[u] + h.sz[u] - 1);
}
#endif
/* Usage
// 先准备 Info/Tag, 本例使用 线段树/泛型插件/区间加区间和.cpp 中的代数层
Graph<false> g(3, 2);
g.add(1, 2); g.add(2, 3);
HLD hld(3);
hld.build(g, 1);                         // 不指定根时扫描全部森林
VLL a{0, 2, 3, 5};
vector<Info> b(4);
for (int i = 1; i <= 3; i++) b[i] = {a[hld.seg[i]], 1}; // 原点权搬到 dfn 序
SegTree<Info, Tag> tr(3);
tr.build(b);
modify_path(hld, tr, 1, 3, {4});
cout << query_path(hld, tr, 1, 3).sum << '\n'; // 22
modify_subtree(hld, tr, 2, {-1});
cout << query_subtree(hld, tr, 2).sum << '\n';  // 14
// 多测先 g.clear(), hld.init(n), tr.init(n), 再加边、剖链、搬点权并建线段树
*/
