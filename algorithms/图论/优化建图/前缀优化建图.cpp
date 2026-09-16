// zoi: prefixGraph
#ifndef Z_OI_PREFIX_GRAPH
#define Z_OI_PREFIX_GRAPH

#include "建图上下文.cpp"

namespace z_graph_detail
{
template <bool In, bool Reverse, class W>
VI chain(GraphBuilder<W>& b, const VI& id)
{
    int n = (int)id.size() - 1;
    VI a(n + 2);
    for (int len = 1; len <= n; len++)
    {
        int i = Reverse ? n - len + 1 : len;
        if (len == 1) a[i] = id[i];
        else
        {
            a[i] = b.new_node();
            int last = a[i + (Reverse ? 1 : -1)];
            if constexpr (In) b.add(id[i], a[i]), b.add(last, a[i]);
            else b.add(a[i], id[i]), b.add(a[i], last);
        }
    }
    return a;
}
}
// 为 1-based id 建前缀汇集链, 返回 a[r] 可被 id[1..r] 到达, a[0] = 0
// 时间 O(w) | 索引 O(w), 新增 max(w-1,0) 点和 2max(w-1,0) 边
template <class W>
VI prefix_in(GraphBuilder<W>& b, const VI& id) { return z_graph_detail::chain<true, false>(b, id); }
// 为 1-based id 建前缀分发链, 返回 a[r] 可到达 id[1..r], a[0] = 0
// 时间 O(w) | 索引 O(w), 新增 max(w-1,0) 点和 2max(w-1,0) 边
template <class W>
VI prefix_out(GraphBuilder<W>& b, const VI& id) { return z_graph_detail::chain<false, false>(b, id); }
// 为 1-based id 建后缀汇集链, 返回 a[l] 可被 id[l..w] 到达, a[w+1] = 0
// 时间 O(w) | 索引 O(w), 新增 max(w-1,0) 点和 2max(w-1,0) 边
template <class W>
VI suffix_in(GraphBuilder<W>& b, const VI& id) { return z_graph_detail::chain<true, true>(b, id); }
// 为 1-based id 建后缀分发链, 返回 a[l] 可到达 id[l..w], a[w+1] = 0
// 时间 O(w) | 索引 O(w), 新增 max(w-1,0) 点和 2max(w-1,0) 边
template <class W>
VI suffix_out(GraphBuilder<W>& b, const VI& id) { return z_graph_detail::chain<false, true>(b, id); }

// 上述四个函数只追加骨架, id 必须为同一上下文的已有非零点, 可非连续或重复
// 不改变状态编号, 不自动取反, 不建立正反配对; 输入与返回索引可销毁, 图中边保留
// 空序列传 VI{0}, 所有入口为 0; 链深 O(w), 下游递归 SCC 需另行评估栈
// 以下为旧单结构接口的兼容封装, 共用上面的建链内核, build 仍清空自己的整图
// 单向骨架 3n-2 点/4n-4 边, 混合 5n-4 点/8n-8 边; 每方向索引约 8n B
// N = max_n, Q = 实际中继预算; 图点表单向/混合约 48N/80N + 16Q B
// 边 Empty/int/LL 为 8/12/16 B, 索引和下游另计; 前缀 r=0、后缀 l=n+1 表示空段, 对象不拷贝或移动
template <class W = LL, bool WithPrefix = true, bool WithSuffix = false>
struct PrefixGraph
{
    static_assert(WithPrefix || WithSuffix);
    GraphBuilder<W> b;
    Graph<true, W>& g;
    int n = 0;
    int& tot;
    // max_n 仅为原点数上限, 骨架自动计入; max_m 预留全图边数(含骨架边), 默认 0, 不足自动扩容
    // max_extra 仅为中继上限, 每次 add_*2new 耗 1 个; 默认 -1 取 max_n, 不用中继传 0
    // 两项点数上限取够用的上界即可, 固定不扩容; 构造后 build(n), 1 <= n <= max_n
    // 时间 O(N + Q) | 空间 O(N + Q + max_m), N/Q 见类头
    PrefixGraph(int max_n = 0, int max_m = 0, int max_extra = -1) :
        b((1 + 2 * (WithPrefix + WithSuffix)) * max_n + (max_extra < 0 ? max_n : max_extra), max_m),
        g(b.g), tot(b.tot),
        point_cap(max_n), extra_cap(max_extra < 0 ? max_n : max_extra)
    {}
    PrefixGraph(const PrefixGraph&) = delete;
    PrefixGraph& operator=(const PrefixGraph&) = delete;
    // 清图并为 1 .. _n 重建所选方向的双链, 1 <= _n <= max_n, 中继预算复位
    // 时间 O(_n + 上轮清图开销) | 额外空间 O(_n), 单向/混合的点边数见类头
    void build(int _n)
    {
        assert(_n >= 1 && _n <= point_cap);
        n = _n;
        b.init(n);
        VI id(n + 1);
        iota(id.begin(), id.end(), 0);
        if constexpr (WithPrefix) pi = prefix_in(b, id), po = prefix_out(b, id);
        if constexpr (WithSuffix) si = suffix_in(b, id), so = suffix_out(b, id);
        base = tot;
    }
    // 添加 u 到 v 权为 w 的单向边
    // 均摊时间 O(1) | 新增 1 条边
    void add_p2p(int u, int v, W w = W()) { g.add(u, v, w); }
    // 从 u 向 [1, r] 每个原点连权为 w 的边, r = 0 时不改图
    // 均摊时间 O(1) | 新增至多 1 条边
    void add_p2pre(int u, int r, W w = W()) requires WithPrefix
    {
        if (r) g.add(u, out(r), w);
    }
    // 从 [1, r] 每个原点向 v 连权为 w 的边, r = 0 时不改图
    // 均摊时间 O(1) | 新增至多 1 条边
    void add_pre2p(int r, int v, W w = W()) requires WithPrefix
    {
        if (r) g.add(in(r), v, w);
    }
    // 从 [1, r1] 向 [1, r2] 全连接权为 w 的边, 任一前缀为空时不改图
    // 均摊时间 O(1) | 新增至多 1 条边, 不新增中继点
    void add_pre2pre(int r1, int r2, W w = W()) requires WithPrefix
    {
        if (r1 && r2) g.add(in(r1), out(r2), w);
    }
    // 从 u 向 [l, n] 每个原点连权为 w 的边, l = n+1 时不改图
    // 均摊时间 O(1) | 新增至多 1 条边
    void add_p2suf(int u, int l, W w = W()) requires WithSuffix
    {
        if (l <= n) g.add(u, out(n - l + 1, true), w);
    }
    // 从 [l, n] 每个原点向 v 连权为 w 的边, l = n+1 时不改图
    // 均摊时间 O(1) | 新增至多 1 条边
    void add_suf2p(int l, int v, W w = W()) requires WithSuffix
    {
        if (l <= n) g.add(in(n - l + 1, true), v, w);
    }
    // 从 [l1, n] 向 [l2, n] 全连接权为 w 的边, 任一后缀为空时不改图
    // 均摊时间 O(1) | 新增至多 1 条边, 不新增中继点
    void add_suf2suf(int l1, int l2, W w = W()) requires WithSuffix
    {
        if (l1 <= n && l2 <= n) g.add(in(n - l1 + 1, true), out(n - l2 + 1, true), w);
    }
    // 从 [1, r] 向 [l, n] 全连接权为 w 的边, 任一段为空时不改图
    // 均摊时间 O(1) | 新增至多 1 条边, 不新增中继点
    void add_pre2suf(int r, int l, W w = W()) requires (WithPrefix && WithSuffix)
    {
        if (r && l <= n) g.add(in(r), out(n - l + 1, true), w);
    }
    // 从 [l, n] 向 [1, r] 全连接权为 w 的边, 任一段为空时不改图
    // 均摊时间 O(1) | 新增至多 1 条边, 不新增中继点
    void add_suf2pre(int l, int r, W w = W()) requires (WithPrefix && WithSuffix)
    {
        if (l <= n && r) g.add(in(n - l + 1, true), out(r), w);
    }
    // 新建中继点并从 u 连权为 w 的边进入, 返回中继编号
    // 均摊时间 O(1) | 新增 1 点、1 边
    int add_p2new(int u, W w = W())
    {
        int p = new_point();
        add_p2p(u, p, w);
        return p;
    }
    // 新建中继点并从 [1, r] 每个原点连权为 w 的边进入, r = 0 时仍返回新建的孤立点
    // 均摊时间 O(1) | 新增 1 点、至多 1 条边, 空前缀也消耗中继预算
    int add_pre2new(int r, W w = W()) requires WithPrefix
    {
        int p = new_point();
        add_pre2p(r, p, w);
        return p;
    }
    // 新建中继点并从 [l, n] 每个原点连权为 w 的边进入, l = n+1 时仍返回新建的孤立点
    // 均摊时间 O(1) | 新增 1 点、至多 1 条边, 空后缀也消耗中继预算
    int add_suf2new(int l, W w = W()) requires WithSuffix
    {
        int p = new_point();
        add_suf2p(l, p, w);
        return p;
    }
private:
    int point_cap, extra_cap, base = 0;
    VI pi, po, si, so;
    int in(int len, bool rev = false) const { return rev ? si[n - len + 1] : pi[len]; }
    int out(int len, bool rev = false) const { return rev ? so[n - len + 1] : po[len]; }
    int new_point()
    {
        assert(tot - base < extra_cap);
        return b.new_node();
    }
};
template <class W = LL>
using SuffixGraph = PrefixGraph<W, false, true>;
template <class W = LL>
using PrefixSuffixGraph = PrefixGraph<W, true, true>;
#endif

/* Usage
// 多组 2-SAT 优先使用 prefix_out/suffix_out, 下面先展示单结构兼容接口
// 题目文件先 include prefixGraph.h 和 dij.h; 无权蕴含图使用 PrefixGraph<Empty>
int main()
{
    PrefixGraph<LL> pg(5, 32, 2);
    pg.build(5);
    pg.add_p2pre(5, 3, 4);       // 5 -> 1, 2, 3, 每条逻辑边权为 4
    pg.add_pre2p(2, 4, 6);       // 1, 2 -> 4
    pg.add_pre2pre(2, 3, 2);     // 前缀间一条边, 不耗中继预算
    int p = pg.add_pre2new(3, 1);
    int q = pg.add_p2new(p, 2);
    pg.add_p2p(q, 4, 1);
    pg.add_p2pre(4, 0, 9);      // 空前缀不加边

    Dijkstra dij(pg.tot);
    dij.run(5, pg.g, pg.tot);    // 下游使用实际总点数, 不能只传 n
    for (int i = 1; i <= pg.n; i++)
        cout << dij.dist[i] << " \n"[i == pg.n]; // 4 4 4 8 0
    pg.build(1);               // 原点和中继预算复用, 旧 p/q 失效

    GraphBuilder<Empty> b(20, 32);
    b.init(12); // 6 个变量, 1..6 选, 7..12 不选
    VI a{0, 3, 4, 2}, neg{0, 9, 10, 8};
    auto pre = prefix_out(b, neg), suf = suffix_out(b, neg);
    for (int i = 1; i <= 3; i++)
    {
        b.add(a[i], pre[i - 1]);
        b.add(a[i], suf[i + 1]); // 0 入口自动跳过, 继续给 b 追加其他组
    }

    SuffixGraph<LL> sg(5, 32, 1);
    sg.build(5);
    sg.add_p2suf(1, 3, 4);     // 1 -> 3, 4, 5, 原点编号不反转
    sg.add_suf2p(4, 2, 2);
    sg.add_suf2suf(3, 5, 1);
    int t = sg.add_suf2new(6); // 空后缀, 仍新建孤立中继
    sg.add_p2p(2, t, 3);

    PrefixSuffixGraph<LL> both(5, 48, 0);
    both.build(5);
    both.add_pre2suf(2, 4, 7); // 1, 2 -> 4, 5
    both.add_suf2pre(5, 3, 2); // 5 -> 1, 2, 3
    dij = Dijkstra(both.tot);
    dij.run(1, both.g, both.tot);
    for (int i = 1; i <= both.n; i++)
        cout << dij.dist[i] << " \n"[i == both.n]; // 0 9 9 7 7
}
*/
