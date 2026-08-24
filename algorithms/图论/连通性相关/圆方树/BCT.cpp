#include <vector>
#include <algorithm>
#include <iostream>
#include <cassert>
#include "../../图的存储/Graph.cpp"

using namespace std;

using VI = vector<int>;
using VVI = vector<vector<int>>;

#ifndef Z_OI_ZFILLN
#define Z_OI_ZFILLN
template<typename... CS>
void z_fill_n(int n, int val, CS&... cs) 
{
    assert(((((int)cs.size()) >= n) && ...));
    (fill(cs.begin(), cs.begin() + min((size_t)(n + 10), cs.size()), val), ...);
}
#endif
struct VBCC
{
    int n;
    int dfn_idx, vbcc_cnt;
    Graph<false, Empty> g;      // 原图
    Graph<false, Empty> tree;   // 圆方树
    VI dfn, low, sta, cut;
    VVI vbcc_cir;               // 存储每个 VBCC 所包含的所有圆点
    VBCC(int max_n = 0, int max_m = 0) : n(max_n), dfn_idx(0), vbcc_cnt(0), 
        g(max_n, max_m), tree(max_n * 2, max_n * 2), 
        dfn(max_n + 10, 0), low(max_n + 10, 0), cut(max_n + 10, 0), 
        vbcc_cir(1, VI{})
    {
        sta.reserve(max_n + 10);
    }
    void init(int _n)
    {
        n = _n;
        g.clear();
        tree.clear();
        z_fill_n(n, 0, dfn, low, cut);
        dfn_idx = vbcc_cnt = 0;
        sta.clear();
        vbcc_cir.assign(1, VI{});
    }
    void add_edge(int u, int v) { g.add(u, v); }
    void tarjan(int u, int root)
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
                tarjan(v, root);
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
        if (u == root && child_cnt == 0)
        {
            vbcc_cnt++;
            vbcc_cir.push_back(VI{u});
        }
    }
    void build(int root = -1)
    {
        if (root != -1)
        {
            tarjan(root, root);
            return;
        }
        for (int i = 1; i <= n; i++)
            if (!dfn[i]) tarjan(i, i);
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
    // 返回单个割点参与的 VBCC 列表
    VI get_bel_vbccs(int u)
    {
        VI res;
        for (auto& e : tree[u])
            res.push_back(e.v - n); 
        return res;
    }
    // 返回单个 VBCC 包含的割点列表
    VI get_cuts_vbcc(int i)
    {
        VI res;
        if (i < 1 || i > vbcc_cnt) return res;
        for (int v : vbcc_cir[i]) 
            if (cut[v]) res.push_back(v);
        return res;
    }
};

/* Usage: 
 * =====================================================================================
 * Block-Cut Tree (圆方树) 
 * 
 * [拓扑结构]
 * - 圆点 (Round) : 原图节点，编号 1 ~ n。
 * - 方点 (Square): VBCC，编号 n+1 ~ n+vbcc_cnt。
 * - 边权性质     : 圆方树为二分图。连边必为 (圆, 方)，无 (圆, 圆) 或 (方, 方)。
 * - 空间警告     : 节点总数极值 2N-1 (原图为树时)，所有树上数组(head, sz, fa) 必开 2 倍！
 * 
 * [连通与转化]
 * - 割点判定 : 圆点度数 >= 2  <=> 该点为割点。
 * - 必经点   : 原图 u->v 路径上的所有必经点 == 圆方树 u->v 简单路径上的所有【圆点】。
 * - 割边性质 : 原图中的孤立割边，对应大小为 2 的 VBCC (2个圆点 + 1个方点)。
 * - 内部路径 : 大小 >=3 的 VBCC 内，任取三点 a,b,c，必存在简单路径 a -> b -> c。
 * 
 * [树上维护高频 Trick (树剖 / LCT)]
 * - 菊花图退化 : 若用方点维护其包含的所有圆点，改圆点时更新周围方点会导致 O(N) 退化。
 * - 【标准解法】: 方点仅维护其在圆方树上的【子节点(圆点)】信息。
 * - 【LCA 特判】: 树上查询 u->v 时，若 LCA 为方点，必须额外并入 LCA 的父节点(圆点)贡献！
 * =====================================================================================
 */