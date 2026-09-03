// zoi: spfaRing
#ifndef Z_OI_SPFARING
#define Z_OI_SPFARING

#include <queue>
#include "../../图的存储/Graph.cpp"
#include "../../../杂项/utils/utils.cpp"

using namespace std;

// ============ SPFA 判负环 ============
// 全源种子(全部点入队, dist 全 0), 判据 = 最短路树边数 cnt >= n,
// 与起点可达性无关
// 内存: dist 8B/点 + cnt/inq 各 4B/点; 预算 = max_n
struct SPFARing
{
    int n;
    VLL dist;
    VI cnt, inq;
    // 构造: 预算 max_n
    // 时间: O(n) | 空间: 16B/点
    SPFARing(int max_n = 0) : n(0), dist(max_n + 10, 0), cnt(max_n + 10, 0), inq(max_n + 10, 0) {}
    // 多测复位: n 重配
    // 时间: O(1) | 空间: O(1)
    void init(int _n) { n = _n; }
    // 判整图是否有负环, 返回 true = 有
    // 时间: 最坏 O(nm) | 空间: O(n)
    template <class G>
    bool run(G& g)
    {
        z_fill_n(n, 0, dist);
        z_fill_n(n, 0, cnt);
        z_fill_n(n, 0, inq);
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
/*
 * Usage:
 * Graph<true, LL> g{n, m};
 * SPFARing sp{n};
 * sp.init(n);
 * for (int i = 1; i <= m; i++) { int u, v; LL w; cin >> u >> v >> w; g.add(u, v, w); }
 * sp.run(g);                         // true = 图里有负环
 * // 多测: g.clear(); sp.init(n); 重跑
 */
