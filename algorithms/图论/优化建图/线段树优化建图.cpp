// zoi: segGraph
#ifndef Z_OI_SEG_GRAPH
#define Z_OI_SEG_GRAPH

#include "../图的存储/Graph.cpp"
#include "../../杂项/utils/utils.cpp"

// 用两棵线段树压缩区间连边, 出树父连子、入树子连父, 骨架边权为 0, 两树共用原点叶子
// 原点 1 .. n, build 后 tot = 3 * n - 2, 另建中继点接在后面; g 为最终有向图
// 区间须满足 1 <= l <= r <= n, 单点参数可用原点或接口返回的中继点; build 后旧虚点编号失效
// Empty 用于可达性/SCC, 数值权用于最短路, 权值限制由下游算法决定; 所有加边时间按均摊计
// N/Q/M 为原点上限/新增中继上限/预留边数; 点表及儿子表约 72N + 16Q B, 每边 LL 为 16 B、Empty 为 8 B
// LL 下 N = Q = 2e5、M = 8e6 约 145.6 MB, 不含下游算法、分配器开销及边数组扩容余量
template <class W = LL>
struct SegGraph
{
    int n, tot;
    Graph<true, W> g;
    // 分配原点和中继点容量, max_extra = -1 时取 max_n; max_m 仅为边预留提示, 不限制加边次数
    // 时间 O(max_n + max_extra) | 空间 O(max_n + max_extra + max_m), max_extra 按实际采用值计
    SegGraph(int max_n = 0, int max_m = 0, int max_extra = -1) : n(0), tot(0),
        g(3 * max_n + (max_extra < 0 ? max_n : max_extra) + 10, max_m),
        point_cap(max_n), extra_cap(max_extra < 0 ? max_n : max_extra),
        tlc(3 * max_n + 10, 0), trc(3 * max_n + 10, 0)
    {}
    // 清掉上轮图并为 1 .. _n 建骨架, 要求 1 <= _n <= max_n, 新增中继预算重新可用
    // 时间 O(_n + 上轮清图开销) | 额外空间 O(log _n), 骨架为 3 * _n - 2 点、4 * _n - 4 边
    void build(int _n)
    {
        assert(_n >= 1 && _n <= point_cap);
        n = _n;
        tot = n;
        g.clear();
        rt_out = build_out(1, n);
        rt_in = build_in(1, n);
    }
    // 添加 u 到 v 的单向边, 权值为 w
    // 时间 O(1) | 新增 1 条边
    void add_p2p(int u, int v, W w = W()) { g.add(u, v, w); }
    // 让 u 向 [l, r] 内每个原点连权为 w 的边
    // 时间 O(log n) | 新增 O(log n) 条边, 递归空间 O(log n)
    void add_p2r(int u, int l, int r, W w = W()) { link_out(rt_out, 1, n, l, r, u, w); }
    // 让 [l, r] 内每个原点向 v 连权为 w 的边
    // 时间 O(log n) | 新增 O(log n) 条边, 递归空间 O(log n)
    void add_r2p(int l, int r, int v, W w = W()) { link_in(rt_in, 1, n, l, r, v, w); }
    // 新建中继点并从 u 连权为 w 的边进入, 返回中继点编号
    // 时间 O(1) | 新增 1 个中继点、1 条边
    int add_p2new(int u, W w = W())
    {
        int p = new_point();
        g.add(u, p, w);
        return p;
    }
    // 新建中继点并从 [l, r] 每个原点连权为 w 的边进入, 返回中继点编号
    // 时间 O(log n) | 新增 1 个中继点、O(log n) 条边, 递归空间 O(log n)
    int add_r2new(int l, int r, W w = W())
    {
        int p = new_point();
        link_in(rt_in, 1, n, l, r, p, w);
        return p;
    }
    // 让 [l1, r1] 每个原点向 [l2, r2] 每个原点连权为 w 的边, 两区间允许重叠
    // 时间 O(log n) | 新增 1 个中继点、O(log n) 条边, 递归空间 O(log n)
    void add_r2r(int l1, int r1, int l2, int r2, W w = W())
    {
        int mid_node = add_r2new(l1, r1);
        add_p2r(mid_node, l2, r2, w);
    }
private:
    int rt_out, rt_in;
    int point_cap, extra_cap;
    VI tlc, trc;
    int new_point()
    {
        assert(tot - (3 * n - 2) < extra_cap);
        return ++tot;
    }
    int build_out(int l, int r)
    {
        if (l == r) return l;          // 叶子 = 原点 id
        int p = ++tot, mid = (l + r) >> 1;
        tlc[p] = build_out(l, mid);
        trc[p] = build_out(mid + 1, r);
        g.add(p, tlc[p]);
        g.add(p, trc[p]);
        return p;
    }
    int build_in(int l, int r)
    {
        if (l == r) return l;
        int p = ++tot, mid = (l + r) >> 1;
        tlc[p] = build_in(l, mid);
        trc[p] = build_in(mid + 1, r);
        g.add(tlc[p], p);
        g.add(trc[p], p);
        return p;
    }
    void link_out(int p, int l, int r, int lo, int hi, int u, W w)
    {
        if (lo <= l && r <= hi) { g.add(u, p, w); return; }
        int mid = (l + r) >> 1;
        if (lo <= mid) link_out(tlc[p], l, mid, lo, hi, u, w);
        if (hi > mid) link_out(trc[p], mid + 1, r, lo, hi, u, w);
    }
    void link_in(int p, int l, int r, int lo, int hi, int v, W w)
    {
        if (lo <= l && r <= hi) { g.add(p, v, w); return; }
        int mid = (l + r) >> 1;
        if (lo <= mid) link_in(tlc[p], l, mid, lo, hi, v, w);
        if (hi > mid) link_in(trc[p], mid + 1, r, lo, hi, v, w);
    }
};
#endif
/* Usage
// 题目文件先 include segGraph.h 和 dij.h

int main()
{
    SegGraph<LL> sg(5, 64, 3);
    sg.build(5);
    sg.add_p2p(1, 2, 3);
    sg.add_p2r(2, 3, 4, 5);
    sg.add_r2p(4, 5, 1, 2);
    sg.add_r2r(1, 2, 5, 5, 7);
    int v = sg.add_r2new(2, 3, 1);
    int t = sg.add_p2new(v, 2);
    sg.add_p2r(t, 4, 5, 1);          // 中继点可继续连边

    Dijkstra dij(sg.tot);
    dij.init(sg.tot);
    dij.run(1, sg.g);                // 非负权, 只取 1 .. n 的原点答案
    for (int i = 1; i <= sg.n; i++)
        cout << dij.dist[i] << " \n"[i == sg.n];
    sg.build(2);                    // 清图并重置中继点编号
}
*/
