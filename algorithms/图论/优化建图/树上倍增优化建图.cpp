// zoi: treeGraph
#ifndef Z_OI_TREE_GRAPH
#define Z_OI_TREE_GRAPH

#include "../图的存储/Graph.cpp"

// 倍增段含向上连续 2^k 个点, 入段子连父、出段父连子, 0 层共用原点, 骨架边权为 0
// 路径按 LCA 分两半, 每半用首尾两个可重叠段覆盖, 一条路径至多 4 段, 不用于统计路径条数
// 原点 1 .. n, 中继点由接口返回, g 为最终有向图; 原树只定义路径, 可通行的树边须自行加入 g
// 外置 LCA 须提供 dep/fa[k][u]/lca(u,v), 根深度为 1、空祖先为 0、断连返回 -1
// build 和后续连边使用同一份未改变的森林建表, 路径端点只用原点, 两条路径可分属不同树
// N/Q/M 为原点上限/中继上限/预留边数, L = floor(log2 N) + 1, 点容量 V = N(2L - 1) + Q
// 点表与倍增编号表约 16V + 8NL B, 每边 int/LL/Empty 为 12/16/8 B; 外置 LCA 与下游另计
// N = 5e4, Q = 1e6, M = 11e6 时 int 约 179.2 MB, 不含分配器及边扩容余量
// 权值限制由下游决定, 默认 W() 为零权; 加边时间按均摊计, T 为一次外置 LCA 查询时间
template <class W = LL>
struct TreeGraph
{
    int n = 0, tot = 0;
    Graph<true, W> g;
    // 分配原点及中继容量, max_extra = -1 时取 max_n; max_m 只预留边, 边数组仍可扩容
    // 时间 O(N log N + Q) | 空间 O(N log N + Q + M)
    TreeGraph(int max_n = 0, int max_m = 0, int max_extra = -1) :
        g(max_n * (2 * levels(max_n) - 1) + (max_extra < 0 ? max_n : max_extra), max_m),
        point_cap(max_n), extra_cap(max_extra < 0 ? max_n : max_extra), base(0),
        in(levels(max_n), VI(max_n + 1)), out(levels(max_n), VI(max_n + 1))
    {}
    // 用已建表的 1 .. _n 清图重建骨架, 1 <= _n <= max_n, 中继预算复位且旧虚点编号失效
    // 时间 O(_n log _n + 上轮清图开销) | 额外空间 O(1), K = floor(log2 _n) 时至多 _n * (2K + 1) 点、4 * _n * K 边
    template <class LCA>
    void build(LCA& lca, int _n)
    {
        assert(_n >= 1 && _n <= point_cap);
        n = _n;
        tot = n;
        g.clear();
        for (int u = 1; u <= n; u++) in[0][u] = out[0][u] = u;
        for (int k = 1; k < levels(n); k++)
        {
            for (int u = 1; u <= n; u++)
            {
                in[k][u] = out[k][u] = 0;
                if (lca.dep[u] < (1 << k)) continue;
                int v = lca.fa[k - 1][u];
                in[k][u] = ++tot;
                out[k][u] = ++tot;
                g.add(in[k - 1][u], in[k][u]);
                g.add(in[k - 1][v], in[k][u]);
                g.add(out[k][u], out[k - 1][u]);
                g.add(out[k][u], out[k - 1][v]);
            }
        }
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
        auto ns = cover(a, b, lca, out);
        for (int v : ns) if (v) g.add(u, v, w);
        return ns[0] != 0;
    }
    // 从路径 a-b 每个原点向 v 连权为 w 的边, a-b 断连时返回 false 且不改图
    // 时间 O(T + log n) | 新增至多 4 条边, 额外空间 O(1)
    template <class LCA>
    bool add_path2p(int a, int b, int v, LCA& lca, W w = W())
    {
        auto ns = cover(a, b, lca, in);
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
        auto ns = cover(a, b, lca, in);
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
        auto src = cover(a, b, lca, in), dst = cover(c, d, lca, out);
        if (!src[0] || !dst[0]) return -1;
        int p = new_point();
        for (int u : src) if (u) g.add(u, p);
        for (int v : dst) if (v) g.add(p, v, w);
        return p;
    }
private:
    int point_cap, extra_cap, base;
    VVI in, out;
    static int levels(int x) { return x <= 1 ? 1 : __lg(x) + 1; }
    int new_point()
    {
        assert(tot - base < extra_cap);
        return ++tot;
    }
    template <class LCA>
    array<int, 4> cover(int u, int v, LCA& lca, VVI& table)
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
#endif

/* Usage
// 题目先 include treeGraph.h、lca.h、dij.h
int main()
{
    Graph<false, int> tree(5, 3);
    tree.add(1, 2, 3);
    tree.add(2, 3, 4);
    tree.add(2, 4, 5);              // 5 是孤立点
    LCA lca(5);
    lca.build(tree);
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
    dij.init(tg.tot);
    dij.run(5, tg.g);
    for (int u = 1; u <= 5; u++)
        cout << (dij.dist[u] == INF ? -1 : dij.dist[u]) << " \n"[u == 5];
    tree.clear();
    lca.init(1);
    lca.build(tree);
    tg.build(lca, 1);               // 原点和中继预算复用, 旧编号失效
}
*/
