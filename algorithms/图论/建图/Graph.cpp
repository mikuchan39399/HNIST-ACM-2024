#include <vector>
using namespace std;
using VI = vector<int>;

#ifndef Z_OI_EMPTY
#define Z_OI_EMPTY
struct Empty {};
#endif

#ifndef Z_OI_GRAPH
#define Z_OI_GRAPH
template <bool Dir = false, class W = Empty>
struct Graph 
{
    struct Edge 
    {
        int v, nxt;                 
        [[no_unique_address]] W w;  
    };
    VI head, used;
    VI deg;     // 无向图的度数 / 有向图的出度
    VI in_deg;  // 有向图的入度(仅 Dir = true 时有效) 
    vector<Edge> edges;     
    Graph(int max_n = 0, int max_m = 0) : 
        head(max_n + 10, -1), deg(max_n + 10, 0)
    {
        if constexpr (Dir) in_deg.assign(max_n + 10, 0);
        used.reserve(max_n + 10);
        edges.reserve(max_m * (Dir ? 1 : 2) + 10); 
    }
    void clear() // O(used) 擦除, 多测复用
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
    int node_cnt() const { return used.size(); } // 触碰过的点数 
    int edge_cnt() const { return (int)edges.size() / (Dir ? 1 : 2); } // 逻辑边数
    int rev(int i) const { return i ^ 1; } // 无向半边 i 的对偶半边 ( Dir = true 时无意义)
    int id(const Edge& e) const { return &e - edges.data(); } // 只能对遍历中的活引用调用
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
    Adj operator[](int u) { return {*this, u}; }
};
#endif