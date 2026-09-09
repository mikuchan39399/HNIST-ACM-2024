#include "../../杂项/utils/utils.cpp"
#include <sstream>

namespace legacy_floyd
{
#define main floyd_entry
#include "../最短路问题/全源最短路径/floyd.cpp"
#undef main
}

// 调用真实旧版 main, 只验本轮矩阵初始化与非负无向图; 不冒充完整 Floyd 引擎验收。
int main()
{
    mt19937 rng(42);
    for (int tc = 0; tc < 304; tc++)
    {
        int n = tc < 4 ? (tc % 2 ? 1 : 109) : 1 + rng() % 12;
        int m = tc < 4 ? n - 1 : rng() % 60;
        vector<VPII> adj(n + 1);
        ostringstream input;
        input << n << ' ' << m << '\n';
        for (int i = 0; i < m; i++)
        {
            int u = tc < 4 ? i + 1 : 1 + rng() % n;
            int v = tc < 4 ? i + 2 : 1 + rng() % n;
            int w = tc < 4 ? 1 : rng() % 30;
            input << u << ' ' << v << ' ' << w << '\n';
            adj[u].emplace_back(v, w); adj[v].emplace_back(u, w);
        }
        legacy_floyd::dp[0][0] = 123456789;
        istringstream in(input.str()); ostringstream out;
        auto old_in = cin.rdbuf(in.rdbuf()); auto old_out = cout.rdbuf(out.rdbuf());
        cin.clear();
        int code = legacy_floyd::floyd_entry();
        cin.rdbuf(old_in); cout.rdbuf(old_out); cin.clear();
        assert(code == 0 && legacy_floyd::dp[0][0] == 123456789);
        istringstream answer(out.str());
        for (int s = 1; s <= n; s++)
        {
            VI dist(n + 1, inf), used(n + 1, 0); dist[s] = 0;
            for (int k = 0; k < n; k++)
            {
                int u = 0;
                for (int v = 1; v <= n; v++) if (!used[v] && dist[v] < dist[u]) u = v;
                if (!u) break;
                used[u] = 1;
                for (auto [v, w] : adj[u]) dist[v] = min(dist[v], dist[u] + w);
            }
            for (int v = 1; v <= n; v++) { int got; assert(answer >> got); assert(got == dist[v]); }
        }
        string extra; assert(!(answer >> extra));
    }
    cout << "floyd_capacity_check passed: 304 cases, 109-1-109-1 reset, independent Dijkstra\n";
}
