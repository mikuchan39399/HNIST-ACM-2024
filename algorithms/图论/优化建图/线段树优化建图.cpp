// zoi: segGraph
#ifndef Z_OI_SEG_GRAPH
#define Z_OI_SEG_GRAPH

#include "建图上下文.cpp"
#include "../../杂项/utils/utils.cpp"

// 映射序列的区间覆盖, In 汇集/Out 分发可独立开启, 同一结构也可传不同的源/目标映射
// build 只追加共享图, 覆盖入口按全局编号返回, 索引不持有输入数组; 重建索引不删除旧图边
// 每方向 w-1 个辅助点、2w-2 条零权边, 编号表约 8w B; 区间 l>r 表示空
// 结构与覆盖必须属于同一未清空的上下文, 不保留路径条数或网络流容量语义
template <class W = LL, bool In = true, bool Out = true>
struct SegLinks
{
    static_assert(In || Out);
    GraphBuilder<W>& b;
    int n = 0;
    // 绑定同权值类型的 builder, 不清图; builder 须先 init, 后续 build(id) 追加骨架
    // w 为序列长, 每启用一个方向需在 builder 另留 max(w - 1, 0) 个点, 结构使用期间 builder 不销毁或移动
    // 时间 O(1) | 空间 O(1)
    SegLinks(GraphBuilder<W>& builder) : b(builder) {}
    // 按 1-based id 追加区间骨架, 点编号均须已存在, 空序列传 VI{0}
    // 时间 O(w) | 索引 O(w), 每启用方向新增 max(w-1,0) 点、2max(w-1,0) 边
    void build(const VI& id) { build_maps(id, id); }
    // 按不同的 1-based 源/目标映射追加双向骨架, 两数组大小相同
    // 时间 O(w) | 索引 O(w), 新增 2max(w-1,0) 点、4max(w-1,0) 边
    void build(const VI& src, const VI& dst) requires (In && Out) { build_maps(src, dst); }
    // 返回 [l,r] 的汇集出口, 下标为局部位置, 空段返回空容器
    // 时间 O(log(w+1)) | 返回 O(log(w+1)) 个全局点编号
    VI in_cover(int l, int r) const requires In { return cover(l, r, in); }
    // 返回 [l,r] 的分发入口, 下标为局部位置, 空段返回空容器
    // 时间 O(log(w+1)) | 返回 O(log(w+1)) 个全局点编号
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

// 单结构兼容封装, 内核复用 SegLinks, 原点 1..n, build 清图并重置中继预算
// g/tot 提供给下游; N = max_n, Q = 实际中继预算
// 骨架 3n-2 点/4n-4 边, 索引约 16n B, 图点表约 48N+16Q B, 边 Empty/int/LL 为 8/12/16 B
// 区间满足 1<=l<=r<=n, 点参数可用原点或返回中继, W() 为零权; 对象不拷贝或移动
template <class W = LL>
struct SegGraph
{
    GraphBuilder<W> b;
    Graph<true, W>& g;
    int n = 0;
    int& tot;
    SegLinks<W> ranges;
    // max_n 仅为原点数上限, 骨架自动计入; max_m 预留全图边数(含骨架边), 默认 0, 不足自动扩容
    // max_extra 仅为中继上限, add_r2r/add_r2new/add_p2new 各耗 1 个; 默认 -1 取 max_n, 不用中继传 0
    // 两项点数上限取够用的上界即可, 固定不扩容; 构造后 build(n), 1 <= n <= max_n
    // 时间 O(N + Q) | 空间 O(N + Q + max_m), N/Q 见类头
    SegGraph(int max_n = 0, int max_m = 0, int max_extra = -1) :
        b(3 * max_n + (max_extra < 0 ? max_n : max_extra) + 10, max_m),
        g(b.g), tot(b.tot), ranges(b),
        point_cap(max_n), extra_cap(max_extra < 0 ? max_n : max_extra)
    {}
    SegGraph(const SegGraph&) = delete;
    SegGraph& operator=(const SegGraph&) = delete;
    // 清掉上轮图并为 1 .. _n 建骨架, 要求 1 <= _n <= max_n, 新增中继预算重新可用
    // 时间 O(_n + 上轮清图开销) | 额外空间 O(_n), 骨架为 3 * _n - 2 点、4 * _n - 4 边
    void build(int _n)
    {
        assert(_n >= 1 && _n <= point_cap);
        n = _n;
        b.init(n);
        VI id(n + 1);
        iota(id.begin(), id.end(), 0);
        ranges.build(id);
    }
    // 添加 u 到 v 的单向边, 权值为 w
    // 时间 O(1) | 新增 1 条边
    void add_p2p(int u, int v, W w = W()) { g.add(u, v, w); }
    // 让 u 向 [l, r] 内每个原点连权为 w 的边
    // 时间 O(log n) | 新增 O(log n) 条边, 递归空间 O(log n)
    void add_p2r(int u, int l, int r, W w = W()) { for (int v : ranges.out_cover(l, r)) b.add(u, v, w); }
    // 让 [l, r] 内每个原点向 v 连权为 w 的边
    // 时间 O(log n) | 新增 O(log n) 条边, 递归空间 O(log n)
    void add_r2p(int l, int r, int v, W w = W()) { for (int u : ranges.in_cover(l, r)) b.add(u, v, w); }
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
        for (int u : ranges.in_cover(l, r)) b.add(u, p, w);
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
