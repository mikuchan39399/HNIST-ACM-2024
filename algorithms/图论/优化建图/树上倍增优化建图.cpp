// zoi: treeGraph
#ifndef Z_OI_TREE_GRAPH
#define Z_OI_TREE_GRAPH

#include "建图上下文.cpp"

// 树上倍增覆盖与最终图分离, 树点 1..w 只用于 LCA, id[u] 决定连接哪个已有图点
// In 汇集/Out 分发独立开启, 单方向最多 w*floor(log2 w) 个辅助点及两倍骨架边
// 编号表每方向约 4wL B, L=floor(log2 w)+1; 原树边不自动加入最终图
// LCA 提供 dep/fa[k][u]/lca(u,v), 根深 1、空祖先 0、断连 -1; 建表后森林不变
// 每路径至多四个可重叠段, 不保留路径计数或网络流容量语义; 结构依赖未清空的上下文
template <class W = LL, bool In = true, bool Out = true>
struct TreeLinks
{
    static_assert(In || Out);
    GraphBuilder<W>& b;
    int n = 0;
    // 绑定同权值类型的 builder, 不清图; builder 须先 init, 后续 build(lca, id) 使用已建表的 LCA
    // w 为树点数, 每启用一个方向在 builder 另留至多 w * floor(log2(max(w,1))) 个点, builder 不销毁或移动
    // 时间 O(1) | 空间 O(1)
    TreeLinks(GraphBuilder<W>& builder) : b(builder) {}
    // 按 1-based id 追加森林倍增骨架, id[u] 是树点 u 的既有图点编号
    // 时间 O(w log(w+1)) | 索引与新增点边 O(w log(w+1))
    template <class LCA>
    void build(LCA& lca, const VI& id) { build_maps(lca, id, id); }
    // 按同样大小的 1-based 源/目标映射追加双向骨架, 两组状态可不同
    // 时间 O(w log(w+1)) | 索引与新增点边 O(w log(w+1))
    template <class LCA>
    void build(LCA& lca, const VI& src, const VI& dst) requires (In && Out) { build_maps(lca, src, dst); }
    // 返回树路径 u-v 的汇集出口, 未用槽为 0, 断连时全 0, 使用建图时的同一森林
    // 时间 O(T + log w) | 返回 4 个槽, T 为 lca 查询时间
    template <class LCA>
    array<int, 4> in_cover(int u, int v, LCA& lca) const requires In { return cover(u, v, lca, in); }
    // 返回树路径 u-v 的分发入口, 未用槽为 0, 断连时全 0
    // 时间 O(T + log w) | 返回 4 个槽, T 为 lca 查询时间
    template <class LCA>
    array<int, 4> out_cover(int u, int v, LCA& lca) const requires Out { return cover(u, v, lca, out); }
private:
    VVI in, out;
    static int levels(int x) { return x <= 1 ? 1 : __lg(x) + 1; }
    template <class LCA>
    void build_maps(LCA& lca, const VI& src, const VI& dst)
    {
        assert(src.size() == dst.size() && !src.empty());
        n = (int)src.size() - 1;
        if constexpr (In) in.assign(levels(n), VI(n + 1)), in[0] = src;
        if constexpr (Out) out.assign(levels(n), VI(n + 1)), out[0] = dst;
        for (int k = 1; k < levels(n); k++)
            for (int u = 1; u <= n; u++)
            {
                if (lca.dep[u] < (1 << k)) continue;
                int v = lca.fa[k - 1][u];
                if constexpr (In)
                {
                    in[k][u] = b.new_node();
                    b.add(in[k - 1][u], in[k][u]);
                    b.add(in[k - 1][v], in[k][u]);
                }
                if constexpr (Out)
                {
                    out[k][u] = b.new_node();
                    b.add(out[k][u], out[k - 1][u]);
                    b.add(out[k][u], out[k - 1][v]);
                }
            }
    }
    template <class LCA>
    array<int, 4> cover(int u, int v, LCA& lca, const VVI& table) const
    {
        array<int, 4> res{};
        int l = lca.lca(u, v), cnt = 0;
        if (l == -1) return res;
        auto half = [&](int x)
        {
            int len = lca.dep[x] - lca.dep[l] + 1;
            int k = __lg(len), step = len - (1 << k);
            res[cnt++] = table[k][x];
            if (!step) return;
            // 第二段止于 l, 与第一段重叠但不越出路径, 定位仍需倍增跳转
            for (int i = k - 1; i >= 0; i--)
                if (step & (1 << i)) x = lca.fa[i][x];
            res[cnt++] = table[k][x];
        };
        half(u);
        if (v != l) half(v);
        return res;
    }
};

// 单结构兼容封装, 复用 TreeLinks, 原点 1..n, build 仍清图与重置中继预算
// 原树和 LCA 外置且结构保持不变; N = max_n, Q = 实际中继预算, L = floor(log2(max(N,1))) + 1
// 点容量 V=N(2L-1)+Q, 图点表约 16V B, 双向索引约 8nL B, 边 Empty/int/LL 为 8/12/16 B
// 点参数可用返回中继, 路径参数使用原树点; 所有权值限制由下游决定, 对象不拷贝或移动
template <class W = LL>
struct TreeGraph
{
    GraphBuilder<W> b;
    Graph<true, W>& g;
    int n = 0;
    int& tot;
    TreeLinks<W> paths;
    // max_n 仅为原树点数上限, 倍增骨架自动计入; max_m 预留全图边数(含骨架边), 默认 0, 不足自动扩容
    // max_extra 仅为中继上限, add_path2path/add_path2new/add_p2new 成功各耗 1 个; 默认 -1 取 max_n, 不用中继传 0
    // 两项点数上限取够用的上界即可, 固定不扩容; 构造后 build(lca, n), LCA 已建表且 1 <= n <= max_n
    // 时间 O(NL + Q) | 空间 O(NL + Q + max_m), N/Q/L 见类头
    TreeGraph(int max_n = 0, int max_m = 0, int max_extra = -1) :
        b(max_n * (2 * levels(max_n) - 1) + (max_extra < 0 ? max_n : max_extra), max_m),
        g(b.g), tot(b.tot), paths(b),
        point_cap(max_n), extra_cap(max_extra < 0 ? max_n : max_extra), base(0)
    {}
    TreeGraph(const TreeGraph&) = delete;
    TreeGraph& operator=(const TreeGraph&) = delete;
    // 用已建表的 1 .. _n 清图重建骨架, 1 <= _n <= max_n, 中继预算复位且旧虚点编号失效
    // 时间 O(_n log(_n+1) + 上轮清图开销) | 索引 O(_n log(_n+1)), 临时映射 O(_n), 骨架点边见类头
    template <class LCA>
    void build(LCA& lca, int _n)
    {
        assert(_n >= 1 && _n <= point_cap);
        n = _n;
        b.init(n);
        VI id(n + 1);
        iota(id.begin(), id.end(), 0);
        paths.build(lca, id);
        base = tot;
    }
    // 添加 u 到 v 权为 w 的单向边, 点参数可用原点或返回的中继点
    // 时间 O(1) | 新增 1 条边
    void add_p2p(int u, int v, W w = W()) { g.add(u, v, w); }
    // 从 u 向路径 a-b 每个原点连权为 w 的边, a-b 断连时返回 false 且不改图
    // 时间 O(T + log n) | 新增至多 4 条边, 额外空间 O(1)
    template <class LCA>
    bool add_p2path(int u, int a, int b, LCA& lca, W w = W())
    {
        auto ns = paths.out_cover(a, b, lca);
        for (int v : ns) if (v) g.add(u, v, w);
        return ns[0] != 0;
    }
    // 从路径 a-b 每个原点向 v 连权为 w 的边, a-b 断连时返回 false 且不改图
    // 时间 O(T + log n) | 新增至多 4 条边, 额外空间 O(1)
    template <class LCA>
    bool add_path2p(int a, int b, int v, LCA& lca, W w = W())
    {
        auto ns = paths.in_cover(a, b, lca);
        for (int u : ns) if (u) g.add(u, v, w);
        return ns[0] != 0;
    }
    // 新建中继点并从 u 连权为 w 的边进入, 返回中继编号
    // 时间 O(1) | 新增 1 点、1 边
    int add_p2new(int u, W w = W())
    {
        int p = new_point();
        add_p2p(u, p, w);
        return p;
    }
    // 新建中继点并从路径 a-b 每个原点连权为 w 的边进入, 断连返回 -1 且不耗预算
    // 时间 O(T + log n) | 新增 1 点、至多 4 边, 额外空间 O(1)
    template <class LCA>
    int add_path2new(int a, int b, LCA& lca, W w = W())
    {
        auto ns = paths.in_cover(a, b, lca);
        if (!ns[0]) return -1;
        int p = new_point();
        for (int u : ns) if (u) g.add(u, p, w);
        return p;
    }
    // 从路径 a-b 向路径 c-d 全连接并返回中继编号, 任一路径断连则返回 -1 且不改图
    // 时间 O(T + log n) | 新增 1 点、至多 8 边, 入中继为零权、出中继为 w, 额外空间 O(1)
    template <class LCA>
    int add_path2path(int a, int b, int c, int d, LCA& lca, W w = W())
    {
        auto src = paths.in_cover(a, b, lca), dst = paths.out_cover(c, d, lca);
        if (!src[0] || !dst[0]) return -1;
        int p = new_point();
        for (int u : src) if (u) g.add(u, p);
        for (int v : dst) if (v) g.add(p, v, w);
        return p;
    }
private:
    int point_cap, extra_cap, base;
    static int levels(int x) { return x <= 1 ? 1 : __lg(x) + 1; }
    int new_point()
    {
        assert(tot - base < extra_cap);
        return b.new_node();
    }
};
#endif

/* Usage
// 题目先 include treeGraph.h、lca.h、dij.h
int main()
{
    Graph<false, int> tree(5, 3);
    tree.add(1, 2, 3);
    tree.add(2, 3, 4);
    tree.add(2, 4, 5); // 5 是孤立点
    LCA lca(5);
    lca.build(tree, 5);
    TreeGraph<int> tg(5, 100, 3);
    tg.build(lca, 5);
    for (int u = 1; u <= 5; u++)
        for (auto& e : tree[u]) tg.add_p2p(u, e.v, e.w);
    tg.add_p2path(5, 1, 3, lca, 7);
    tg.add_path2p(3, 4, 5, lca, 2);
    tg.add_path2path(1, 3, 4, 4, lca, 1);
    int p = tg.add_path2new(1, 2, lca, 3);
    int q = tg.add_p2new(p, 2);
    tg.add_p2path(q, 3, 4, lca, 1);
    cout << tg.add_path2new(1, 5, lca) << endl; // -1, 不耗中继预算
    Dijkstra dij(tg.tot);

    dij.run(5, tg.g, tg.tot);
    for (int u = 1; u <= 5; u++)
        cout << (dij.dist[u] == INF ? -1 : dij.dist[u]) << " \n"[u == 5];
    GraphBuilder<int> b(40, 100);
    b.init(10);
    TreeLinks<int, false, true> dst(b);
    dst.build(lca, VI{0, 6, 7, 8, 9, 10}); // 原树点 u 映射为图点 u+5
    b.link(VI{1}, dst.out_cover(2, 4, lca), 3); // 图点 1 -> 状态点 7,9
    tree.clear();

    lca.build(tree, 1);
    tg.build(lca, 1); // 原点和中继预算复用, 旧编号失效
}
*/
