// zoi: segGraph
#ifndef Z_OI_SEG_GRAPH
#define Z_OI_SEG_GRAPH

#include "建图上下文.cpp"
#include "../../杂项/utils/utils.cpp"

// 区间优化建图; In 汇集 / Out 分发, 局部位置 1-based, l>r 为空段
// 每方向新增 max(w-1,0) 点、两倍零权边; 空间 O(w); 只追加, 重建不删旧图
// 覆盖入口须同属未清空的上下文, 不保留路径计数/网络流语义
template <class W = LL, bool In = true, bool Out = true>
struct SegLinks
{
    static_assert(In || Out);
    GraphBuilder<W>& b;
    int n = 0;
    // 绑定已 init 的 builder; 使用期间不销毁/移动 builder. O(1)
    SegLinks(GraphBuilder<W>& builder) : b(builder) {}
    // 按 1-based 已有点 id 追加骨架, 空序列 VI{0}. 时空 O(w)
    void build(const VI& id) { build_maps(id, id); }
    // 按等长 1-based 源/目标映射追加双向骨架. 时空 O(w)
    void build(const VI& src, const VI& dst) requires (In && Out) { build_maps(src, dst); }
    // [l,r] 汇集出口, 返回全局点号, 空段为空. 时空 O(log(w+1))
    VI in_cover(int l, int r) const requires In { return cover(l, r, in); }
    // [l,r] 分发入口, 返回全局点号, 空段为空. 时空 O(log(w+1))
    VI out_cover(int l, int r) const requires Out { return cover(l, r, out); }
private:
    VI in, out;
    void build_maps(const VI& src, const VI& dst)
    {
        assert(src.size() == dst.size() && !src.empty());
        n = (int)src.size() - 1;
        if constexpr (In)
        {
            in.assign(2 * n + 1, 0);
            if (n) build_node(1, 1, n, src, in, true);
        }
        if constexpr (Out)
        {
            out.assign(2 * n + 1, 0);
            if (n) build_node(1, 1, n, dst, out, false);
        }
    }
    void build_node(int p, int l, int r, const VI& id, VI& tr, bool inward)
    {
        if (l == r) { tr[p] = id[l]; return; }
        int mid = (l + r) >> 1, lc = p + 1, rc = p + 2 * (mid - l + 1);
        tr[p] = b.new_node();
        build_node(lc, l, mid, id, tr, inward);
        build_node(rc, mid + 1, r, id, tr, inward);
        if (inward) b.add(tr[lc], tr[p]), b.add(tr[rc], tr[p]);
        else b.add(tr[p], tr[lc]), b.add(tr[p], tr[rc]);
    }
    VI cover(int lo, int hi, const VI& tr) const
    {
        VI res;
        if (!n || lo > hi) return res;
        auto dfs = [&](auto&& self, int p, int l, int r) -> void
        {
            if (lo <= l && r <= hi) { res.push_back(tr[p]); return; }
            int mid = (l + r) >> 1;
            if (lo <= mid) self(self, p + 1, l, mid);
            if (hi > mid) self(self, p + 2 * (mid - l + 1), mid + 1, r);
        };
        dfs(dfs, 1, 1, n);
        return res;
    }
};

// 独占整图的封装, 不拷贝/移动; 下游用 g/tot, 原点 1..n, W() 为零权
// 骨架 3n-2 点/4n-4 边; 区间 1<=l<=r<=n, 点参数可用中继编号
template <class W = LL>
struct SegGraph
{
    GraphBuilder<W> b;
    Graph<true, W>& g;
    int n = 0;
    int& tot;
    SegLinks<W> ranges;
    // N=max_n 为原点上限, Q=max_extra 为中继上限(-1 取 N); 两者固定, 骨架自动计入
    // max_m 为边预留(可扩容); 构造后 build. 时间 O(N+Q), 空间 O(N+Q+max_m)
    SegGraph(int max_n = 0, int max_m = 0, int max_extra = -1) :
        b(3 * max_n + (max_extra < 0 ? max_n : max_extra) + 10, max_m),
        g(b.g), tot(b.tot), ranges(b),
        point_cap(max_n), extra_cap(max_extra < 0 ? max_n : max_extra)
    {}
    SegGraph(const SegGraph&) = delete;
    SegGraph& operator=(const SegGraph&) = delete;
    // 清图并重建 1..n, 1<=n<=max_n, 中继预算复位. 时间 O(n+旧图大小), 额外空间 O(n)
    void build(int _n)
    {
        assert(_n >= 1 && _n <= point_cap);
        n = _n;
        b.init(n);
        VI id(n + 1);
        iota(id.begin(), id.end(), 0);
        ranges.build(id);
    }
    // u -> v, 权 w. 均摊 O(1), 新增 1 边
    void add_p2p(int u, int v, W w = W()) { g.add(u, v, w); }
    // u -> [l,r], 权 w. 均摊时空/新增边 O(log n)
    void add_p2r(int u, int l, int r, W w = W()) { for (int v : ranges.out_cover(l, r)) b.add(u, v, w); }
    // [l,r] -> v, 权 w. 均摊时空/新增边 O(log n)
    void add_r2p(int l, int r, int v, W w = W()) { for (int u : ranges.in_cover(l, r)) b.add(u, v, w); }
    // 新建中继并连 u -> 中继, 权 w; 返回编号. 均摊 O(1), 新增 1 点 1 边
    int add_p2new(int u, W w = W())
    {
        int p = new_point();
        g.add(u, p, w);
        return p;
    }
    // 新建中继并连 [l,r] -> 中继, 权 w; 返回编号. 均摊时空 O(log n), 1 点 O(log n) 边
    int add_r2new(int l, int r, W w = W())
    {
        int p = new_point();
        for (int u : ranges.in_cover(l, r)) b.add(u, p, w);
        return p;
    }
    // [l1,r1] -> [l2,r2] 全连接, 权 w, 可重叠. 均摊时空 O(log n), 1 点 O(log n) 边
    void add_r2r(int l1, int r1, int l2, int r2, W w = W())
    {
        int mid_node = add_r2new(l1, r1);
        add_p2r(mid_node, l2, r2, w);
    }
private:
    int point_cap, extra_cap;
    int new_point()
    {
        assert(tot - (3 * n - 2) < extra_cap);
        return b.new_node();
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
    sg.add_p2r(t, 4, 5, 1); // 中继点可继续连边

    Dijkstra dij(sg.tot);

    dij.run(1, sg.g, sg.tot); // 非负权, 只取 1 .. n 的原点答案
    for (int i = 1; i <= sg.n; i++)
        cout << dij.dist[i] << " \n"[i == sg.n];
    sg.build(2); // 清图并重置中继点编号

    GraphBuilder<LL> b(24, 48);
    b.init(6);
    SegLinks<LL, false, true> dst(b);
    dst.build(VI{0, 6, 2, 4}); // 局部位置 1,2,3 映射到图点 6,2,4
    b.link(VI{1}, dst.out_cover(2, 3), 5); // 1 -> 图点 2,4
    SegLinks<LL, true, false> src(b);
    src.build(VI{0, 3, 5});
    b.link(src.in_cover(1, 2), dst.out_cover(1, 3), 7); // 跨两份结构

}
*/
