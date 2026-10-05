// zoi: spfaRing
#ifndef Z_OI_SPFARING
#define Z_OI_SPFARING

#include "../../图的存储/Graph.cpp"
#include "../../../杂项/128位整数/128int.cpp"

// 判断整图负环(含不连通分量), 1-based, int/LL 边权. 空间 O(n)
// cnt 为松弛路径边数, dist 不作为最短路答案
struct SPFARing
{
    int n;
    vector<i128> dist;
    VI cnt, inq;
    // 预留 max_n 个点. 时空 O(max_n)
    SPFARing(int max_n = 0) : n(0), dist(max_n + 10, 0), cnt(max_n + 10, 0), inq(max_n + 10, 0) {}
    // 设置 n<=容量, run 再清状态. O(1)
    void init(int _n) { n = _n; }
    // 有负环返回 true, 自动复位. 最坏时间 O(nm+n), 队列空间 O(n)
    template <class G>
    bool run(G& g, int _n)
    {
        init(_n);
        z_fill_n(n, 0, dist, cnt, inq);
        queue<int> q;
        for (int i = 1; i <= n; i++) { q.push(i); inq[i] = 1; }
        while (q.size())
        {
            int u = q.front();
            q.pop();
            inq[u] = 0;
            for (auto& [v, nxt, w] : g[u])
                if (dist[u] + w < dist[v])
                {
                    dist[v] = dist[u] + w;
                    cnt[v] = cnt[u] + 1;
                    if (cnt[v] >= n) return true;
                    if (!inq[v]) { q.push(v); inq[v] = 1; }
                }
        }
        return false;
    }
};
#endif
/* Usage
int main()
{
    Graph<true, LL> g(4, 3);
    g.add(1, 2, 5); g.add(3, 4, -2); g.add(4, 3, 1);
    SPFARing sp(4);

    cout << sp.run(g, 4) << '\n'; // 1, 从 1 不可达的负环也会发现
    g.clear(); g.add(1, 2, -3);
    cout << sp.run(g, 4) << '\n'; // 0, run 自带复位
     g.clear();
    cout << sp.run(g, 1) << '\n'; // 0
}
*/
