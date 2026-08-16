#include <vector>
#include <algorithm>
#include <cassert>
#include <type_traits>
using namespace std;
using LL = long long;
using VI = vector<int>;
using VVI = vector<vector<int>>;

#ifndef Z_OI_EMPTY
#define Z_OI_EMPTY
struct Empty {};
#endif


template<typename... CS>
void z_fill_n(int n, int val, CS&... cs) 
{
    assert(((int)cs.size() >= n && ...));
    (fill(cs.begin(), cs.begin() + min((size_t)(n + 10), cs.size()), val), ...);
}
struct LCA
{
    int n;
    int idx;
    int max_bit;
    int LOG;           
    VI dep, dfn, rnk, rt;
    vector<LL> dis;     // 真实距离
    VVI rmq, fa; 
    LCA(int max_n = 0) : n(max_n), idx(0),
        max_bit(max_n == 0 ? 0 : __lg(max_n)),
        LOG(max_n <= 1 ? 2 : __lg(max_n) + 2),
        dep(max_n + 10, 0), dfn(max_n + 10, 0), rnk(max_n + 10, 0),
        rt(max_n + 10, 0), dis(max_n + 10, 0)
    {
        rmq.assign(LOG, VI(max_n + 10, 0));
        fa.assign(LOG, VI(max_n + 10, 0));
    }
    void init(int _n) // 可仅多测使用
    {
        n = _n;
        idx = 0;
        max_bit = n == 0 ? 0 : __lg(n);
        z_fill_n(n + 10, 0, dep, dfn, rt, dis);
    }
    template <class G>
    void dfs(int u, int p, int root, G& g)
    {
        fa[0][u] = p;
        rt[u] = root;
        dep[u] = dep[p] + 1;
        dfn[u] = ++idx;
        rnk[idx] = u;
        rmq[0][idx] = u;
        for (auto& e : g[u])
        {
            int v = e.v;
            if (v == p) continue;
            if constexpr (is_same_v<decltype(e.w), Empty>)
                dis[v] = dis[u] + 1;  
            else
                dis[v] = dis[u] + e.w;  // 自定义边权请修改此处
            dfs(v, u, root, g);
        }
    }
    template <class G>
    void build(G& g)
    {
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
    // 节点不联通返回 -1
    int lca(int u, int v)
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
    // 树上真实距离, 不连通返回 -1
    LL dist(int u, int v)
    {
        int l = lca(u, v);
        if (l == -1) return -1;
        return dis[u] + dis[v] - 2 * dis[l];
    }
    // u 向 v 方向移动 k 步: k <= 0 返回 u; 超过 u-v 路长返回 v; 不连通返回 -1
    int jump(int u, int v, int k)
    {
        if (k <= 0) return u;
        int l = lca(u, v);
        if (l == -1) return -1;
        int du = dep[u] - dep[l];
        int dv = dep[v] - dep[l];
        if (du + dv < k) return v;
        if (k <= du)
        {
            int x = u;
            int step_bit = __lg(k);
            for (int i = step_bit; i >= 0; i--)
            {
                if (k & (1 << i))
                {
                    x = fa[i][x];
                    if (x == 0) break;
                }
            }
            return x;
        }
        else
        {
            int remain = k - du;
            int up_steps = dv - remain;
            int x = v;
            if (up_steps == 0) return x;
            int step_bit = __lg(up_steps);
            for (int i = step_bit; i >= 0; i--)
            {
                if (up_steps & (1 << i))
                {
                    x = fa[i][x];
                    if (x == 0) break;
                }
            }
            return x;
        }
    }
};