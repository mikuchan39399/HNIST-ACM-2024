// zoi: graphBuilder
#ifndef Z_OI_GRAPH_BUILDER
#define Z_OI_GRAPH_BUILDER

#include "../图的存储/Graph.cpp"

// 共享有向图与编号; 1..n 为初始点, 辅助点均由 new_node 分配, W() 为零权
// 结构使用期间不销毁/移动上下文; init 使旧结构及入口失效
template <class W = Empty>
struct GraphBuilder
{
    Graph<true, W> g;
    int n = 0, tot = 0;
    // 点容量 max_n=初始点+骨架点+中继点, 固定; max_m 为边预留, 可扩容
    // 构造后先 init(n). 时间 O(max_n), 空间 O(max_n+max_m)
    GraphBuilder(int max_n = 0, int max_m = 0) : g(max_n, max_m), cap(max_n) {}
    // 清图并保留 1..n, n<=容量. 时间 O(旧触碰点数+旧边数)
    void init(int _n)
    {
        assert(0 <= _n && _n <= cap);
        g.clear();
        n = tot = _n;
    }
    // 申请并返回新点号, 消耗 1 点容量. O(1)
    int new_node()
    {
        assert(tot < cap);
        return ++tot;
    }
    // 加 u->v, 任一端为 0 则忽略. 均摊 O(1), 至多 1 边
    void add(int u, int v, W w = W()) { if (u && v) g.add(u, v, w); }
    // src 汇集出口到 dst 分发入口全连接, 忽略 0; 须同一上下文
    // 均摊 O(|src|+|dst|), 至多 1 中继点、|src|+|dst| 条边
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
