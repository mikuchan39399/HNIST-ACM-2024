// zoi: hldlca
#ifndef Z_OI_HLD_LCA
#define Z_OI_HLD_LCA

#include "../../../杂项/utils/utils.cpp"

#ifndef Z_OI_EMPTY
#define Z_OI_EMPTY
struct Empty {};
#endif

// 用重链跳转求 LCA, 输入为无向森林; fa 为父点, son 为重儿子, top 为链顶
// dep 为跳数深度, dis 为根到点的权和, sz 为子树大小, dfn 为重链优先序, rt 为所在树根
// 无权边按 1 计权; 每点约 36 B, max_n = 1e6 时约 36 MB
struct HLD_LCA
{
    int n, dfn_idx;
    VI fa, dep, sz, son, top, dfn, rt;
    VLL dis;
    // 分配 max_n 个点的查询表, 首次可直接 build
    // 时间 O(max_n) | 空间 O(max_n)
    HLD_LCA(int max_n = 0) : n(max_n), dfn_idx(0),
        fa(max_n + 10, 0), dep(max_n + 10, 0), sz(max_n + 10, 0),
        son(max_n + 10, 0), top(max_n + 10, 0), dfn(max_n + 10, 0),
        rt(max_n + 10, 0), dis(max_n + 10, 0)
    {}
    // 清空上次建树状态并设置本次点数 _n, _n 不超过构造容量
    // 时间 O(_n) | 额外空间 O(1)
    void init(int _n)
    {
        n = _n;
        dfn_idx = 0;
        z_fill_n(n, 0, fa, dep, sz, son, top, dfn, rt, dis);
    }
    // 按 root 建表, -1 表示每棵树取最小编号点为根, 指定根时仅处理所在树; 重建前先 init(n)
    // 时间 O(n) | 递归栈 O(h), h 为最大树高
    template <class G>
    void build(G& g, int root = -1)
    {
        if (root != -1)
        {
            dfs1(root, 0, root, g);
            dfs2(root, root, g);
            return;
        }
        for (int i = 1; i <= n; i++)
        {
            if (!dfn[i])
            {
                dfs1(i, 0, i, g);
                dfs2(i, i, g);
            }
        }
    }
    // 返回已建表的 u 与 v 的最近公共祖先, 不连通返回 -1
    // 时间 O(log n) | 空间 O(1)
    int lca(int u, int v)
    {
        if (rt[u] != rt[v]) return -1;
        if (u == v) return u;
        while (top[u] != top[v])
        {
            if (dep[top[u]] < dep[top[v]]) swap(u, v);
            u = fa[top[u]];
        }
        return dep[u] < dep[v] ? u : v;
    }
    // 返回整个 0-based 已建表点集 nodes 的最近公共祖先, 空集或跨树返回 -1
    // 时间 O(nodes.size() + log n) | 空间 O(1)
    int lca(const VI& nodes)
    {
        if (nodes.empty()) return -1;
        int min_node = nodes[0];
        int max_node = nodes[0];
        for (int i = 1; i < (int)nodes.size(); i++)
        {
            int u = nodes[i];
            if (dfn[u] < dfn[min_node]) min_node = u;
            if (dfn[u] > dfn[max_node]) max_node = u;
        }
        return lca(min_node, max_node);
    }
    // 返回已建表的 u 到 v 的路径权和, 不连通返回 -1; 负权下用 lca 判断连通性
    // 时间 O(log n) | 空间 O(1)
    LL dist(int u, int v)
    {
        int l = lca(u, v);
        if (l == -1) return -1;
        return dis[u] + dis[v] - 2 * dis[l];
    }
private:
    template <class G>
    void dfs1(int u, int f, int root, G& g)
    {
        fa[u] = f;
        rt[u] = root;
        dep[u] = dep[f] + 1;
        sz[u] = 1;
        son[u] = 0;
        for (auto& e : g[u])
        {
            int v = e.v;
            if (v == f) continue;
            if constexpr (is_same_v<decltype(e.w), Empty>)
            {
                dis[v] = dis[u] + 1;
            }
            else
            {
                dis[v] = dis[u] + e.w;      // 针对自定义边权请修改这里
            }
            dfs1(v, u, root, g);
            sz[u] += sz[v];
            if (sz[v] > sz[son[u]]) son[u] = v;
        }
    }
    template <class G>
    void dfs2(int u, int t, G& g)
    {
        top[u] = t;
        dfn[u] = ++dfn_idx;
        if (son[u]) dfs2(son[u], t, g);
        for (auto& e : g[u])
        {
            int v = e.v;
            if (v == fa[u] || v == son[u]) continue;
            dfs2(v, v, g);
        }
    }
};
#endif

/*
 * Usage:
 * // 内存严苛场景轻量版; 需 jump / fa[k] 祖先表请用 DFN_LCA
 * Graph<false> g(n, n - 1);      // 带边权: Graph<false, LL> + g.add(u, v, w)
 * HLD_LCA lca(n);
 * for (int i = 1; i < n; i++) { int u, v; cin >> u >> v; g.add(u, v); }
 * lca.build(g);                  // 指定根: lca.build(g, root)
 * lca.lca(u, v);                 // LCA; 不连通 -1; 多点: lca.lca({u, v, w})
 * lca.dist(u, v);                // 树上真实距离, 边权图自动按 w 累计
 * // 直读: dep 深度 | fa 父 | sz 子树 | son 重儿子 | top 链顶 | dfn 时间戳 | rt 所在根
 * // build 为递归 DFS, 深链依赖评测机栈宽(同 DFN_LCA)
 */
