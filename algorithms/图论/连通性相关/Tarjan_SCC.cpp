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
    Graph<true, Empty> g;      // 原有向图
    Graph<true, Empty> dag;    // 缩点后的 DAG
    VI dfn, low, bel, in_stk, sta;
    SCC(int max_n = 0, int max_m = 0) : n(max_n), dfn_idx(0), scc_cnt(0),
        g(max_n, max_m), dag(max_n, max_m),
        dfn(max_n + 10, 0), low(max_n + 10, 0),
        bel(max_n + 10, 0), in_stk(max_n + 10, 0)
    {
        sta.reserve(max_n + 10);
    }
    void init(int _n)
    {
        n = _n;
        g.clear();
        dag.clear();
        z_fill_n(n, 0, dfn, low, bel, in_stk);
        dfn_idx = scc_cnt = 0;
        sta.clear();
    }
    void add_edge(int u, int v) { g.add(u, v); }
    void build()
    {
        for (int i = 1; i <= n; i++)
            if (!dfn[i]) tarjan(i);
    }
    // 构建缩点后的 DAG
    void build_dag()
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
    void build_dag_unique()
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
    void tarjan(int u)
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
                tarjan(v);
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
const int MAXN = 500005;
const int MAXM = 1000005;
SCC graph(MAXN, MAXM);

int scc_val[MAXN];
int dp[MAXN]; // 记录到达每个 SCC 的最大权值和

void solve()
{
    int n, m;
    cin >> n >> m;

    graph.init(n);

    VI val(n + 1);
    for (int i = 1; i <= n; i++)
    {
        cin >> val[i];
        scc_val[i] = 0;
        dp[i] = 0;
    }

    for (int i = 1; i <= m; i++)
    {
        int u, v;
        cin >> u >> v;
        graph.add_edge(u, v);
    }

    // 1. 跑 Tarjan 找 SCC
    graph.build();

    // 2. 将原图权值累加到对应的 SCC 新节点上
    for (int i = 1; i <= n; i++)
    {
        scc_val[graph.bel[i]] += val[i];
    }

    // 3. 构建 DAG
    graph.build_dag();

    // 4. DAG 上 DP (利用 Tarjan 自带的拓扑序)
    // 初始化 DP 数组为当前点权
    for (int i = 1; i <= graph.scc_cnt; i++)
    {
        dp[i] = scc_val[i];
    }

    // Tarjan 的编号(scc_cnt)越大，在拓扑序中越靠前(靠近源点)
    // 逆序遍历 scc_cnt，等价于正向拓扑排序遍历
    for (int u = graph.scc_cnt; u >= 1; u--)
    {
        for (auto& e : graph.dag[u])
        {
            int v = e.v;
            dp[v] = max(dp[v], dp[u] + scc_val[v]);
        }
    }

    // 统计整张图的最长路
    int ans = 0;
    for (int i = 1; i <= graph.scc_cnt; i++)
    {
        ans = max(ans, dp[i]);
    }
    cout << ans << endl;
}
*/
