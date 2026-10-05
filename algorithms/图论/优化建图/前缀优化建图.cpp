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
// 前缀汇集: id[1..r] -> a[r], a[0]=0. 时空 O(w), 新增 max(w-1,0) 点、两倍边
template <class W>
VI prefix_in(GraphBuilder<W>& b, const VI& id) { return z_graph_detail::chain<true, false>(b, id); }
// 前缀分发: a[r] -> id[1..r], a[0]=0. 时空 O(w), 新增 max(w-1,0) 点、两倍边
template <class W>
VI prefix_out(GraphBuilder<W>& b, const VI& id) { return z_graph_detail::chain<false, false>(b, id); }
// 后缀汇集: id[l..w] -> a[l], a[w+1]=0. 时空 O(w), 新增 max(w-1,0) 点、两倍边
template <class W>
VI suffix_in(GraphBuilder<W>& b, const VI& id) { return z_graph_detail::chain<true, true>(b, id); }
// 后缀分发: a[l] -> id[l..w], a[w+1]=0. 时空 O(w), 新增 max(w-1,0) 点、两倍边
template <class W>
VI suffix_out(GraphBuilder<W>& b, const VI& id) { return z_graph_detail::chain<false, true>(b, id); }

// 上述函数: id 为 1-based 已有图点(可重复), 空序列 VI{0}; 只追加, 链深 O(w)
// 下面封装独占整图, 不拷贝/移动; 单向骨架 3n-2 点/4n-4 边, 双向 5n-4 点/8n-8 边
// 前缀 r=0、后缀 l=n+1 为空段; add 均摊 O(1), 至多 1 边, *2new 另耗 1 中继
template <class W = LL, bool WithPrefix = true, bool WithSuffix = false>
struct PrefixGraph
{
    static_assert(WithPrefix || WithSuffix);
    GraphBuilder<W> b;
    Graph<true, W>& g;
    int n = 0;
    int& tot;
    // N=max_n 为原点上限, Q=max_extra 为中继上限(-1 取 N); 两者固定, 骨架自动计入
    // max_m 为边预留(可扩容); 构造后 build. 时间 O(N+Q), 空间 O(N+Q+max_m)
    PrefixGraph(int max_n = 0, int max_m = 0, int max_extra = -1) :
        b((1 + 2 * (WithPrefix + WithSuffix)) * max_n + (max_extra < 0 ? max_n : max_extra), max_m),
        g(b.g), tot(b.tot),
        point_cap(max_n), extra_cap(max_extra < 0 ? max_n : max_extra)
    {}
    PrefixGraph(const PrefixGraph&) = delete;
    PrefixGraph& operator=(const PrefixGraph&) = delete;
    // 清图并重建 1..n, 1<=n<=max_n, 中继预算复位. 时间 O(n+旧图大小), 额外空间 O(n)
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
    // u -> v, 权 w
    void add_p2p(int u, int v, W w = W()) { g.add(u, v, w); }
    // u -> [1,r], 权 w
    void add_p2pre(int u, int r, W w = W()) requires WithPrefix
    {
        if (r) g.add(u, out(r), w);
    }
    // [1,r] -> v, 权 w
    void add_pre2p(int r, int v, W w = W()) requires WithPrefix
    {
        if (r) g.add(in(r), v, w);
    }
    // [1,r1] -> [1,r2] 全连接, 权 w
    void add_pre2pre(int r1, int r2, W w = W()) requires WithPrefix
    {
        if (r1 && r2) g.add(in(r1), out(r2), w);
    }
    // u -> [l,n], 权 w
    void add_p2suf(int u, int l, W w = W()) requires WithSuffix
    {
        if (l <= n) g.add(u, out(n - l + 1, true), w);
    }
    // [l,n] -> v, 权 w
    void add_suf2p(int l, int v, W w = W()) requires WithSuffix
    {
        if (l <= n) g.add(in(n - l + 1, true), v, w);
    }
    // [l1,n] -> [l2,n] 全连接, 权 w
    void add_suf2suf(int l1, int l2, W w = W()) requires WithSuffix
    {
        if (l1 <= n && l2 <= n) g.add(in(n - l1 + 1, true), out(n - l2 + 1, true), w);
    }
    // [1,r] -> [l,n] 全连接, 权 w
    void add_pre2suf(int r, int l, W w = W()) requires (WithPrefix && WithSuffix)
    {
        if (r && l <= n) g.add(in(r), out(n - l + 1, true), w);
    }
    // [l,n] -> [1,r] 全连接, 权 w
    void add_suf2pre(int l, int r, W w = W()) requires (WithPrefix && WithSuffix)
    {
        if (l <= n && r) g.add(in(n - l + 1, true), out(r), w);
    }
    // 新建中继并连 u -> 中继, 权 w; 返回新编号
    int add_p2new(int u, W w = W())
    {
        int p = new_point();
        add_p2p(u, p, w);
        return p;
    }
    // 新建中继并连 [1,r] -> 中继, 权 w; 返回新编号, 空段仍耗点
    int add_pre2new(int r, W w = W()) requires WithPrefix
    {
        int p = new_point();
        add_pre2p(r, p, w);
        return p;
    }
    // 新建中继并连 [l,n] -> 中继, 权 w; 返回新编号, 空段仍耗点
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
// include prefixGraph.h 和 dij.h; 无权图用 Empty
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
