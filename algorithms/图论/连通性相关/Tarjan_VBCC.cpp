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

// 点双与圆方森林; cut[u] 判割点, vbcc_cir[i] 存第 i 个点双的原图点, 孤立点自成一块
// 圆点 1 .. n, 方点 n + i, 总点数最多 2 * n; 接 HLD 等算法时按 n + vbcc_cnt 建树
// 原图外置, 只读邻接 e.v; 允许重边, 自环须过滤, 递归深度最坏 n
struct VBCC
{
    int n;
    int dfn_idx, vbcc_cnt;
    Graph<false, Empty> tree;
    VI dfn, low, sta, cut;
    VVI vbcc_cir;
    // N 取原图最大点数; 圆方森林已预留 2 * N 点, 2 * N 条无向边, 原图另建 Graph(N, M)
    // 64 位 GCC 基础预留约 72 * N B (N = 2e5 约 14.4 MB), 另加成员表 24 * C + 4 * S B
    // C / S 为外层 / 各内层 vector 的容量总数; 不含原图, 分配器开销与递归栈
    // 时间 O(N) | 空间 O(N)
    VBCC(int max_n = 0) : n(max_n), dfn_idx(0), vbcc_cnt(0),
        tree(max_n * 2, max_n * 2),
        dfn(max_n + 10, 0), low(max_n + 10, 0), cut(max_n + 10, 0),
        vbcc_cir(1, VI{})
    {
        sta.reserve(max_n + 10);
    }
    // 复位本轮结果与内部图; n 不超过构造时的 N, 原图需另行 clear()
    // 时间 O(n + 上轮点数) | 额外空间 O(1)
    void init(int _n)
    {
        n = _n;
        tree.clear();
        z_fill_n(n, 0, dfn, low, cut);
        dfn_idx = vbcc_cnt = 0;
        sta.clear();
        vbcc_cir.assign(1, VI{});
    }
    // 求点双与割点; 先 init(n), root = -1 扫全图, 否则只扫 root 所在连通块
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
    // 向 tree 追加圆方森林, 每个方点连接该点双的全部圆点; 重建先 tree.clear()
    // 时间 O(n) | 额外空间 O(n)
    void build_tree()
    {
        for (int i = 1; i <= vbcc_cnt; i++)
        {
            int u = n + i;
            for (int v : vbcc_cir[i])
                tree.add(u, v);
        }
    }
    // 返回 u 所属点双编号 (1-based, 无序); 须先 build_tree(), 孤立点也返回一项
    // 时间 O(返回项数) | 额外空间 O(返回项数)
    VI get_bel_vbccs(int u)
    {
        VI res;
        for (auto& e : tree[u])
            res.push_back(e.v - n);
        return res;
    }
    // 返回第 i 个点双中的原图割点编号 (无序), i 越界返回空表; 无需 build_tree()
    // 时间 O(该点双大小) | 额外空间 O(返回项数)
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
int main()
{
    int n, m;
    cin >> n >> m;
    VBCC vbcc(n);
    Graph<false> g(n, m);
    vbcc.init(n);
    for (int i = 1; i <= m; i++)
    {
        int u, v;
        cin >> u >> v;
        if (u != v) g.add(u, v); // 过滤自环, 保留重边
    }
    vbcc.build(g, n);
    for (int i = 1; i <= vbcc.vbcc_cnt; i++)
    {
        for (int u : vbcc.vbcc_cir[i]) cout << u << ' '; // 第 i 个点双的原图点
        cout << '\n';
        for (int u : vbcc.get_cuts_vbcc(i)) cout << u << ' '; // 其中的割点
        cout << '\n';
    }
    vbcc.build_tree();
    for (int i : vbcc.get_bel_vbccs(1)) cout << i << ' '; // 点 1 所属的点双编号
    cout << '\n';
    return 0;
}
*/
