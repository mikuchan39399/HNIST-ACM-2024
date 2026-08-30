// zoi: diameterDp
#ifndef Z_OI_TREE_DIAMETER_DP
#define Z_OI_TREE_DIAMETER_DP

#include <vector>
#include <algorithm>
#include <type_traits>
#include "../../图的存储/Graph.cpp"
#include "../../../杂项/utils/utils.cpp"

using namespace std;

// ============ 树的直径 (树形 dp, 仅求长度) ============
// 支持负边权(与两次 DFS 版的分界); W 为 Empty 时按 1 计权
// 森林仅算节点 1 所在的树
template <class G>
struct TreeDiameterDP
{
    int n;
    LL len;
private:
    VLL down;
    template <class E>
    static LL w_of(const E& e)
    {
        if constexpr (is_same_v<decltype(e.w), Empty>) return 1;
        else return e.w;
    }
    void dfs(int u, int p, G& g)
    {
        LL top1 = 0, top2 = 0;
        for (auto& e : g[u])
        {
            int v = e.v;
            if (v == p) continue;
            dfs(v, u, g);
            LL d = down[v] + w_of(e);
            if (d > top1) { top2 = top1; top1 = d; }
            else if (d > top2) top2 = d;
        }
        down[u] = top1;
        len = max(len, top1 + top2);
    }
public:
    // 对 1..n 的树 g 树形 dp 并返回直径长度; 多测直接重跑即可全量自复位
    // 时间: O(n) | 空间: O(n)
    LL build(G& g, int _n)
    {
        n = _n;
        down.assign(n + 10, 0);
        len = 0;
        dfs(1, 0, g);
        return len;
    }
};
#endif

/*
 * Usage:
 * // 可负边权时用本版; 要端点/路径(非负权)用两次 DFS 版
 * Graph<false, LL> g(n, n - 1);
 * TreeDiameterDP<Graph<false, LL>> dp;
 * for (int i = 1; i < n; i++) { int u, v; LL w; cin >> u >> v >> w; g.add(u, v, w); }
 * cout << dp.build(g, n);
 * // build 为递归 DFS, 深链依赖评测机栈宽
 */
