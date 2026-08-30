// zoi: topo
#ifndef Z_OI_TOPO_SORT
#define Z_OI_TOPO_SORT

#include <vector>
#include "../图的存储/Graph.cpp"
#include "../../杂项/utils/utils.cpp"

using namespace std;

// ============ 拓扑排序 (Kahn) ============
// 仅接受有向图 Graph<true, ...>;
// build 返回 false = 有环; true = DAG, 无任何环,
// 负权边的最短路/DP 可直接按 get() 的顺序松弛
template <class G>
struct TopoSort
{
    VI in;   // 入度副本, 不动原图
    VI ord;  // 拓扑序
    // 返回值: true = DAG(无环) | false = 有环
    // 时间: O(n + m) | 空间: O(n)
    bool build(G& g, int n)
    {
        in = g.in_deg;
        ord.clear();
        ord.reserve(n);
        for (int u = 1; u <= n; u++)
            if (in[u] == 0) ord.push_back(u);
        for (size_t i = 0; i < ord.size(); i++)
            for (auto& e : g[ord[i]])
                if (--in[e.v] == 0) ord.push_back(e.v);
        return (int)ord.size() == n;
    }
    // 取拓扑序 (build 返回 true 时才有意义)
    // 时间: O(1) | 空间: O(1)
    VI& get() { return ord; }
};
#endif

/*
 * Usage:
 * Graph<true> g(n, m);
 * TopoSort<Graph<true>> ts;
 * for (int i = 1; i <= m; i++) { int u, v; cin >> u >> v; g.add(u, v); }
 * if (!ts.build(g, n)) { cout << "-1" << '\n'; }  // 有环
 * for (int u : ts.get()) cout << u << ' ';
 * // DAG 上带负权 DP: 按 ts.get() 顺序松弛即可, 无环保证无负环
 */
