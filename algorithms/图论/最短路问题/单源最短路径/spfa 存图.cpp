// zoi: spfa
#ifndef Z_OI_SPFA
#define Z_OI_SPFA

#include <queue>
#include "../../图的存储/Graph.cpp"
#include "../../../杂项/utils/utils.cpp"

using namespace std;

// ============ SPFA 队列优化单源最短路 ============
// 允许负边权(图需无负环, 判环用 spfaRing); dist 不可达 = INF
// 内存: dist 8B/点 + inq 4B/点 + 队 O(n) 4B/项; 预算 = max_n
struct SPFA
{
    int n;
    VLL dist;
    VI inq;
    // 构造: 预算 max_n
    // 时间: O(n) | 空间: 12B/点
    SPFA(int max_n = 0) : n(0), dist(max_n + 10, INF), inq(max_n + 10, 0) {}
    // 多测复位: dist 清 INF, inq 清 0
    // 时间: O(n) | 空间: O(1)
    void init(int _n)
    {
        n = _n;
        z_fill_n(_n, INF, dist);
        z_fill_n(_n, 0, inq);
    }
    // 单源跑最短路, 结果写 dist
    // 时间: 最坏 O(nm) | 空间: O(n)
    template <class G>
    void run(int s, G& g)
    {
        queue<int> q;
        dist[s] = 0;
        q.push(s);
        inq[s] = 1;
        while (q.size())
        {
            int u = q.front();
            q.pop();
            inq[u] = 0;
            for (auto& [v, nxt, w] : g[u])   // 上轮被松弛的点才可能再松弛, 队列加速
                if (dist[u] + w < dist[v])
                {
                    dist[v] = dist[u] + w;
                    if (!inq[v]) { q.push(v); inq[v] = 1; }
                }
        }
    }
};
#endif
/*
 * Usage:
 * Graph<true, LL> g{n, m};          // 负边权也行
 * SPFA sp{n};
 * sp.init(n);
 * for (int i = 1; i <= m; i++) { int u, v; LL w; cin >> u >> v >> w; g.add(u, v, w); }
 * sp.run(s, g);
 * sp.dist[v];                        // 不可达 = INF
 * // 多测: g.clear(); sp.init(n); 重跑
 */
