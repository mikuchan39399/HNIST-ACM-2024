// zoi: topo
#ifndef Z_OI_TOPO_SORT
#define Z_OI_TOPO_SORT

#include "../图的存储/Graph.cpp"
#include "../../杂项/utils/utils.cpp"

// Kahn 拓扑排序, 图中边端点须在 1 到 n 内, in 保存 0 到 n 的入度副本, ord 为出队顺序
// 不改原图, 多测只处理本轮点数; 两个容器保留历史容量, 每点约 8 B, 最大 n = 1e6 时约 8 MB
struct TopoSort
{
    VI in;
    VI ord;
    // 生成 g 中 1 到 n 的拓扑序, 无环返回 true, 有环返回 false 并保留已出队部分; n = 0 返回 true
    // 时间 O(n + m) | 本轮有效空间 O(n), 容器保留历史容量, m 为本轮边数
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
    // 返回出队序列的引用, 仅 build 返回 true 时包含全部 n 个点
    // 时间 O(1) | 空间 O(1)
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
