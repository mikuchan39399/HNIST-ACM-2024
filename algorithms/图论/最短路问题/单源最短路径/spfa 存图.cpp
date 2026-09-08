// zoi: spfa
#ifndef Z_OI_SPFA
#define Z_OI_SPFA

#include "../../图的存储/Graph.cpp"
#include "../../../杂项/utils/utils.cpp"

// 队列只重扫距离变小的点, 允许负整数边权; 源点可达部分须无负环, 最坏仍为 O(nm)
// dist 不可达为 INF, 有限距离 < INF, 所有候选加法在 LL 内
// 工作表每点 12 B, 队列每项 4 B 且至多 n 项, 20 万点约 3.2 MB, 不含 Graph
struct SPFA
{
    int n;
    VLL dist;
    VI inq;
    // 分配 max_n 个点的工作表, 点编号为 1..n, n >= 1
    // 时间 O(max_n) | 空间 O(max_n)
    SPFA(int max_n = 0) : n(0), dist(max_n + 10, INF), inq(max_n + 10, 0) {}
    // 在构造容量内设为 n 个点并清空工作表, 每次 run 前调用
    // 时间 O(n) | 额外空间 O(1)
    void init(int _n)
    {
        n = _n;
        z_fill_n(_n, INF, dist);
        z_fill_n(_n, 0, inq);
    }
    // 从 s 跑最短路并写入 dist, 不修改图
    // 最坏时间 O(nm+n) | 额外空间 O(n)
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
            for (auto& [v, nxt, w] : g[u])
                if (dist[u] + w < dist[v])
                {
                    dist[v] = dist[u] + w;
                    if (!inq[v]) { q.push(v); inq[v] = 1; }
                }
        }
    }
};
#endif
/* Usage
int main()
{
    Graph<true, LL> g(4, 3);
    g.add(1, 2, 5); g.add(2, 3, -8); g.add(1, 3, 1);
    SPFA sp(4);
    sp.init(4); sp.run(1, g);
    cout << sp.dist[3] << ' ' << (sp.dist[4] == INF) << '\n'; // -3 1
    g.clear(); g.add(2, 1, -7);
    sp.init(2); sp.run(2, g);
    cout << sp.dist[1] << '\n'; // -7
}
*/
