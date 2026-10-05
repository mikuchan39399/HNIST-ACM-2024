// zoi: lca
#ifndef Z_OI_LCA
#define Z_OI_LCA

#include "../../../杂项/utils/utils.cpp"

#ifndef Z_OI_EMPTY
#define Z_OI_EMPTY
struct Empty {};
#endif

// 无向森林 LCA, 各树以最小点号为根; 无权边计 1, dep 为跳数, dis 为根路径权和
// rt 为树根, fa 为倍增祖先; dfn/rnk 为 DFS 序正反表, 子树区间 [dfn[u],dfn[u]+sz[u]-1]
struct LCA
{
    int n;
    int idx;
    int max_bit;
    int LOG;
    VI dep, dfn, rnk, rt, sz;
    VLL dis;
    VVI rmq, fa;
    // 预留 max_n 个点. 时空 O(max_n log max_n)
    LCA(int max_n = 0) : n(max_n), idx(0),
        max_bit(max_n == 0 ? 0 : __lg(max_n)),
        LOG(max_n <= 1 ? 2 : __lg(max_n) + 2),
        dep(max_n + 10, 0), dfn(max_n + 10, 0), rnk(max_n + 10, 0),
        rt(max_n + 10, 0), sz(max_n + 10, 0), dis(max_n + 10, 0)
    {
        rmq.assign(LOG, VI(max_n + 10, 0));
        fa.assign(LOG, VI(max_n + 10, 0));
    }
    // 清空状态, n<=容量. 时间 O(n)
    void init(int _n)
    {
        n = _n;
        idx = 0;
        max_bit = n == 0 ? 0 : __lg(n);
        z_fill_n(n, 0, dep, dfn, rt, sz, dis);
    }
    // 为 g[1..n] 建表, 自动复位. 时间 O(n log n), 栈 O(树高)
    template <class G>
    void build(G& g, int _n)
    {
        init(_n);
        for (int i = 1; i <= n; i++)
            if (!dfn[i]) dfs(i, 0, i, g);
        for (int k = 1; k <= max_bit; k++)
        {
            for (int i = 1; i + (1 << k) - 1 <= n; i++)
            {
                int u = rmq[k - 1][i];
                int v = rmq[k - 1][i + (1 << (k - 1))];
                rmq[k][i] = dep[u] < dep[v] ? u : v;
            }
        }
        for (int k = 1; k <= max_bit; k++)
            for (int i = 1; i <= n; i++)
                fa[k][i] = fa[k - 1][fa[k - 1][i]];
    }
    // 两点 LCA, 不连通返回 -1. O(1)
    int lca(int u, int v) const
    {
        if (rt[u] != rt[v]) return -1;
        if (u == v) return u;
        int l = dfn[u], r = dfn[v];
        if (l > r) swap(l, r);
        l++;
        int k = __lg(r - l + 1);
        int u_node = rmq[k][l];
        int v_node = rmq[k][r - (1 << k) + 1];
        int w = dep[u_node] < dep[v_node] ? u_node : v_node;
        return fa[0][w];
    }
    // 整个 nodes 的 LCA, 空集/跨树返回 -1. 时间 O(nodes.size())
    int lca(const VI& nodes) const
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
    // 路径权和, 不连通返回 -1; 负权时用 lca 判连通. O(1)
    LL dist(int u, int v) const
    {
        int l = lca(u, v);
        if (l == -1) return -1;
        return dis[u] + dis[v] - 2 * dis[l];
    }
    // u 向上跳 step 步, 0<=step<=dep[u], step=dep[u] 返回 0. 时间 O(log n)
    int jump_up(int u, int step) const
    {
        for (int i = 0; step > 0 && u > 0; i++, step >>= 1)
        {
            if (step & 1) u = fa[i][u];
        }
        return u;
    }

    // u 向 v 走 k 条边; k<=0 返回 u, 否则断连 -1, 超长返回 v. 时间 O(log n).
    int jump(int u, int v, int k) const
    {
        if (k <= 0) return u;
        int l = lca(u, v);
        if (l == -1) return -1;
        int du = dep[u] - dep[l];
        int dv = dep[v] - dep[l];
        if (du + dv <= k) return v;
        return k <= du ? jump_up(u, k) : jump_up(v, dv - (k - du));
    }
private:
    template <class G>
    void dfs(int u, int p, int root, G& g)
    {
        fa[0][u] = p;
        rt[u] = root;
        dep[u] = dep[p] + 1;
        dfn[u] = ++idx;
        rnk[idx] = u;
        rmq[0][idx] = u;
        sz[u] = 1;
        for (auto& e : g[u])
        {
            int v = e.v;
            if (v == p) continue;
            if constexpr (is_same_v<decltype(e.w), Empty>)
                dis[v] = dis[u] + 1;
            else
                dis[v] = dis[u] + e.w; // 自定义边权改此处
            dfs(v, u, root, g);
            sz[u] += sz[v];
        }
    }
};
#endif

/*
 * Usage:
 * // 森林自动多根, 每棵树取最小编号点为根; 每轮直接 build 并传实际点数
 * Graph<false> g(n, n - 1); // 带边权: Graph<false, LL> + g.add(u, v, w)
 * LCA lca(n);
 * for (int i = 1; i < n; i++) { int u, v; cin >> u >> v; g.add(u, v); }
 * lca.build(g, n);
 * lca.lca(u, v); // 不连通 -1; 多点: lca.lca({u, v, w})
 * lca.dist(u, v); // 真实距离, 边权图自动按 w 累计
 * lca.jump_up(u, k); // 0<=k<=dep[u], 跳过根返回 0
 * lca.jump(u, v, k); // u 沿 u->v 方向 k 步; k<=0 返 u, 超路长返 v
 * // 直读: dep | dfn/rnk 时间戳正反 | rt 所在根 | sz 子树大小 | fa[k][u] 2^k 祖先
 * // 子树 u = dfn 区间 [dfn[u], dfn[u]+sz[u]-1]; 递归 build 依赖评测机栈宽
 */
