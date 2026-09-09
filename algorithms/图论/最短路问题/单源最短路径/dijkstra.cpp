// zoi: dijN
#ifndef Z_OI_DIJN
#define Z_OI_DIJN

#include "../../图的存储/Graph.cpp"
#include "../../../杂项/utils/utils.cpp"

// 逐点扫描选最近点, 适合稠密图; 非负整数边权与有限距离 < INF, 候选加法在 LL 内
// dist 不可达为 INF, st 标记已选点; 每点 12 B, 2000 点约 24 KB, 不含 Graph
struct DijkstraN
{
    int n;
    VLL dist;
    VI st;
    // 分配 max_n 个点的工作表, 点编号为 1..n
    // 时间 O(max_n) | 空间 O(max_n)
    DijkstraN(int max_n = 0) : n(0), dist(max_n + 10, INF), st(max_n + 10, 0) {}
    // 在构造容量内设为 n 个点并清空距离与选点状态, run 内部自动调用
    // 时间 O(n) | 额外空间 O(1)
    void init(int _n)
    {
        n = _n;
        z_fill_n(_n, INF, dist);
        z_fill_n(_n, 0, st);
    }
    // 重算本轮 _n 点中从 s 出发的最短路并写入 dist, 自动复位且不修改图
    // 时间 O(n^2 + m) | 额外空间 O(1)
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
            if (!t) break; // 其余点全不可达
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
