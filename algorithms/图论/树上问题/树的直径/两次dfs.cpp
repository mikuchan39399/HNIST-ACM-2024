// zoi: diameterDfs
#ifndef Z_OI_TREE_DIAMETER
#define Z_OI_TREE_DIAMETER

#include <vector>
#include <algorithm>
#include <type_traits>
#include "../../图的存储/Graph.cpp"
#include "../../../杂项/utils/utils.cpp"

using namespace std;

// ============ 树的直径 (两次 DFS, 提供端点与路径) ============
// 仅支持非负边权, 可负边权请用 dp 版(树形dp法.cpp)
// W 为 Empty 时按 1 计权; 森林仅算节点 1 所在的树
// 内存: pre 4B/点 + path 动态
template <class G>
struct TreeDiameter
{
    int n;
    VI pre;    // 父节点, 重建路径用
    VI path;   // 直径路径 end_u -> end_v
    int end_u, end_v;
    LL len;
    VI cur_far;
    VLL cur_d;
    // 对 1..n 的树 g 两次 DFS; 多测直接重跑即可全量自复位
    // 时间: O(n) | 空间: O(n)
    void build(G& g, int _n)
    {
        n = _n;
        pre.assign(n + 10, 0);
        cur_far.assign(n + 10, 0);
        cur_d.assign(n + 10, 0);
        cur_d[0] = -1;
        dfs(1, 0, 0, g);
        end_u = cur_far[0];
        pre[end_u] = 0;
        cur_d[0] = -1;
        dfs(end_u, 0, 0, g);
        end_v = cur_far[0];
        len = cur_d[0];
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
        if (d > cur_d[0]) cur_d[0] = d, cur_far[0] = u;
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
