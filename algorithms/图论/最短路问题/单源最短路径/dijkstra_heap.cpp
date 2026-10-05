// zoi: dij
#ifndef Z_OI_DIJ
#define Z_OI_DIJ

#include "../../图的存储/Graph.cpp"
#include "../../../杂项/utils/utils.cpp"

// 非负整数权最短路, 点号 1..n; 边权及有限距离<INF, 候选加法须在 LL 内
// dist 不可达为 INF; 距离表 O(n), 堆空间 O(n+m)
struct Dijkstra
{
    VLL dist;
    // 预留 max_n 个点. 时空 O(max_n)
    Dijkstra(int max_n = 0) : dist(max_n + 10, INF) {}
    // 清空 n<=容量 个点的距离; run 自动调用. 时间 O(n)
    void init(int _n) { z_fill_n(_n, INF, dist); }
    // 整个 nodes 为零距离源集(可空可重复), dist 为到最近源的距离, 自动复位
    // 时间 O(n+k+(n+m)log(n+m+1)), 额外空间 O(n+m), k=源集长度
    template <class G>
    void run(const VI& nodes, G& g, int n)
    {
        init(n);
        priority_queue<PLI, vector<PLI>, greater<PLI>> heap;
        for (int s : nodes)
        {
            if (dist[s] == 0) continue;
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
    // 单源最短路, 自动复位并写入 dist. 时间 O(n+(n+m)log(m+2)), 堆 O(m+1)
    template <class G>
    void run(int s, G& g, int n) { run(VI{s}, g, n); }
};
#endif
/* Usage
int main()
{
    Graph<true, LL> g(4, 3);
    g.add(1, 2, 5); g.add(2, 3, 2); g.add(4, 3, 1);
    Dijkstra dij(4);
    dij.run(1, g, 4);
    cout << dij.dist[3] << ' ' << (dij.dist[4] == INF) << '\n'; // 7 1
    dij.run(VI{1, 4, 4}, g, 4);
    cout << dij.dist[3] << '\n'; // 1, 多源取最近距离
    g.clear(); dij.run(2, g, 2); // 新图与算法器分别复位
}
*/
