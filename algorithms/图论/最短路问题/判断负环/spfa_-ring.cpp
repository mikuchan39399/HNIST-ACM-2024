// zoi: spfaRing
#ifndef Z_OI_SPFARING
#define Z_OI_SPFARING

#include "../../图的存储/Graph.cpp"
#include "../../../杂项/128位整数/128int.cpp"

// 全点以零距离入队, cnt 记录当前松弛路径的边数, 达到 n 即有负环, 不是累计入队次数
// Graph 用 int/LL 整数权, dist 用库内 i128 避免负环反复降低距离时溢出, 可查任意分量
// 工作表每点 24 B, 队列每项 4 B 且至多 n 项, 20 万点约 5.6 MB, 不含 Graph
struct SPFARing
{
    int n;
    vector<i128> dist;
    VI cnt, inq;
    // 分配 max_n 个点的工作表, 点编号为 1..n, n >= 1
    // 时间 O(max_n) | 空间 O(max_n)
    SPFARing(int max_n = 0) : n(0), dist(max_n + 10, 0), cnt(max_n + 10, 0), inq(max_n + 10, 0) {}
    // 在构造容量内设为 n 个点, 工作表由 run 清空
    // 时间 O(1) | 额外空间 O(1)
    void init(int _n) { n = _n; }
    // 返回整图是否有负环, 每次自动清空工作表, 不修改图
    // 最坏时间 O(nm+n) | 额外空间 O(n)
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
