// zoi: ebcc
#ifndef Z_OI_EBCC
#define Z_OI_EBCC

#include <vector>
#include <algorithm>
#include <iostream>
#include <cassert>
#include "../图的存储/Graph.cpp"
#include "../../杂项/utils/utils.cpp"

using namespace std;

// 边双与桥森林; bel[u] 为 1 .. ebcc_cnt, ebcc_points[i] 存原图点, tree 点号为分量号
// 原图外置, 需 g[u], id / rev / edges 半边接口; 允许重边与自环, 递归深度最坏 n
struct EBCC
{
    int n;
    int dfn_idx, ebcc_cnt;
    Graph<false, Empty> tree;
    VI dfn, low, bel, sta;
    VVI ebcc_points;
    // N 取原图最大点数; 内部桥森林已预留 N 点, N 条无向边, 原图另建 Graph(N, M)
    // 64 位 GCC 基础预留约 44 * N B (N = 2e5 约 8.8 MB), 另加成员表 24 * C + 4 * S B
    // C / S 为外层 / 各内层 vector 的容量总数; 不含原图, 分配器开销与递归栈
    // 时间 O(N) | 空间 O(N)
    EBCC(int max_n = 0) : n(max_n), dfn_idx(0), ebcc_cnt(0),
        tree(max_n, max_n),
        dfn(max_n + 10, 0), low(max_n + 10, 0), bel(max_n + 10, 0),
        ebcc_points(1, VI{})
    {
        sta.reserve(max_n + 10);
    }
    // 复位本轮结果与内部图; n 不超过构造时的 N, 原图需另行 clear()
    // 时间 O(n + 上轮点数) | 额外空间 O(1)
    void init(int _n)
    {
        n = _n;
        tree.clear();
        z_fill_n(n, 0, dfn, low, bel);
        dfn_idx = ebcc_cnt = 0;
        sta.clear();
        ebcc_points.assign(1, VI());
    }
    // 求 1 .. n 的边双, 结果写入 bel 与 ebcc_points; 每轮先 init(n)
    // 时间: O(n + m) | 空间: O(n)
    template <class G>
    void build(G& g, int _n)
    {
        n = _n;
        for (int i = 1; i <= n; i++)
            if (!dfn[i]) tarjan(g, i, -1);
    }
    // 向 tree 追加桥森林, 点号为 bel; 每轮 build 后调用一次, 重建先 tree.clear()
    // 时间 O(m) | 额外空间 O(n)
    template <class G>
    void build_tree(G& g)
    {
        for (size_t i = 0; i < g.edges.size(); i += 2)
        {
            int u = g.edges[g.rev(i)].v;
            int v = g.edges[i].v;
            if (bel[u] != bel[v]) tree.add(bel[u], bel[v]);
        }
    }
    // 返回桥的偶数半边号 i(0-based, 不保证 DFS 方向); 端点为 edges[i].v 与 edges[i^1].v
    // 按 i 递增, 空表表示无桥; 逻辑边号 i/2, 时间 O(m) | 额外空间 O(桥数)
    template <class G>
    VI get_bridges(G& g)
    {
        VI bridges;
        for (size_t i = 0; i < g.edges.size(); i += 2)
        {
            int u = g.edges[g.rev(i)].v;
            int v = g.edges[i].v;
            if (bel[u] != bel[v]) bridges.push_back((int)i);
        }
        return bridges;
    }
private:
    template <class G>
    void tarjan(G& g, int u, int in_edge)
    {
        dfn_idx++;
        dfn[u] = low[u] = dfn_idx;
        sta.push_back(u);
        for (auto& e : g[u])
        {
            int i = g.id(e);
            if (in_edge == g.rev(i)) continue;
            int v = e.v;
            if (!dfn[v])
            {
                tarjan(g, v, i);
                low[u] = min(low[u], low[v]);
            }
            else low[u] = min(low[u], dfn[v]);
        }
        if (low[u] == dfn[u])
        {
            ebcc_cnt++;
            ebcc_points.push_back(VI());
            int t;
            do
            {
                t = sta.back();
                sta.pop_back();
                bel[t] = ebcc_cnt;
                ebcc_points[ebcc_cnt].push_back(t);
            } while (t != u);
        }
    }
};

#endif
// Usage:
/*
int main()
{
    int n, m;
    cin >> n >> m;
    EBCC ebcc(n);
    Graph<false> g(n, m);
    ebcc.init(n);
    for (int i = 1; i <= m; i++)
    {
        int u, v;
        cin >> u >> v;
        g.add(u, v);
    }
    ebcc.build(g, n);
    for (int u = 1; u <= n; u++) cout << ebcc.bel[u] << ' '; // 输出各点的边双编号
    cout << '\n';
    for (int i : ebcc.get_bridges(g))
        cout << g.edges[i ^ 1].v << ' ' << g.edges[i].v << '\n'; // 输出桥的原图端点
    ebcc.build_tree(g); // ebcc.tree 是桥森林, 点号为 1 .. ebcc_cnt
    return 0;
}
*/
