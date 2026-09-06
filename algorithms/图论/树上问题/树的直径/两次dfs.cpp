// zoi: diameterDfs
#ifndef Z_OI_TREE_DIAMETER
#define Z_OI_TREE_DIAMETER

#include "../../图的存储/Graph.cpp"
#include "../../../杂项/utils/utils.cpp"

// 两次 DFS 求直径端点与路径, 仅支持非负边权, 无权边按 1 计权
// 输入为 n >= 1 的无向森林, 仅处理点 1 所在的树; pre 为第二次 DFS 的父表
// len 为路径权和, path 按 end_u 到 end_v 排列, 同长路径取遍历中先遇到的一条
// pre 每点 4 B, path 每点 4 B; n = 1e6 时有效元素合计至多约 8 MB, 递归栈另计
template <class G>
struct TreeDiameter
{
    int n;
    VI pre;
    VI path;
    int end_u, end_v;
    LL len;
    int cur_far;
    LL cur_d;
    // 重建 g 中点 1 所在树的状态, 将直径长度、端点和路径写入 len、end_u/end_v 和 path
    // 时间 O(n) | 空间 O(n), 含递归栈
    void build(G& g, int _n)
    {
        n = _n;
        pre.assign(n + 10, 0);
        cur_d = -1;
        dfs(1, 0, 0, g);
        end_u = cur_far;
        pre[end_u] = 0;
        cur_d = -1;
        dfs(end_u, 0, 0, g);
        end_v = cur_far;
        len = cur_d;
        path.clear();
        for (int u = end_v; u; u = pre[u]) path.push_back(u);
        reverse(path.begin(), path.end());
    }
private:
    template <class E>
    static LL w_of(const E& e)
    {
        if constexpr (is_same_v<decltype(e.w), Empty>) return 1;
        else return e.w;
    }
    void dfs(int u, int p, LL d, G& g)
    {
        if (d > cur_d) cur_d = d, cur_far = u;
        for (auto& e : g[u])
        {
            int v = e.v;
            if (v == p) continue;
            pre[v] = u;
            dfs(v, u, d + w_of(e), g);
        }
    }
};
#endif

/*
 * Usage:
 * // 多测: 直接再 build, 无需 init; 边权须非负, 可负用 dp 版(树形dp法.cpp)
 * Graph<false> g(n, n - 1);      // 带边权: Graph<false, LL> + g.add(u, v, w)
 * TreeDiameter<Graph<false>> td;
 * for (int i = 1; i < n; i++) { int u, v; cin >> u >> v; g.add(u, v); }
 * td.build(g, n);
 * td.len;                        // 直径长度(无权图按 1 计)
 * td.end_u; td.end_v;            // 直径两端点
 * td.path;                       // 直径路径 end_u -> end_v
 * td.pre;                        // 全体父表, 应用层重建路径/分叉用
 * // build 为递归 DFS, 深链依赖评测机栈宽
 */
