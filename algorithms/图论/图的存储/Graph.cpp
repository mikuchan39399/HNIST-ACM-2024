// zoi: graph
#ifndef Z_OI_GRAPH
#define Z_OI_GRAPH

#include "../../杂项/utils/utils.cpp"

#ifndef Z_OI_EMPTY
#define Z_OI_EMPTY
struct Empty {};
#endif

// 链式前向星, 点编号 1-based, 半边编号 0-based, 邻接表按加边的逆序遍历
// head 存首条半边, used 存触碰过的点, deg 存出度或无向度数, in_deg 仅有向图使用
// 无向边存两条半边, 自环贡献 2 度; 孤立点不计入 node_cnt(), 点数由调用方保存
// 每半边 Empty 为 8 B, LL 为 16 B; 预留半边数 = max_m * (Dir ? 1 : 2) + 10
// 点表约 (Dir ? 16 : 12) * max_n B; 无向 LL 图 n = m = 2e5 时合计约 8.8 MB
// max_m 仅作预留提示, 边数组可扩容且编号不变; 加边期间不持有边引用或遍历邻接表
// 默认拷贝相互独立但不保证保留预留容量, 移动后源对象仅用于析构或重新赋值
template <bool Dir = false, class W = Empty>
struct Graph
{
    struct Edge
    {
        int v, nxt;
        [[no_unique_address]] W w;
    };
    VI head, used;
    VI deg;
    VI in_deg;
    vector<Edge> edges;
    // 分配 max_n 个点的状态表并预留 max_m 条逻辑边, 点编号须在 1 到 max_n 内
    // 时间 O(max_n) | 空间 O(max_n + max_m)
    Graph(int max_n = 0, int max_m = 0) :
        head(max_n + 10, -1), deg(max_n + 10, 0)
    {
        if constexpr (Dir) in_deg.assign(max_n + 10, 0);
        used.reserve(max_n + 10);
        edges.reserve(max_m * (Dir ? 1 : 2) + 10);
    }
    // 清空边和触碰过的点状态, 保留容器容量
    // 时间 O(k + m) | 额外空间 O(1), k 为触碰点数, m 为半边数; 标量权值无需逐边析构
    void clear()
    {
        for (size_t i = 0; i < used.size(); i++)
        {
            int u = used[i];
            head[u] = -1;
            deg[u] = 0;
            if constexpr (Dir) in_deg[u] = 0;
        }
        used.clear();
        edges.clear();
    }
    // 添加 u 到 v 的边并返回首条半边编号, 无向图同时添加反向边, 首编号为偶数
    // 均摊时间 O(1) | 单次扩容时间和额外空间 O(m), m 为已有半边数, 权值复制按 O(1) 计
    int add(int u, int v, const W& w = W())
    {
        auto mark = [&](int x)
        {
            bool first = (head[x] == -1) && (deg[x] == 0);
            if constexpr (Dir) first = first && (in_deg[x] == 0);
            if (first) used.push_back(x);
        };
        mark(u);
        if (u != v) mark(v);
        int idx = edges.size();
        edges.push_back({v, head[u], w});
        head[u] = idx;
        deg[u]++;
        if constexpr (!Dir)
        {
            edges.push_back({u, head[v], w});
            head[v] = idx + 1;
            deg[v]++;
        }
        else in_deg[v]++;
        return idx;
    }
    // 返回加边触碰过的点数, 不含孤立点
    // 时间 O(1) | 空间 O(1)
    int node_cnt() const { return used.size(); }
    // 返回逻辑边数, 无向边的两条半边只计一次
    // 时间 O(1) | 空间 O(1)
    int edge_cnt() const { return (int)edges.size() / (Dir ? 1 : 2); }
    // 返回无向半边 i 的对偶编号, 仅用于无向图
    // 时间 O(1) | 空间 O(1)
    int rev(int i) const { return i ^ 1; }
    // 返回当前图中边引用 e 的半边编号, 不能传边的副本
    // 时间 O(1) | 空间 O(1)
    int id(const Edge& e) const { return &e - edges.data(); }
    struct Iter
    {
        Graph& g; int e;
        Edge& operator*() { return g.edges[e]; }
        Edge* operator->() { return &g.edges[e]; }
        Iter& operator++() { e = g.edges[e].nxt; return *this; }
        bool operator!=(const Iter& o) const { return e != o.e; }
    };
    struct Adj
    {
        Graph& g; int u;
        Iter begin() { return {g, g.head[u]}; }
        Iter end() { return {g, -1}; }
    };
    // 返回 u 的邻接范围, 用 auto& e 遍历可读取或修改半边权值
    // 时间 O(1), 遍历 O(deg[u]) | 空间 O(1)
    Adj operator[](int u) { return {*this, u}; }
};
#endif

/* Usage
int n = 3, m = 2;
Graph<false, LL> g(n, m);
int e = g.add(1, 2, 5);       // e = 0, g.rev(e) = 1
g.add(2, 3, 7);
for (auto& edge : g[2])
{
    int v = edge.v;           // 依次访问 3, 1
    LL w = edge.w;
    int i = g.id(edge);       // 当前半边编号
    (void)v; (void)w; (void)i;
}
g.clear();                   // 下一测复用, 点数仍由调用方保存
Graph<true> dag(n, m);
dag.add(1, 2);               // 无权有向图, deg 为出度, in_deg 为入度
*/
