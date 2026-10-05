// zoi: bf
#ifndef Z_OI_BF
#define Z_OI_BF

#include "../../图的存储/Graph.cpp"
#include "../../../杂项/utils/utils.cpp"

// 可负整数权最短路, 1-based; 源点可达部分不能有负环
// dist 不可达为 INF, 有限距离<INF, 候选加法在 LL 内; 松弛轮数不是边数限制. 空间 O(n)
struct BellmanFord
{
    int n;
    VLL dist;
    // 预留 max_n 个点. 时空 O(max_n)
    BellmanFord(int max_n = 0) : n(0), dist(max_n + 10, INF) {}
    // 清空 n<=容量 个点的距离; run 自动调用. 时间 O(n)
    void init(int _n)
    {
        n = _n;
        z_fill_n(_n, INF, dist);
    }
    // 从 s 重算 dist, 自动复位. 最坏时间 O(n(n+m)), 额外空间 O(1)
    template <class G>
    void run(int s, G& g, int _n)
    {
        init(_n);
        dist[s] = 0;
        for (int i = 1; i < n; i++)
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
/* Usage
int main()
{
    Graph<true, LL> g(4, 3);
    g.add(1, 2, 5); g.add(2, 3, -8); g.add(1, 3, 1);
    BellmanFord bf(4);
    bf.run(1, g, 4);
    cout << bf.dist[3] << ' ' << (bf.dist[4] == INF) << '\n'; // -3 1
    g.clear(); g.add(2, 1, -7);
    bf.run(2, g, 2);
    cout << bf.dist[1] << '\n'; // -7
}
*/
