// zoi: centroid
#ifndef Z_OI_TREE_CENTROID
#define Z_OI_TREE_CENTROID

#include "../../图的存储/Graph.cpp"
#include "../../../杂项/utils/utils.cpp"

// 最小化删点后最大连通块权和, n>=1 的无向树; pt 为点权(默认 1, 可负)
// centroids 为升序答案, min_max_part 为最优值(单点树为 0), sz 为子树权和
template <class G>
struct TreeCentroid
{
    int n;
    VLL pt;
    VLL sz;
    VI centroids;
    LL min_max_part;
    LL total;
    // 预留 max_n 个点, 点权置 1. 时空 O(max_n)
    TreeCentroid(int max_n = 0) { init(max_n); }
    // 重设点数, 点权全部置 1; 结果待 build 更新. 时空 O(n)
    void init(int _n)
    {
        n = _n;
        pt.assign(n + 10, 1);
        sz.assign(n + 10, 0);
    }
    // 按 pt 重建全部答案; pt 容量不足时全部重置为 1
    // 时间 O(n+c log c), 空间/递归深度 O(n), c 为答案数
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
        LL mx = numeric_limits<LL>::min();
        for (auto& e : g[u])
        {
            int v = e.v;
            if (v == p) continue;
            dfs(v, u, g);
            sz[u] += sz[v];
            mx = max(mx, sz[v]);
        }
        if (p) mx = max(mx, total - sz[u]);
        if (n == 1) mx = 0;
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
 * tc.centroids;                  // 全部最优删点, 零/负点权下可能超过 2 个
 * tc.min_max_part;               // 删重心后最大连通块的权
 * tc.sz;                         // 子树权和(以 1 为根)
 * // build 为递归 DFS, 深链依赖评测机栈宽
 */
