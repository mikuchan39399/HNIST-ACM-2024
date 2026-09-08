// zoi: bf
#ifndef Z_OI_BF
#define Z_OI_BF

#include "../../图的存储/Graph.cpp"
#include "../../../杂项/utils/utils.cpp"

// 每轮原地松弛全部可达边, 允许负整数边权; 源点可达部分须无负环
// dist 不可达为 INF, 有限距离 < INF, 所有候选加法在 LL 内; 原地更新的轮数不是路径边数上限
// 每点 8 B, 20 万点约 1.6 MB, 不含 Graph
struct BellmanFord
{
    int n;
    VLL dist;
    // 分配 max_n 个点的工作表, 点编号为 1..n, n >= 1
    // 时间 O(max_n) | 空间 O(max_n)
    BellmanFord(int max_n = 0) : n(0), dist(max_n + 10, INF) {}
    // 在构造容量内设为 n 个点并清空距离, 每次 run 前调用
    // 时间 O(n) | 额外空间 O(1)
    void init(int _n)
    {
        n = _n;
        z_fill_n(_n, INF, dist);
    }
    // 从 s 跑最短路并写入 dist, 不修改图
    // 最坏时间 O(n(n+m)) | 额外空间 O(1)
    template <class G>
    void run(int s, G& g)
    {
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
    bf.init(4); bf.run(1, g);
    cout << bf.dist[3] << ' ' << (bf.dist[4] == INF) << '\n'; // -3 1
    g.clear(); g.add(2, 1, -7);
    bf.init(2); bf.run(2, g);
    cout << bf.dist[1] << '\n'; // -7
}
*/
