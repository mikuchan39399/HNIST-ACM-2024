// zoi: dij
#ifndef Z_OI_DIJ
#define Z_OI_DIJ

#include <queue>
#include "../../图的存储/Graph.cpp"
#include "../../../杂项/utils/utils.cpp"

using namespace std;

// ============ Dijkstra 堆优化单源最短路 ============
// 要求边权非负; dist 不可达 = INF(utils)
// 内存: dist 8B/点 + 堆峰值 O(n) 16B/项; 预算 = max_n
struct Dijkstra
{
    VLL dist;
    // 构造: 预算 max_n
    // 时间: O(n) | 空间: 8B/点
    Dijkstra(int max_n = 0) : dist(max_n + 10, INF) {}
    // 多测复位: dist 清 INF
    // 时间: O(n) | 空间: O(1)
    void init(int _n) { z_fill_n(_n, INF, dist); }
    // 多源跑最短路, nodes 全部作源(距离 0), 结果写 dist
    // 时间: O(m log n) | 空间: O(n)
    template <class G>
    void run(const VI& nodes, G& g)
    {
        priority_queue<PLI, vector<PLI>, greater<PLI>> heap;
        for (int s : nodes)
        {
            dist[s] = 0;
            heap.push({0, s});
        }
        while (heap.size())
        {
            auto [d, u] = heap.top();
            heap.pop();
            if (d > dist[u]) continue;
            for (auto& [v, nxt, w] : g[u])
                if (dist[u] + w < dist[v])
                {
                    dist[v] = dist[u] + w;
                    heap.push({dist[v], v});
                }
        }
    }
    // 单源跑最短路, 结果写 dist
    // 时间: O(m log n) | 空间: O(n)
    template <class G>
    void run(int s, G& g) { run(VI{s}, g); }
};
#endif
/*
 * Usage:
 * Graph<true, LL> g{n, m};          // 有向带权
 * Dijkstra dij{n};
 * dij.init(n);
 * for (int i = 1; i <= m; i++) { int u, v; LL w; cin >> u >> v >> w; g.add(u, v, w); }
 * dij.run(s, g);                    // 单源; 多源传 VI{a, b, c}
 * dij.dist[v];                      // 不可达 = INF
 * // 多测: g.clear(); dij.init(n); 重跑
 */
