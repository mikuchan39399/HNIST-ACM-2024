// zoi: bfRing
#ifndef Z_OI_BFRING
#define Z_OI_BFRING

#include "../../图的存储/Graph.cpp"
#include "../../../杂项/128位整数/128int.cpp"

// 判断整图负环(含不连通分量), 1-based, int/LL 边权; dist 不作为最短路答案. 空间 O(n)
struct BFRing
{
    int n;
    vector<i128> dist;
    // 预留 max_n 个点. 时空 O(max_n)
    BFRing(int max_n = 0) : n(0), dist(max_n + 10, 0) {}
    // 设置 n<=容量, run 再清状态. O(1)
    void init(int _n) { n = _n; }
    // 有负环返回 true, 自动复位. 最坏时间 O(n(n+m)), 额外空间 O(1)
    template <class G>
    bool run(G& g, int _n)
    {
        init(_n);
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
            if (!flag) return false;
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

    cout << bf.run(g, 4) << '\n'; // 1, 从 1 不可达的负环也会发现
    g.clear(); g.add(1, 2, -3);
    cout << bf.run(g, 4) << '\n'; // 0, run 自带复位
     g.clear();
    cout << bf.run(g, 1) << '\n'; // 0
}
*/
