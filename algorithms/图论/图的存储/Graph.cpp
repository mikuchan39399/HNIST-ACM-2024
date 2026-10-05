// zoi: graph
#ifndef Z_OI_GRAPH
#define Z_OI_GRAPH

#include "../../杂项/utils/utils.cpp"

#ifndef Z_OI_EMPTY
#define Z_OI_EMPTY
struct Empty {};
#endif

// 链式前向星; 点 1-based, 半边 0-based, 邻接按加边逆序遍历
// 无向边占两条半边, 自环计 2 度; deg 为出度/无向度, in_deg 为有向入度
// 点数自行保存; max_m 为预留逻辑边数, 可扩容. 加边时勿持有边引用或遍历邻接表
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
    // 预留 max_n 个点、max_m 条边, 点号须在 1..max_n. 时间 O(max_n), 空间 O(max_n+max_m)
    Graph(int max_n = 0, int max_m = 0) :
        head(max_n + 10, -1), deg(max_n + 10, 0)
    {
        if constexpr (Dir) in_deg.assign(max_n + 10, 0);
        used.reserve(max_n + 10);
        edges.reserve(max_m * (Dir ? 1 : 2) + 10);
    }
    // 清空并保留容量. 时间 O(触碰点数+半边数), 额外空间 O(1)
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
    // 加边, 返回首条半边号(无向图为偶数). 均摊 O(1), 扩容时空 O(m)
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
    // 触碰过的点数, 不含孤立点. O(1)
    int node_cnt() const { return used.size(); }
    // 逻辑边数. O(1)
    int edge_cnt() const { return (int)edges.size() / (Dir ? 1 : 2); }
    // 无向半边 i 的反向半边号. O(1)
    int rev(int i) const { return i ^ 1; }
    // 边引用 e 的半边号, 不能传副本. O(1)
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
    // u 的邻接范围, auto& 可改权值; 遍历 O(deg[u]), 额外空间 O(1)
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
