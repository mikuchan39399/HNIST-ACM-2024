#include <vector>
#include <algorithm>
#include <iostream>
#include <cassert>
using namespace std;
using VI = vector<int>;
using VVI = vector<vector<int>>;

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

#ifndef Z_OI_ZFILLN
#define Z_OI_ZFILLN
template<typename... CS>
void z_fill_n(int n, int val, CS&... cs) 
{
    assert(((((int)cs.size()) >= n) && ...));
    (fill(cs.begin(), cs.begin() + min((size_t)(n + 10), cs.size()), val), ...);
}
#endif
// 分清题目中冗余的重边和故意的重边, 只有前者需要去重
struct EBCC 
{
    struct EData { int id; }; // 将原图的边编号作为有效载荷
    int n;
    int dfn_idx, ebcc_cnt;
    Graph<false, EData> g;    // 原图
    Graph<false, Empty> tree; // 缩点后建的桥树（无权无向图）
    VI dfn, low, bel, sta;
    VVI ebcc_points;          // ebcc_points[i] 存第 i 个 EBCC 里的所有原图点
    EBCC(int max_n = 0, int max_m = 0) : n(max_n), dfn_idx(0), ebcc_cnt(0), 
        g(max_n, max_m), tree(max_n, max_n), 
        dfn(max_n + 10, 0), low(max_n + 10, 0), bel(max_n + 10, 0),
        ebcc_points(1, VI{})
    {
        sta.reserve(max_n + 10);
    }
    void init(int _n) 
    {
        n = _n;
        g.clear();
        tree.clear();
        z_fill_n(n, 0, dfn, low, bel);
        dfn_idx = ebcc_cnt = 0;
        sta.clear();
        ebcc_points.assign(1, VI()); 
    }
    void add_edge(int u, int v, int eid) { g.add(u, v, {eid}); }
    void tarjan(int u, int in_edge) 
    {
        dfn_idx++;
        dfn[u] = low[u] = dfn_idx;
        sta.push_back(u);
        for (auto& e : g[u]) 
        {
            int i = g.id(e);
            if (in_edge == g.rev(i)) continue; 
            int v = e.v;
            if (!dfn[v]) 
            {
                tarjan(v, i);
                low[u] = min(low[u], low[v]);
            } 
            else low[u] = min(low[u], dfn[v]);
        }
        if (low[u] == dfn[u]) 
        {
            ebcc_cnt++;
            ebcc_points.push_back(VI());
            int t;
            do 
            {
                t = sta.back();
                sta.pop_back();
                bel[t] = ebcc_cnt;
                ebcc_points[ebcc_cnt].push_back(t);
            } while (t != u);
        }
    }
    void build() 
    {
        for (int i = 1; i <= n; i++) 
            if (!dfn[i]) tarjan(i, -1);
    }
    // 缩点建树
    void build_tree() 
    {
        // 遍历所有正向边
        for (size_t i = 0; i < g.edges.size(); i += 2) 
        {
            int u = g.edges[g.rev(i)].v; // 利用反向边的终点，O(1) 获取当前边的起点
            int v = g.edges[i].v;
            if (bel[u] != bel[v]) tree.add(bel[u], bel[v]);
        }
    }
    VI get_bridges() // 获取所有割边的逻辑编号
    {
        VI bridges;
        for (size_t i = 0; i < g.edges.size(); i += 2) 
        {
            int u = g.edges[g.rev(i)].v;
            int v = g.edges[i].v;
            if (bel[u] != bel[v]) bridges.push_back(g.edges[i].w.id);
        }
        return bridges;
    }
};

// Usage: 
/*
const int MAXN = 500005;
const int MAXM = 1000005;

EBCC graph(MAXN, MAXM);

void dfs_tree(int u, int fa, VI& vis) 
{
    vis[u] = 1;
    for (auto& e : graph.tree[u]) 
    {
        int v = e.v;
        if (v == fa) continue;
        dfs_tree(v, u, vis);
    }
}

void solve() 
{
    int n, m; cin >> n >> m;
    graph.init(n); 
    for (int i = 1; i <= m; i++) 
    {
        int u, v; cin >> u >> v;
        if (u == v) continue;
        // 自动建无向边，无需正反加两次
        graph.add_edge(u, v, i); 
    }
    graph.build();       // 跑 Tarjan 求 EBCC
    graph.build_tree();  // 生成缩点桥树
    VI bridges = graph.get_bridges(); // 获取割边(桥)集合
    // 遍历桥树的连通块
    VI vis(graph.ebcc_cnt + 1, 0);
    for (int i = 1; i <= graph.ebcc_cnt; i++) 
    {
        if (!vis[i]) 
        {
            dfs_tree(i, 0, vis);
        }
    }
}
*/