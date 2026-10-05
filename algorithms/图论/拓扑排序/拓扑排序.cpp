// zoi: topo
#ifndef Z_OI_TOPO_SORT
#define Z_OI_TOPO_SORT

#include "../图的存储/Graph.cpp"
#include "../../杂项/utils/utils.cpp"

// Kahn 拓扑排序, 点号 1..n; in 为入度副本, ord 为出队序. 空间 O(n)
struct TopoSort
{
    VI in;
    VI ord;
    // 重建拓扑序, 无环返回 true; 有环返回 false, ord 保留部分结果. 时间 O(n+m)
    template <class G>
    bool build(G& g, int n)
    {
        in.assign(g.in_deg.begin(), g.in_deg.begin() + n + 1);
        ord.clear();
        ord.reserve(n);
        for (int u = 1; u <= n; u++)
            if (in[u] == 0) ord.push_back(u);
        for (size_t i = 0; i < ord.size(); i++)
            for (auto& e : g[ord[i]])
                if (--in[e.v] == 0) ord.push_back(e.v);
        return (int)ord.size() == n;
    }
    // 出队序列的引用, build 成功时才含全部点. O(1)
    VI& get() { return ord; }
};
#endif

/*
 * Usage:
 * Graph<true> g(n, m);
 * TopoSort ts;
 * for (int i = 1; i <= m; i++) { int u, v; cin >> u >> v; g.add(u, v); }
 * if (!ts.build(g, n)) cout << "-1" << '\n';  // 有环
 * else for (int u : ts.get()) cout << u << ' ';
 * // DAG 上带负权 DP: 按 ts.get() 顺序松弛即可, 无环保证无负环
 * g.clear();
 * ts.build(g, 0);             // 空图返回 true, get() 为空; 多测按本轮点数重建
 */
