// zoi: scc
#ifndef Z_OI_SCC
#define Z_OI_SCC

#include <vector>
#include <algorithm>
#include <iostream>
#include <cassert>
#include "../图的存储/Graph.cpp"
#include "../../杂项/utils/utils.cpp"

using namespace std;

struct SCC
{
    int n;
    int dfn_idx, scc_cnt;
    Graph<true, Empty> dag;    // 缩点 DAG
    VI dfn, low, bel, in_stk, sta;
    SCC(int max_n = 0, int max_m = 0) : n(max_n), dfn_idx(0), scc_cnt(0),
        dag(max_n, max_m),
        dfn(max_n + 10, 0), low(max_n + 10, 0),
        bel(max_n + 10, 0), in_stk(max_n + 10, 0)
    {
        sta.reserve(max_n + 10);
    }
    void init(int _n)
    {
        n = _n;
        dag.clear();
        z_fill_n(n, 0, dfn, low, bel, in_stk);
        dfn_idx = scc_cnt = 0;
        sta.clear();
    }
    // 强连通分量
    // 时间: O(n + m) | 空间: O(n)
    template <class G>
    void build(G& g, int _n)
    {
        n = _n;
        for (int i = 1; i <= n; i++)
            if (!dfn[i]) tarjan(g, i);
    }
    // 缩点 DAG
    template <class G>
    void build_dag(G& g)
    {
        for (int u = 1; u <= n; u++)
        {
            for (auto& e : g[u])
            {
                int v = e.v;
                if (bel[u] != bel[v]) dag.add(bel[u], bel[v]);
            }
        }
    }
    // 去重边 DAG
    template <class G>
    void build_dag_unique(G& g)
    {
        VPII edges;
        for (int u = 1; u <= n; u++)
        {
            for (auto& e : g[u])
            {
                int v = e.v;
                if (bel[u] != bel[v]) edges.push_back({bel[u], bel[v]});
            }
        }
        sort(edges.begin(), edges.end());
        edges.erase(unique(edges.begin(), edges.end()), edges.end());
        for (auto& edge : edges)
            dag.add(edge.first, edge.second);
    }
private:
    template <class G>
    void tarjan(G& g, int u)
    {
        dfn_idx++;
        low[u] = dfn[u] = dfn_idx;
        sta.push_back(u);
        in_stk[u] = 1;
        for (auto& e : g[u])
        {
            int v = e.v;
            if (!dfn[v])
            {
                tarjan(g, v);
                low[u] = min(low[u], low[v]);
            }
            else if (in_stk[v]) low[u] = min(low[u], dfn[v]);
        }
        if (low[u] == dfn[u])
        {
            scc_cnt++;
            int t;
            do
            {
                t = sta.back();
                sta.pop_back();
                in_stk[t] = 0;
                bel[t] = scc_cnt;
            } while (t != u);
        }
    }
};

#endif
// Usage:
/*
SCC scc(N, M);
Graph<true> g(N, M);
scc.init(n); g.clear();
for (int i = 1; i <= m; i++) { int u, v; cin >> u >> v; g.add(u, v); }

scc.build(g, n);    // bel[u] = 所在 SCC 编号, 编号越大拓扑序越靠前
scc.build_dag(g);   // 可选: 缩点 DAG, 原图带权直接喂不受影响

// DAG DP 沿 scc_cnt 降序即拓扑序:
for (int u = scc.scc_cnt; u >= 1; u--)
    for (auto& e : scc.dag[u]) ...
*/
