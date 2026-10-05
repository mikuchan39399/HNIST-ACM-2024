// zoi: hld
#ifndef Z_OI_HLD
#define Z_OI_HLD

#include "../../图的存储/Graph.cpp"
#include "../../../数据结构/线段树/泛型线段树.cpp"
#include "../../../杂项/utils/utils.cpp"

// 无向森林重链剖分, 1-based; fa/dep/sz/son/top 为父点/深度/子树大小/重儿子/链顶
// dfn/seg 为重链序正反表, 子树区间 [dfn[u],dfn[u]+sz[u]-1]. 空间 O(n)
// 线段树外置, 按 dfn 存点权; 路径两端须已建表且同树, 包含 LCA, Info 合并须可交换
struct HLD
{
    int n, dfn_idx;
    VI fa, dep, sz, son, top, dfn, seg;
    // 预留 max_n 个点. 时空 O(max_n)
    HLD(int max_n = 0) : n(max_n), dfn_idx(0),
        fa(max_n + 10, 0), dep(max_n + 10, 0), sz(max_n + 10, 0),
        son(max_n + 10, 0), top(max_n + 10, 0),
        dfn(max_n + 10, 0), seg(max_n + 10, 0)
    {}
    // 清空剖链状态, n<=容量. 时间 O(n)
    void init(int _n)
    {
        n = _n;
        dfn_idx = 0;
        z_fill_n(n, 0, fa, dep, sz, son, top, dfn, seg);
    }
    // 剖分并自动复位; root=-1 时逐树取最小点号为根, 否则仅处理所在树. 时间 O(n), 栈 O(树高)
    template <class G>
    void build(G& g, int _n, int root)
    {
        init(_n);
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

// 路径各点应用同一标记 k, 不按位置平移. 时间 O(log² n), 栈 O(log n)
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

// 合并路径点信息, Info{} 须为单位元. 时间 O(log² n), 栈 O(log n)
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

// 子树各点应用 k, 包含 u. 时间/栈 O(log n)
template <class Info, class Tag>
void modify_subtree(HLD& h, SegTree<Info, Tag>& t, int u, const Tag& k)
{
    t.modify(h.dfn[u], h.dfn[u] + h.sz[u] - 1, k);
}

// 子树点信息按 dfn 升序合并, 包含 u. 时间/栈 O(log n)
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
hld.build(g, 3, 1); // root=-1 时扫描全部森林
VLL a{0, 2, 3, 5};
vector<Info> b(4);
for (int i = 1; i <= 3; i++) b[i] = Info(a[hld.seg[i]]); // 原点权搬到 dfn 序
SegTree<Info, Tag> tr(3);
tr.build(b);
modify_path(hld, tr, 1, 3, {4});
cout << query_path(hld, tr, 1, 3).sum << '\n'; // 22
modify_subtree(hld, tr, 2, {-1});
cout << query_subtree(hld, tr, 2).sum << '\n'; // 14

*/
