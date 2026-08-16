#include <vector>
#include <algorithm>
#include <type_traits>

using namespace std;

using LL = long long;
using VI = vector<int>;
using VLL  = vector<LL>;

template<typename... CS>
void z_fill_n(int n, int val, CS&... cs) 
{
    (fill(cs.begin(), cs.begin() + min((size_t)(n + 10), cs.size()), val), ...);
}
struct HLD_LCA 
{
    int n, dfn_idx;
    VI fa, dep, sz, son, top, dfn, rt;
    VLL dis;
    HLD_LCA(int max_n = 0) : n(max_n), dfn_idx(0), 
        fa(max_n + 10, 0), dep(max_n + 10, 0), sz(max_n + 10, 0), 
        son(max_n + 10, 0), top(max_n + 10, 0), dfn(max_n + 10, 0),
        rt(max_n + 10, 0), dis(max_n + 10, 0)
    {}
    void init(int _n) 
    {
        n = _n;
        dfn_idx = 0;
        z_fill_n(n + 10, 0, fa, dep, sz, son, top, dfn, rt, dis);
    }
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
    template <typename... Args>
    int lca(int u, int v, Args... rest)
    {
        int res = lca(u, v);
        ((res = lca(res, rest)), ...);
        return res;
    }
    // 树上距离, 不连通返回 -1, 使用真实距离
    LL dist(int u, int v)
    {
        int l = lca(u, v);
        if (l == -1) return -1;
        return dis[u] + dis[v] - 2 * dis[l];
    }
};