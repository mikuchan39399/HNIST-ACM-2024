// zoi: dijN
#ifndef Z_OI_DIJN
#define Z_OI_DIJN

#include "../../图的存储/Graph.cpp"
#include "../../../杂项/utils/utils.cpp"

using namespace std;

// ============ Dijkstra 朴素 O(n^2) 单源最短路 ============
// 要求边权非负; 稠密图(m≈n^2)比堆版省堆开销; dist 不可达 = INF
// 内存: dist 8B/点 + st 4B/点; 预算 = max_n
struct DijkstraN
{
    int n;
    VLL dist;
    VI st;
    // 构造: 预算 max_n
    // 时间: O(n) | 空间: 12B/点
    DijkstraN(int max_n = 0) : n(0), dist(max_n + 10, INF), st(max_n + 10, 0) {}
    // 多测复位: dist 清 INF, st 清 0 (dist[0]=INF 兼作选点哨兵)
    // 时间: O(n) | 空间: O(1)
    void init(int _n)
    {
        n = _n;
        z_fill_n(_n, INF, dist);
        z_fill_n(_n, 0, st);
    }
    // 单源跑最短路, 结果写 dist
    // 时间: O(n^2 + m) | 空间: O(1)
    template <class G>
    void run(int s, G& g)
    {
        dist[s] = 0;
        for (int i = 1; i < n; i++)   // 每轮确认一个点, 共 n-1 轮
        {
            int t = 0;
            for (int j = 1; j <= n; j++)
                if (!st[j] && dist[j] < dist[t]) t = j;
            if (!t) break;            // 其余点全不可达
            st[t] = true;
            for (auto& [v, nxt, w] : g[t])
                dist[v] = min(dist[v], dist[t] + w);
        }
    }
};
#endif
/*
 * Usage:
 * Graph<true, int> g{n, n * n};     // 稠密图用 int 权省内存
 * DijkstraN dij{n};
 * dij.init(n);
 * for (int i = 1; i <= m; i++) { int u, v, w; cin >> u >> v >> w; g.add(u, v, w); }
 * dij.run(s, g);
 * dij.dist[v];                       // 不可达 = INF
 * // 多测: g.clear(); dij.init(n); 重跑
 */
