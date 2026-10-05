// zoi: spfa
#ifndef Z_OI_SPFA
#define Z_OI_SPFA

#include "../../图的存储/Graph.cpp"
#include "../../../杂项/utils/utils.cpp"

// 可负整数权最短路, 1-based; 源点可达部分不能有负环
// dist 不可达为 INF, 有限距离<INF, 候选加法在 LL 内. 空间 O(n)
struct SPFA
{
    int n;
    VLL dist;
    VI inq;
    // 预留 max_n 个点. 时空 O(max_n)
    SPFA(int max_n = 0) : n(0), dist(max_n + 10, INF), inq(max_n + 10, 0) {}
    // 清空 n<=容量 个点的状态; run 自动调用. 时间 O(n)
    void init(int _n)
    {
        n = _n;
        z_fill_n(_n, INF, dist);
        z_fill_n(_n, 0, inq);
    }
    // 从 s 重算 dist, 自动复位. 最坏时间 O(nm+n), 队列空间 O(n)
    template <class G>
    void run(int s, G& g, int _n)
    {
        init(_n);
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
    sp.run(1, g, 4);
    cout << sp.dist[3] << ' ' << (sp.dist[4] == INF) << '\n'; // -3 1
    g.clear(); g.add(2, 1, -7);
    sp.run(2, g, 2);
    cout << sp.dist[1] << '\n'; // -7
}
*/
