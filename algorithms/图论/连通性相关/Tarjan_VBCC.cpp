// zoi: vbcc
#ifndef Z_OI_VBCC
#define Z_OI_VBCC

#include <vector>
#include <algorithm>
#include <iostream>
#include <cassert>
#include "../图的存储/Graph.cpp"
#include "../../杂项/utils/utils.cpp"

using namespace std;

struct VBCC
{
    int n;
    int dfn_idx, vbcc_cnt;
    Graph<false, Empty> tree;   // 圆方树, 方点 = n+1..n+vbcc_cnt
    VI dfn, low, sta, cut;
    VVI vbcc_cir;               // 各 VBCC 的圆点表
    VBCC(int max_n = 0) : n(max_n), dfn_idx(0), vbcc_cnt(0),
        tree(max_n * 2, max_n * 2),
        dfn(max_n + 10, 0), low(max_n + 10, 0), cut(max_n + 10, 0),
        vbcc_cir(1, VI{})
    {
        sta.reserve(max_n + 10);
    }
    void init(int _n)
    {
        n = _n;
        tree.clear();
        z_fill_n(n, 0, dfn, low, cut);
        dfn_idx = vbcc_cnt = 0;
        sta.clear();
        vbcc_cir.assign(1, VI{});
    }
    // 点双+割点; g 任意无向邻接(只读 e.v); root = -1 扫全图
    // 时间: O(n + m) | 空间: O(n)
    template <class G>
    void build(G& g, int _n, int root = -1)
    {
        n = _n;
        if (root != -1)
        {
            tarjan(g, root, root);
            return;
        }
        for (int i = 1; i <= n; i++)
        {
            if (!dfn[i])
            {
                tarjan(g, i, i);
            }
        }
    }
    // 构建圆方树
    void build_tree()
    {
        for (int i = 1; i <= vbcc_cnt; i++)
        {
            int u = n + i;
            for (int v : vbcc_cir[i])
                tree.add(u, v);
        }
    }
    // u 参与的 VBCC 编号表
    VI get_bel_vbccs(int u)
    {
        VI res;
        for (auto& e : tree[u])
            res.push_back(e.v - n);
        return res;
    }
    // 第 i 个 VBCC 的割点表
    VI get_cuts_vbcc(int i)
    {
        VI res;
        if (i < 1 || i > vbcc_cnt) return res;
        for (int v : vbcc_cir[i])
        {
            if (cut[v]) res.push_back(v);
        }
        return res;
    }
private:
    template <class G>
    void tarjan(G& g, int u, int root)
    {
        dfn_idx++;
        dfn[u] = low[u] = dfn_idx;
        sta.push_back(u);
        int child_cnt = 0;
        for (auto& e : g[u])
        {
            int v = e.v;
            if (!dfn[v])
            {
                child_cnt++;
                tarjan(g, v, root);
                low[u] = min(low[u], low[v]);
                if (low[v] >= dfn[u])
                {
                    if (u != root) cut[u] = 1;
                    vbcc_cnt++;
                    vbcc_cir.push_back(VI{});
                    int t;
                    do
                    {
                        t = sta.back();
                        sta.pop_back();
                        vbcc_cir[vbcc_cnt].push_back(t);
                    } while (t != v);
                    vbcc_cir[vbcc_cnt].push_back(u);
                }
            }
            else low[u] = min(low[u], dfn[v]);
        }
        if (u == root && child_cnt >= 2) cut[u] = 1;
        // 孤立点自成 VBCC
        if (u == root && child_cnt == 0)
        {
            vbcc_cnt++;
            vbcc_cir.push_back(VI{u});
        }
    }
};

#endif
// Usage:
/*
VBCC vbcc(N);
Graph<false> g(N, M);
vbcc.init(n); g.clear();
for (int i = 1; i <= m; i++) { int u, v; cin >> u >> v; if (u != v) g.add(u, v); }

vbcc.build(g, n);   // cut[u] = 割点, vbcc_cir 成员表
vbcc.build_tree();  // 圆方树: 方点 n+i = 第 i 个 VBCC, 树上数组开 2n
// 反查: get_bel_vbccs(u) / get_cuts_vbcc(i)
*/
