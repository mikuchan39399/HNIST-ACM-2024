// zoi: dij
#ifndef Z_OI_DIJ
#define Z_OI_DIJ

#include "../../图的存储/Graph.cpp"
#include "../../../杂项/utils/utils.cpp"

// 非负整数边权最短路, 边权和有限距离 < INF, 候选加法在 LL 内; dist 不可达为 INF
// dist 每点 8 B, 堆项通常 16 B, 含过期项的峰值为 O(n+m), 重边多时不能按 n 预算
struct Dijkstra
{
    VLL dist;
    // 分配 max_n 个点的距离表, 点编号为 1..n
    // 时间 O(max_n) | 空间 O(max_n)
    Dijkstra(int max_n = 0) : dist(max_n + 10, INF) {}
    // 在构造容量内清空距离, 每次 run 前调用
    // 时间 O(n) | 额外空间 O(1)
    void init(int _n) { z_fill_n(_n, INF, dist); }
    // nodes 整个数组作零距离源集, 可空可重复, 各点到最近源的距离写入 dist
    // 时间 O(k + (n+m) log(n+m+1)) | 额外空间 O(n+m), k 为源数组长度
    template <class G>
    void run(const VI& nodes, G& g)
    {
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
    // 从 s 跑最短路并写入 dist, 不修改图
    // 时间 O((n+m) log(m+2)) | 额外空间 O(m+1)
    template <class G>
    void run(int s, G& g) { run(VI{s}, g); }
};
#endif
/* Usage
int main()
{
    Graph<true, LL> g(4, 3);
    g.add(1, 2, 5); g.add(2, 3, 2); g.add(4, 3, 1);
    Dijkstra dij(4);
    dij.init(4); dij.run(1, g);
    cout << dij.dist[3] << ' ' << (dij.dist[4] == INF) << '\n'; // 7 1
    dij.init(4); dij.run(VI{1, 4, 4}, g);
    cout << dij.dist[3] << '\n'; // 1, 多源取最近距离
    g.clear(); dij.init(2); dij.run(2, g); // 新图与算法器分别复位
}
*/
