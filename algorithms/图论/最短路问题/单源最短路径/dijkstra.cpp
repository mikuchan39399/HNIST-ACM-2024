// zoi: dijN
#ifndef Z_OI_DIJN
#define Z_OI_DIJN

#include "../../图的存储/Graph.cpp"
#include "../../../杂项/utils/utils.cpp"

// 稠密图非负整数权最短路, 1-based; 边权及有限距离<INF, 候选加法在 LL 内
// dist 不可达为 INF. 空间 O(n)
struct DijkstraN
{
    int n;
    VLL dist;
    VI st;
    // 预留 max_n 个点. 时空 O(max_n)
    DijkstraN(int max_n = 0) : n(0), dist(max_n + 10, INF), st(max_n + 10, 0) {}
    // 清空 n<=容量 个点的状态; run 自动调用. 时间 O(n)
    void init(int _n)
    {
        n = _n;
        z_fill_n(_n, INF, dist);
        z_fill_n(_n, 0, st);
    }
    // 从 s 重算 dist, 自动复位. 时间 O(n²+m), 额外空间 O(1)
    template <class G>
    void run(int s, G& g, int _n)
    {
        init(_n);
        dist[s] = 0;
        for (int i = 1; i < n; i++)
        {
            int t = 0;
            for (int j = 1; j <= n; j++)
                if (!st[j] && dist[j] < dist[t]) t = j;
            if (!t) break;
            st[t] = true;
            for (auto& [v, nxt, w] : g[t])
                dist[v] = min(dist[v], dist[t] + w);
        }
    }
};
#endif
/* Usage
int main()
{
    Graph<true, int> g(4, 3);
    g.add(1, 2, 5); g.add(2, 3, 2); g.add(1, 3, 9);
    DijkstraN dij(4);
    dij.run(1, g, 4);
    cout << dij.dist[3] << ' ' << (dij.dist[4] == INF) << '\n'; // 7 1
    g.clear(); g.add(2, 1, 3);
    dij.run(2, g, 2);
    cout << dij.dist[1] << '\n'; // 3
}
*/
