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

// 分清题目中冗余的重边和故意的重边, 只有前者需要去重
struct EBCC
{
    int n;
    int dfn_idx, ebcc_cnt;
    Graph<false, Empty> tree; // 桥树(缩点后)
    VI dfn, low, bel, sta;
    VVI ebcc_points;          // 各 EBCC 的原图点表
    EBCC(int max_n = 0) : n(max_n), dfn_idx(0), ebcc_cnt(0),
        tree(max_n, max_n),
        dfn(max_n + 10, 0), low(max_n + 10, 0), bel(max_n + 10, 0),
        ebcc_points(1, VI{})
    {
        sta.reserve(max_n + 10);
    }
    void init(int _n)
    {
        n = _n;
        tree.clear();
        z_fill_n(n, 0, dfn, low, bel);
        dfn_idx = ebcc_cnt = 0;
        sta.clear();
        ebcc_points.assign(1, VI());
    }
    // 边双; g 需无向半边结构(g[u] 遍历 + g.id/g.rev/edges)
    // 时间: O(n + m) | 空间: O(n)
    template <class G>
    void build(G& g, int _n)
    {
        n = _n;
        for (int i = 1; i <= n; i++)
            if (!dfn[i]) tarjan(g, i, -1);
    }
    // 桥树, 每个 EBCC 缩成一点
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
    // 桥的树方向半边 id(恒偶); 端点 = g.edges[i].v / g.edges[g.rev(i)].v, 逻辑边号 i/2
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
EBCC ebcc(N);
Graph<false> g(N, M);
ebcc.init(n); g.clear();
for (int i = 1; i <= m; i++) { int u, v; cin >> u >> v; if (u != v) g.add(u, v); }

ebcc.build(g, n);            // bel[u] = EBCC 编号, ebcc_points 成员表
VI br = ebcc.get_bridges(g); // 桥 = 树方向半边 id, 端点经 g.edges 取
ebcc.build_tree(g);          // 桥树, 点 = 1..ebcc_cnt, 可接树上算法
*/
