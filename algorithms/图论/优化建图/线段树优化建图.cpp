// zoi: segGraph
#ifndef Z_OI_SEG_GRAPH
#define Z_OI_SEG_GRAPH

#include "../图的存储/Graph.cpp"
#include "../../杂项/utils/utils.cpp"

using namespace std;

// ============ 线段树优化建图 (区间连边压成 O(log n) 条) ============
// 出树边父->子 0 权, u->[l,r] 一条边进覆盖点, 顺骨架漏到区间每个叶子;
// 入树边子->父 0 权, [l,r]->v 区间叶子汇流上覆盖点, 一条边出去;
// 两树叶子=原点共用 id, 点既是源又是汇, 骨架自身不产生点->点的免费捷径
// 内存: 结点实拿 3n-2(池 4n) + 树孩子表 8B/结点 + Graph 边 16B/条;
//   边 = 4n 条树边 + 每次区间操作 <= 2*ceil(log2 n) 条; n=q=1e5 ≈ 67MB
struct SegGraph
{
    int n, tot;
    Graph<true, LL> g;
    // 构造: 预算 max_n 个原点(结点池 4n) / max_m 条边
    // 时间: O(n) | 空间: 结点池 4n + 边池 m
    SegGraph(int max_n = 0, int max_m = 0) : n(0), tot(max_n),
        g(4 * max_n + 10, max_m), tlc(4 * max_n + 10, 0), trc(4 * max_n + 10, 0)
    {}
    // 建骨架: 两棵 0 权树覆盖点 1..n(叶子即原点), 重复调用自动清图
    // 时间: O(n) | 空间: O(1)
    void build(int _n)
    {
        n = _n;
        tot = n;
        g.clear();
        rt_out = build_out(1, n);
        rt_in = build_in(1, n);
    }
    // 普通单向边 u->v 权 w
    // 时间: O(1) | 空间: O(1)
    void add_edge(int u, int v, LL w) { g.add(u, v, w); }
    // u -> [l,r] 全体, 权 w
    // 时间: O(log n) | 空间: O(1)
    void add_out(int u, int l, int r, LL w) { link_out(rt_out, 1, n, l, r, u, w); }
    // [l,r] 全体 -> v, 权 w
    // 时间: O(log n) | 空间: O(1)
    void add_in(int l, int r, int v, LL w) { link_in(rt_in, 1, n, l, r, v, w); }
private:
    int rt_out, rt_in;
    VI tlc, trc;
    int build_out(int l, int r)
    {
        if (l == r) return l;          // 叶子 = 原点 id, 不另开
        int p = ++tot, mid = (l + r) >> 1;
        tlc[p] = build_out(l, mid);
        trc[p] = build_out(mid + 1, r);
        g.add(p, tlc[p], 0);           // 父->子, 值顺骨架下漏
        g.add(p, trc[p], 0);
        return p;
    }
    int build_in(int l, int r)
    {
        if (l == r) return l;
        int p = ++tot, mid = (l + r) >> 1;
        tlc[p] = build_in(l, mid);
        trc[p] = build_in(mid + 1, r);
        g.add(tlc[p], p, 0);           // 子->父, 值汇流上行
        g.add(trc[p], p, 0);
        return p;
    }
    void link_out(int p, int l, int r, int lo, int hi, int u, LL w)
    {
        if (lo <= l && r <= hi) { g.add(u, p, w); return; }
        int mid = (l + r) >> 1;
        if (lo <= mid) link_out(tlc[p], l, mid, lo, hi, u, w);
        if (hi > mid) link_out(trc[p], mid + 1, r, lo, hi, u, w);
    }
    void link_in(int p, int l, int r, int lo, int hi, int v, LL w)
    {
        if (lo <= l && r <= hi) { g.add(p, v, w); return; }
        int mid = (l + r) >> 1;
        if (lo <= mid) link_in(tlc[p], l, mid, lo, hi, v, w);
        if (hi > mid) link_in(trc[p], mid + 1, r, lo, hi, v, w);
    }
};
#endif
/*
 * Usage:
 * int n, q, s; cin >> n >> q >> s;         // 例题: CF786B Legacy
 * SegGraph sg{n, 8 * n + 34 * q};          // 边预算: 4n 树边 + 每操作 2*ceil(log2 n)
 * sg.build(n);
 * while (q--)
 * {
 *     int op; cin >> op;
 *     if (op == 1) { int u, v; LL w; cin >> u >> v >> w; sg.add_edge(u, v, w); }
 *     if (op == 2) { int u, l, r; LL w; cin >> u >> l >> r >> w; sg.add_out(u, l, r, w); }
 *     if (op == 3) { int v, l, r; LL w; cin >> v >> l >> r >> w; sg.add_in(l, r, v, w); }
 * }
 * Dijkstra dij{sg.tot};                    // 引擎: 最短路 dij.h
 * dij.init(sg.tot);
 * dij.run(s, sg.g);
 * for (int i = 1; i <= n; i++)             // 原点 i 的最短路, 不可达 = INF
 *     cout << dij.dist[i] << " \n"[i == n];
 * // 多测: sg.build(n) 重跑(内部自动清图)
 */
