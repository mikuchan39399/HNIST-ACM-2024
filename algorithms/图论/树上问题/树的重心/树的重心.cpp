// zoi: centroid
#ifndef Z_OI_TREE_CENTROID
#define Z_OI_TREE_CENTROID

#include <vector>
#include <algorithm>
#include <limits>
#include "../../图的存储/Graph.cpp"
#include "../../../杂项/utils/utils.cpp"

using namespace std;

// ============ 树的重心 (带点权) ============
// 重心 = 删除后最大连通块权最小的点; 最多 2 个
// 支持零/负点权; 森林仅算节点 1 所在的树
template <class G>
struct TreeCentroid
{
    int n;
    VLL pt;   // 点权, 默认 1, 建树前直填
    VLL sz;   // 子树权和, build 后有效
    VI centroids;    // 全体重心, 升序
    LL min_max_part; // 删重心后最大连通块的权
    LL total;
    TreeCentroid(int max_n = 0) { init(max_n); }
    // 多测复位点权为 1 (其余 build 自复位)
    // 时间: O(n) | 空间: O(n)
    void init(int _n)
    {
        n = _n;
        pt.assign(n + 10, 1);
        sz.assign(n + 10, 0);
    }
    // 对 1..n 的树 g 求全体重心
    // 时间: O(n) | 空间: O(n)
    void build(G& g, int _n)
    {
        n = _n;
        if ((int)pt.size() < n + 10) pt.assign(n + 10, 1);
        total = 0;
        for (int i = 1; i <= n; i++) total += pt[i];
        sz.assign(n + 10, 0);
        centroids.clear();
        min_max_part = numeric_limits<LL>::max();
        dfs(1, 0, g);
        sort(centroids.begin(), centroids.end());
    }
private:
    void dfs(int u, int p, G& g)
    {
        sz[u] = pt[u];
        LL mx = numeric_limits<LL>::min();   // 负点权下所有块可为负, 不能拿 0 当哨兵
        for (auto& e : g[u])
        {
            int v = e.v;
            if (v == p) continue;
            dfs(v, u, g);
            sz[u] += sz[v];
            mx = max(mx, sz[v]);
        }
        mx = max(mx, total - sz[u]);   // up 块在子树累加完成后算
        if (mx < min_max_part) min_max_part = mx, centroids = {u};
        else if (mx == min_max_part) centroids.push_back(u);
    }
};
#endif

/*
 * Usage:
 * // 多测: 有点权改动时 init(n) 复位再直填 pt
 * Graph<false> g(n, n - 1);
 * TreeCentroid<Graph<false>> tc(n);
 * // tc.pt[i] = w[i];             // 带点权时直填, 不填默认全 1
 * for (int i = 1; i < n; i++) { int u, v; cin >> u >> v; g.add(u, v); }
 * tc.build(g, n);
 * tc.centroids;                  // 重心列表(最多 2 个)
 * tc.min_max_part;               // 删重心后最大连通块的权
 * tc.sz;                         // 子树权和(以 1 为根)
 * // build 为递归 DFS, 深链依赖评测机栈宽
 */
