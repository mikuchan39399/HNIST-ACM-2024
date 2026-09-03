// zoi: hld
#ifndef Z_OI_HLD
#define Z_OI_HLD

#include <algorithm>
#include "../../图的存储/Graph.cpp"
#include "../../../数据结构/线段树/泛型线段树.cpp"
#include "../../../杂项/utils/utils.cpp"

using namespace std;

// ============ HLD 重链剖分 + 泛型线段树 ============
// 1-based, 与 图的存储/Graph.cpp 配套(无向加边, g[u] 邻接迭代)
// 功能: 两遍 dfs 剖链 O(n); 路径/子树 加与查 O(log^2 n)/O(log n),
//   查 = 代数层 Info 合并, 换代数即换维护量(不止求和一种)
// 约定: 点编号 [1, n]; build(g) 扫全森林(dfn==0 起剖), build(g, root)
//   单树; 路径/子树函数要求操作点同树(跨分量无契约)
// 组装: 线段树按 dfn 序建, 原点权先搬进 b: for i in 1..n 有
//   b[i] = {a[hld.seg[i]], 1}; 代数层抄 线段树/泛型插件/
// 内存: 剖链数组 28B/n + 线段树 4n×(sizeof(Info)+sizeof(Tag));
//   加和代数共 124B/n; n=1e5 ≈ 13MB
// 注意: dfs1/dfs2 为递归, 深链退化 O(n) 栈深, 小栈环境需改迭代
struct HLD
{
    int n, dfn_idx;
    VI fa, dep, sz, son, top, dfn, seg;
    // 构造: 预算 max_n 的七个剖链数组
    // 时间: O(n) | 空间: 28B/n
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

// 树上 u-v 路径整体应用标记 k
// 时间: O(log^2 n) | 空间: O(1)
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

// 树上 u-v 路径信息合并, 返回 Info (Info{} 当单位元起步)
// 时间: O(log^2 n) | 空间: O(1)
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

// u 的子树整体应用标记 k
// 时间: O(log n) | 空间: O(1)
template <class Info, class Tag>
void modify_subtree(HLD& h, SegTree<Info, Tag>& t, int u, const Tag& k)
{
    t.modify(h.dfn[u], h.dfn[u] + h.sz[u] - 1, k);
}

// u 的子树信息合并, 返回 Info
// 时间: O(log n) | 空间: O(1)
template <class Info, class Tag>
Info query_subtree(HLD& h, SegTree<Info, Tag>& t, int u)
{
    return t.query(h.dfn[u], h.dfn[u] + h.sz[u] - 1);
}
#endif
/* Usage:
    int n, m; cin >> n >> m;
    Graph<false, Empty> g{n, n};
    HLD hld{n};
    SegTree<Info, Tag> tr{n};         // 代数层抄 线段树/泛型插件/区间加区间和.cpp
    VI a(n + 1);                      // 1-based 原点权
    hld.init(n);
    for (int i = 1; i <= n; i++) cin >> a[i];
    for (int i = 1; i < n; i++) { int u, v; cin >> u >> v; g.add(u, v); }
    hld.build(g);                     // 剖链(森林全扫; 单树传 hld.build(g, root))
    vector<Info> b(n + 1);
    for (int i = 1; i <= n; i++) b[i] = {a[hld.seg[i]], 1};   // 按 dfn 序搬点权
    tr.init(n);
    tr.build(b);                      // 按 dfn 序建线段树
    modify_path(hld, tr, u, v, {k});  // 路径加
    query_path(hld, tr, u, v).sum;    // 路径和(从 Info 取字段)
    modify_subtree(hld, tr, u, {k});  // 子树加
    query_subtree(hld, tr, u).sum;    // 子树和
    // 多测: g.clear(); hld.init(n); 重跑 build
*/
