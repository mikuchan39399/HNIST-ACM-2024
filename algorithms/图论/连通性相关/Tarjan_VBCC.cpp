// zoi: vbcc
#ifndef Z_OI_VBCC
#define Z_OI_VBCC

#include <vector>
#include <algorithm>
#include <iostream>
#include <cassert>
#include "../图的存储/Graph.cpp"
#include "../../杂项/utils/utils.cpp"

using namespace std;

struct VBCC
{
    int n;
    int dfn_idx, vbcc_cnt;
    Graph<false, Empty> tree;   // 圆方树
    VI dfn, low, sta, cut;
    VVI vbcc_cir;               // 存储每个 VBCC 所包含的所有圆点
    VBCC(int max_n = 0) : n(max_n), dfn_idx(0), vbcc_cnt(0),
        tree(max_n * 2, max_n * 2),
        dfn(max_n + 10, 0), low(max_n + 10, 0), cut(max_n + 10, 0),
        vbcc_cir(1, VI{})
    {
        sta.reserve(max_n + 10);
    }
    void init(int _n)
    {
        n = _n;
        tree.clear();
        z_fill_n(n, 0, dfn, low, cut);
        dfn_idx = vbcc_cnt = 0;
        sta.clear();
        vbcc_cir.assign(1, VI{});
    }
    // 跑 Tarjan 求点双; g = 无向 Graph 形邻接(范围 for g[u] 取 e.v 即可,
    // 权任意); root = -1 时扫全图 1..n
    // 时间: O(n + m) | 空间: O(n)
    template <class G>
    void build(G& g, int _n, int root = -1)
    {
        n = _n;
        if (root != -1)
        {
            tarjan(g, root, root);
            return;
        }
        for (int i = 1; i <= n; i++)
        {
            if (!dfn[i])
            {
                tarjan(g, i, i);
            }
        }
    }
    // 构建圆方树
    void build_tree()
    {
        for (int i = 1; i <= vbcc_cnt; i++)
        {
            int u = n + i;
            for (int v : vbcc_cir[i])
            {
                // Graph<false> 内部自动正反向建边，只需 add 一次即可！
                tree.add(u, v);
            }
        }
    }
    // 返回单个割点参与的 VBCC 列表
    VI get_bel_vbccs(int u)
    {
        VI res;
        for (auto& e : tree[u])
        {
            int v = e.v;           // v 是方点，编号在 n + 1 到 n + vbcc_cnt 之间
            res.push_back(v - n);  // 减去 n 就是真正的 VBCC 编号
        }
        return res;
    }
    // 返回单个 VBCC 包含的割点列表
    VI get_cuts_vbcc(int i)
    {
        VI res;
        if (i < 1 || i > vbcc_cnt) return res;
        for (int v : vbcc_cir[i])
        {
            if (cut[v]) res.push_back(v);
        }
        return res;
    }
private:
    template <class G>
    void tarjan(G& g, int u, int root)
    {
        dfn_idx++;
        dfn[u] = low[u] = dfn_idx;
        sta.push_back(u);
        int child_cnt = 0;
        for (auto& e : g[u])
        {
            int v = e.v;
            if (!dfn[v])
            {
                child_cnt++;
                tarjan(g, v, root);
                low[u] = min(low[u], low[v]);
                if (low[v] >= dfn[u])
                {
                    if (u != root) cut[u] = 1;
                    vbcc_cnt++;
                    vbcc_cir.push_back(VI{});
                    int t;
                    do
                    {
                        t = sta.back();
                        sta.pop_back();
                        vbcc_cir[vbcc_cnt].push_back(t);
                    } while (t != v);
                    vbcc_cir[vbcc_cnt].push_back(u);
                }
            }
            else low[u] = min(low[u], dfn[v]);
        }
        if (u == root && child_cnt >= 2) cut[u] = 1;
        // 处理没有任何子节点的根节点（孤立点 / 单点自环）
        if (u == root && child_cnt == 0)
        {
            vbcc_cnt++;
            vbcc_cir.push_back(VI{u});
        }
    }
};


#endif
// Usage:
/*
const int N = 5e5 + 10;
const int M = 1e6 + 10;

VBCC graph(N);
Graph<false> g(N, M);

void dfs_tree(int u, int fa, VI& vis)
{
    vis[u] = 1;
    // 在圆方树上遍历，u <= n 是圆点(原图节点)，u > n 是方点(VBCC集合)
    for (auto& e : graph.tree[u])
    {
        int v = e.v;
        if (v == fa) continue;
        dfs_tree(v, u, vis);
    }
}

void solve()
{
    int n, m; cin >> n >> m;
    graph.init(n);
    g.clear();
    for (int i = 1; i <= m; i++)
    {
        int u, v;
        cin >> u >> v;
        if (u == v) continue;
        // 外部只需要加一次，Graph<false> 会自动搞定无向边
        g.add(u, v);
    }
    graph.build(g, n);
    graph.build_tree();
    int tree_nodes = n + graph.vbcc_cnt;
    VI vis(tree_nodes + 1, 0);
    for (int i = 1; i <= tree_nodes; i++)
    {
        int dfn_check = 0;
        if (i <= n)
        {
            dfn_check = graph.dfn[i];
        }
        else if (!graph.vbcc_cir[i - n].empty())
        {
            dfn_check = graph.dfn[graph.vbcc_cir[i - n][0]];
        }
        if (!vis[i] && dfn_check)
        {
            dfs_tree(i, 0, vis);
        }
    }
}
*/
