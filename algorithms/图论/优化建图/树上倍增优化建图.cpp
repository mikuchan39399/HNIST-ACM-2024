// zoi: treeGraph
#ifndef Z_OI_TREE_GRAPH
#define Z_OI_TREE_GRAPH

#include "建图上下文.cpp"

// 树路径优化建图; In 汇集 / Out 分发, id[u] 为树点 u 对应的已有图点
// 每方向至多 w*floor(log2 w) 个辅助点、两倍零权边; 原树边不自动加入图
// LCA 须有 dep/fa[k][u]/lca: 根深 1, 空祖先 0, 断连 -1; 建表后森林保持不变
// 覆盖入口须同属未清空的上下文, 不保留路径计数/网络流语义
template <class W = LL, bool In = true, bool Out = true>
struct TreeLinks
{
    static_assert(In || Out);
    GraphBuilder<W>& b;
    int n = 0;
    // 绑定已 init 的 builder; 使用期间不销毁/移动 builder. O(1)
    TreeLinks(GraphBuilder<W>& builder) : b(builder) {}
    // 按 1-based id 追加骨架, LCA 须已建表. 时空/新增点边 O(w log(w+1))
    template <class LCA>
    void build(LCA& lca, const VI& id) { build_maps(lca, id, id); }
    // 按等长 1-based 源/目标映射追加双向骨架. 时空/新增点边 O(w log(w+1))
    template <class LCA>
    void build(LCA& lca, const VI& src, const VI& dst) requires (In && Out) { build_maps(lca, src, dst); }
    // u-v 汇集出口, 返回 4 槽, 未用/断连为 0. 时间 O(T+log w), T=LCA 查询时间
    template <class LCA>
    array<int, 4> in_cover(int u, int v, LCA& lca) const requires In { return cover(u, v, lca, in); }
    // u-v 分发入口, 返回 4 槽, 未用/断连为 0. 时间 O(T+log w)
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

            for (int i = k - 1; i >= 0; i--)
                if (step & (1 << i)) x = lca.fa[i][x];
            res[cnt++] = table[k][x];
        };
        half(u);
        if (v != l) half(v);
        return res;
    }
};

// 独占整图的封装, 不拷贝/移动; 原点 1..n, 下游用 g/tot; 原树与 LCA 保持不变
// 路径参数为树点, 点参数可用中继; 路径操作均摊时间 O(T+log n), 额外空间 O(1)
template <class W = LL>
struct TreeGraph
{
    GraphBuilder<W> b;
    Graph<true, W>& g;
    int n = 0;
    int& tot;
    TreeLinks<W> paths;
    // N=max_n 为原点上限, Q=max_extra 为中继上限(-1 取 N); 固定容量, 骨架自动计入
    // max_m 为边预留(可扩容); 构造后 build. 时间 O(N log N+Q), 空间另加 O(max_m)
    TreeGraph(int max_n = 0, int max_m = 0, int max_extra = -1) :
        b(max_n * (2 * levels(max_n) - 1) + (max_extra < 0 ? max_n : max_extra), max_m),
        g(b.g), tot(b.tot), paths(b),
        point_cap(max_n), extra_cap(max_extra < 0 ? max_n : max_extra), base(0)
    {}
    TreeGraph(const TreeGraph&) = delete;
    TreeGraph& operator=(const TreeGraph&) = delete;
    // 按已建表 LCA 清图重建, 1<=n<=max_n, 中继预算复位
    // 时间 O(n log(n+1)+旧图大小), 索引/新增点边 O(n log(n+1))
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
    // u -> v, 权 w. 均摊 O(1), 新增 1 边
    void add_p2p(int u, int v, W w = W()) { g.add(u, v, w); }
    // u -> 路径 a-b, 权 w; 断连返回 false 且不改图. 至多 4 边
    template <class LCA>
    bool add_p2path(int u, int a, int b, LCA& lca, W w = W())
    {
        auto ns = paths.out_cover(a, b, lca);
        for (int v : ns) if (v) g.add(u, v, w);
        return ns[0] != 0;
    }
    // 路径 a-b -> v, 权 w; 断连返回 false 且不改图. 至多 4 边
    template <class LCA>
    bool add_path2p(int a, int b, int v, LCA& lca, W w = W())
    {
        auto ns = paths.in_cover(a, b, lca);
        for (int u : ns) if (u) g.add(u, v, w);
        return ns[0] != 0;
    }
    // 新建中继并连 u -> 中继, 权 w; 返回编号. 均摊 O(1), 1 点 1 边
    int add_p2new(int u, W w = W())
    {
        int p = new_point();
        add_p2p(u, p, w);
        return p;
    }
    // 新建中继并连路径 a-b -> 中继, 权 w; 返回编号, 断连 -1 且不耗点. 1 点至多 4 边
    template <class LCA>
    int add_path2new(int a, int b, LCA& lca, W w = W())
    {
        auto ns = paths.in_cover(a, b, lca);
        if (!ns[0]) return -1;
        int p = new_point();
        for (int u : ns) if (u) g.add(u, p, w);
        return p;
    }
    // 路径 a-b -> c-d 全连接, 权 w; 返回中继编号, 断连 -1 且不改图. 1 点至多 8 边
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
