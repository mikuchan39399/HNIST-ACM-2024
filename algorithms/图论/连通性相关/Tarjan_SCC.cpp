// zoi: scc
#ifndef Z_OI_SCC
#define Z_OI_SCC

#include "../图的存储/Graph.cpp"
#include "../../杂项/utils/utils.cpp"

// 强连通分量; bel[u] 为 1 .. scc_cnt, 跨分量边从大编号指向小编号, dag 恒无权
// 原图外置, build 可接带权图 (含 SegGraph), 只读邻接中的 e.v; 递归深度最坏 n
struct SCC
{
    int n;
    int dfn_idx, scc_cnt;
    Graph<true, Empty> dag;
    VI dfn, low, bel, in_stk, sta;
    // N 取原图最大点数, M 取缩点后最多保留的有向边数 (直接取原图 m 即可)
    // 64 位 GCC, int = 4 B 时预留约 36 * N + 8 * M B; N = M = 2e5 约 8.8 MB, 不含原图与递归栈
    // 时间 O(N) | 空间 O(N + M)
    SCC(int max_n = 0, int max_m = 0) : n(max_n), dfn_idx(0), scc_cnt(0),
        dag(max_n, max_m),
        dfn(max_n + 10, 0), low(max_n + 10, 0),
        bel(max_n + 10, 0), in_stk(max_n + 10, 0)
    {
        sta.reserve(max_n + 10);
    }
    // 复位本轮结果与内部图; n 不超过构造时的 N, 原图需另行 clear()
    // 时间 O(n + 上轮点数) | 额外空间 O(1)
    void init(int _n)
    {
        n = _n;
        dag.clear();
        z_fill_n(n, 0, dfn, low, bel, in_stk);
        dfn_idx = scc_cnt = 0;
        sta.clear();
    }
    // 将 g 的 1 .. n 点划分到 bel; 每轮先 init(n), 边权不参与计算
    // 时间: O(n + m) | 空间: O(n)
    template <class G>
    void build(G& g, int _n)
    {
        n = _n;
        for (int i = 1; i <= n; i++)
            if (!dfn[i]) tarjan(g, i);
    }
    // 向 dag 追加跨分量边, 保留重边, 丢弃权值; 重建先 dag.clear()
    // 时间 O(n + m) | 额外空间 O(m)
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
    // 向 dag 追加去重后的跨分量边; 与 build_dag 二选一, 重建先 dag.clear()
    // 时间 O(n + m log m) | 额外空间 O(m)
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
int main()
{
    int n, m;
    cin >> n >> m;
    SCC scc(n, m);
    Graph<true> g(n, m);
    scc.init(n);
    for (int i = 1; i <= m; i++)
    {
        int u, v;
        cin >> u >> v;
        g.add(u, v);
    }
    scc.build(g, n);
    for (int u = 1; u <= n; u++) cout << scc.bel[u] << ' '; // 输出各点的分量号
    cout << '\n';
    scc.build_dag_unique(g); // 保留重边时改用 build_dag(g)
    for (int u = scc.scc_cnt; u >= 1; u--) // 按拓扑序遍历缩点图
        for (auto e : scc.dag[u]) cout << u << ' ' << e.v << '\n';
    return 0;
}
*/
