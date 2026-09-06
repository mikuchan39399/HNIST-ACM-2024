// zoi: diameterDp
#ifndef Z_OI_TREE_DIAMETER_DP
#define Z_OI_TREE_DIAMETER_DP

#include "../../图的存储/Graph.cpp"
#include "../../../杂项/utils/utils.cpp"

// 树形 DP 求直径长度, 支持负边权, 无权边按 1 计权, 允许单点路径且长度为 0
// 输入为 n >= 1 的无向森林, 仅处理点 1 所在的树; down 为从各点向下的最大路径权和
// 每点 8 B; n = 1e6 时约 8 MB, 递归栈另计
template <class G>
struct TreeDiameterDP
{
    int n;
    LL len;
    VLL down;
    // 重建 g 中点 1 所在树的状态并返回直径长度, 全负边权时返回 0
    // 时间 O(n) | 空间 O(n), 含递归栈
    LL build(G& g, int _n)
    {
        n = _n;
        down.assign(n + 10, 0);
        len = 0;
        dfs(1, 0, g);
        return len;
    }
private:
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
