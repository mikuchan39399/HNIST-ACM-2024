// zoi: bfRing
#ifndef Z_OI_BFRING
#define Z_OI_BFRING

#include "../../图的存储/Graph.cpp"
#include "../../../杂项/128位整数/128int.cpp"

// 全点以零距离起步, 第 n 轮仍能松弛即有负环, 可发现任意分量的负环
// Graph 用 int/LL 整数权, dist 用库内 i128 避免负环反复降低距离时溢出, 不作最短路结果
// 每点 16 B, 20 万点约 3.2 MB, 不含 Graph
struct BFRing
{
    int n;
    vector<i128> dist;
    // 分配 max_n 个点的工作表, 点编号为 1..n, n >= 1
    // 时间 O(max_n) | 空间 O(max_n)
    BFRing(int max_n = 0) : n(0), dist(max_n + 10, 0) {}
    // 在构造容量内设为 n 个点, 工作表由 run 清空
    // 时间 O(1) | 额外空间 O(1)
    void init(int _n) { n = _n; }
    // 返回整图是否有负环, 每次自动清空工作表, 不修改图
    // 最坏时间 O(n(n+m)) | 额外空间 O(1)
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
/* Usage
int main()
{
    Graph<true, LL> g(4, 3);
    g.add(1, 2, 5); g.add(3, 4, -2); g.add(4, 3, 1);
    BFRing bf(4);
    bf.init(4);
    cout << bf.run(g) << '\n'; // 1, 从 1 不可达的负环也会发现
    g.clear(); g.add(1, 2, -3);
    cout << bf.run(g) << '\n'; // 0, run 自带复位
    bf.init(1); g.clear();
    cout << bf.run(g) << '\n'; // 0
}
*/
