// zoi: dij
#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>

using namespace std;
using LL = long long;
using VI = vector<int>;
using VLL = vector<LL>;
using PLI = pair<LL, int>;

const LL INF = 0x3f3f3f3f3f3f3f3f;

struct Dijkstra
{
    struct node
    {};
    VLL dist;
    Dijkstra(int max_n = 0) : 
        dist(max_n + 10, INF)
    {}
    void init(int _n) 
    { 
        fill(dist.begin(), dist.begin() + _n + 10, INF); 
    }
    template <class G>
    void build(const VI& nodes, G& g)
    {
        priority_queue<PLI, vector<PLI>, greater<PLI>> heap;
        for (int s : nodes)
        {
            dist[s] = 0;
            heap.push({0, s});
        }
        while (heap.size())
        {
            auto [d, u] = heap.top();
            heap.pop();
            if (d > dist[u]) continue;
            for (auto& [v, nxt, w] : g[u])
            {
                if (dist[u] + w < dist[v])
                {
                    dist[v] = dist[u] + w;
                    heap.push({dist[v], v});
                }
            }
        }
    }
    template <class G>
    void build(int s, G& g) { build(VI{s}, g); }
};

/* Usage: 
 * const int MAXN = 1e5 + 10;
 * const int MAXM = 2e5 + 10;
 * 
 * Graph<true, LL> g(MAXN, MAXM);
 * Dijkstra dij(MAXN);
 * 
 * void solve()
 * {
 *     int n, m, s; cin >> n >> m >> s;
 *     g.clear();
 *     dij.init(n);
 *     for (int i = 1; i <= m; i++)
 *     {
 *         int u, v; LL w;
 *         cin >> u >> v >> w;
 *         g.add(u, v, w);
 *     }
 *     // 跑以 s 为起点的单源最短路
 *     dij.build(s, g);
 *     for (int i = 1; i <= n; i++)
 *     {
 *         // P3371/P4779 输出格式, 无法到达输出 2^31-1 
 *         if (dij.dist[i] == INF) 
 *         {
 *             cout << (1LL << 31) - 1 << " ";
 *         }
 *         else 
 *         {
 *             cout << dij.dist[i] << " ";
 *         }
 *     }
 *     cout << '\n';
 * }
 */