// zoi: bf
#ifndef Z_OI_BF
#define Z_OI_BF

#include "../../图的存储/Graph.cpp"
#include "../../../杂项/utils/utils.cpp"

using namespace std;

// ============ Bellman-Ford 单源最短路 ============
// 允许负边权(图需无负环, 判环用 bfRing); dist 不可达 = INF
// 内存: dist 8B/点; 预算 = max_n
struct BellmanFord
{
    int n;
    VLL dist;
    // 构造: 预算 max_n
    // 时间: O(n) | 空间: 8B/点
    BellmanFord(int max_n = 0) : n(0), dist(max_n + 10, INF) {}
    // 多测复位: dist 清 INF
    // 时间: O(n) | 空间: O(1)
    void init(int _n)
    {
        n = _n;
        z_fill_n(_n, INF, dist);
    }
    // 单源跑最短路, 结果写 dist
    // 时间: O(nm) | 空间: O(1)
    template <class G>
    void run(int s, G& g)
    {
        dist[s] = 0;
        for (int i = 1; i < n; i++)   // 每轮松弛全部边, 无更新提前收工
        {
            bool flag = false;
            for (int u = 1; u <= n; u++)
            {
                if (dist[u] == INF) continue;
                for (auto& [v, nxt, w] : g[u])
                    if (dist[u] + w < dist[v])
                    {
                        dist[v] = dist[u] + w;
                        flag = true;
                    }
            }
            if (!flag) break;
        }
    }
};
#endif
/*
 * Usage:
 * Graph<true, LL> g{n, m};          // 负边权也行
 * BellmanFord bf{n};
 * bf.init(n);
 * for (int i = 1; i <= m; i++) { int u, v; LL w; cin >> u >> v >> w; g.add(u, v, w); }
 * bf.run(s, g);
 * bf.dist[v];                        // 不可达 = INF
 * // 多测: g.clear(); bf.init(n); 重跑
 */
