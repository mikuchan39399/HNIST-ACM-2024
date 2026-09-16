// zoi: graphBuilder
#ifndef Z_OI_GRAPH_BUILDER
#define Z_OI_GRAPH_BUILDER

#include "../图的存储/Graph.cpp"

// 共享最终图和编号, 1 .. n 为初始状态点, 后续骨架及中继都从 new_node 分配
// 上下文须比所有结构活得久, 结构存在时不移动上下文; init 后所有旧结构和入口失效
// 点表约 16V B, 每边 Empty/int/LL 为 8/12/16 B, 下游与结构索引另计; W() 为零权
template <class W = Empty>
struct GraphBuilder
{
    Graph<true, W> g;
    int n = 0, tot = 0;
    // max_n 为整图点数上限 = 初始状态点 + 所有骨架点 + 中继点, 取够用的上界, 固定不扩容
    // max_m 预留全图边数(含骨架边), 可省略为 0, 不足自动扩容; 构造后 init(n), 默认零点容量不可申请点
    // 时间 O(max_n) | 空间 O(max_n + max_m)
    GraphBuilder(int max_n = 0, int max_m = 0) : g(max_n, max_m), cap(max_n) {}
    // 清空整张图并保留 1 .. _n 状态点, 0 <= _n <= max_n
    // 时间 O(上轮触碰点数 + 上轮边数) | 额外空间 O(1)
    void init(int _n)
    {
        assert(0 <= _n && _n <= cap);
        g.clear();
        n = tot = _n;
    }
    // 申请一个孤立图点并返回全局编号, 消耗一个点容量
    // 时间 O(1) | 额外空间 O(1)
    int new_node()
    {
        assert(tot < cap);
        return ++tot;
    }
    // 添加 u 到 v 的边, 0 表示空入口, 任一端为 0 时不加边
    // 均摊时间 O(1) | 新增至多 1 条边
    void add(int u, int v, W w = W()) { if (u && v) g.add(u, v, w); }
    // 将汇集覆盖 src 全连接到分发覆盖 dst, 忽略 0, 任一侧仅一个入口时直接连边
    // 均摊时间 O(|src| + |dst|) | 至多 1 个中继, 至多 |src| + |dst| 条边
    template <class S, class D>
    void link(const S& src, const D& dst, W w = W())
    {
        int a = 0, b = 0, u = 0, v = 0;
        for (int x : src) if (x) a++, u = x;
        for (int x : dst) if (x) b++, v = x;
        if (!a || !b) return;
        if (a == 1) { for (int x : dst) add(u, x, w); return; }
        if (b == 1) { for (int x : src) add(x, v, w); return; }
        int p = new_node();
        for (int x : src) add(x, p);
        for (int x : dst) add(p, x, w);
    }
private:
    int cap;
};
#endif

/* Usage
// include graphBuilder.h
int main()
{
    GraphBuilder<Empty> b(10, 16);
    b.init(4); // 已有图点 1 .. 4
    int p = b.new_node();
    b.add(1, p);
    b.link(VI{p, 2}, VI{3, 4});
    cout << b.tot << ' ' << b.g.edge_cnt() << endl; // 6 5
    b.init(2); // 新一轮, 旧入口全部失效
}
*/
