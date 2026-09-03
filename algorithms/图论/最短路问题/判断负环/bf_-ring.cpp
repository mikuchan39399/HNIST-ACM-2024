// zoi: bfRing
#ifndef Z_OI_BFRING
#define Z_OI_BFRING

#include "../../图的存储/Graph.cpp"
#include "../../../杂项/utils/utils.cpp"

using namespace std;

// ============ Bellman-Ford 判负环 ============
// 全源种子(dist 全 0), 判据 = 第 n 轮全边松弛仍有更新, 与起点可达性无关
// 内存: dist 8B/点; 预算 = max_n
struct BFRing
{
    int n;
    VLL dist;
    // 构造: 预算 max_n
    // 时间: O(n) | 空间: 8B/点
    BFRing(int max_n = 0) : n(0), dist(max_n + 10, 0) {}
    // 多测复位: n 重配
    // 时间: O(1) | 空间: O(1)
    void init(int _n) { n = _n; }
    // 判整图是否有负环, 返回 true = 有
    // 时间: O(nm) | 空间: O(1)
    template <class G>
    bool run(G& g)
    {
        z_fill_n(n, 0, dist);
        for (int i = 1; i <= n; i++)
        {
            bool flag = false;
            for (int u = 1; u <= n; u++)
                for (auto& [v, nxt, w] : g[u])
                    if (dist[u] + w < dist[v])
                    {
                        dist[v] = dist[u] + w;
                        flag = true;
                    }
            if (!flag) return false;   // n 轮内收敛 = 无负环
        }
        return true;
    }
};
#endif
/*
 * Usage:
 * Graph<true, LL> g{n, m};
 * BFRing bf{n};
 * bf.init(n);
 * for (int i = 1; i <= m; i++) { int u, v; LL w; cin >> u >> v >> w; g.add(u, v, w); }
 * bf.run(g);                         // true = 图里有负环
 * // 多测: g.clear(); bf.init(n); 重跑
 */
