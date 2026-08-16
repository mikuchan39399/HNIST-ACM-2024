#include <vector>
#include <algorithm>
#include <iostream>
#include <cassert>

using namespace std;
using VI = vector<int>;
using VVI = vector<vector<int>>;
using PII = pair<int, int>;
using VPII = vector<PII>;

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
        for (int i = 0; i < used.size(); i++)
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
    int count() const { return used.size(); } 
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
    assert(((int)cs.size() >= n && ...));
    (fill(cs.begin(), cs.begin() + min((size_t)(n + 10), cs.size()), val), ...);
}
#endif
struct SCC
{
    int n;
    int dfn_idx, scc_cnt;
    Graph<true, Empty> g;      // 原有向图
    Graph<true, Empty> dag;    // 缩点后的 DAG
    VI dfn, low, bel, in_stk, sta;
    SCC(int max_n = 0, int max_m = 0) : n(max_n), dfn_idx(0), scc_cnt(0), 
        g(max_n, max_m), dag(max_n, max_m),
        dfn(max_n + 10, 0), low(max_n + 10, 0), 
        bel(max_n + 10, 0), in_stk(max_n + 10, 0)
    {
        sta.reserve(max_n + 10);
    }
    void init(int _n)
    {
        n = _n;
        g.clear();
        dag.clear();
        z_fill_n(n, 0, dfn, low, bel, in_stk);
        dfn_idx = scc_cnt = 0;
        sta.clear();
    }
    void add_edge(int u, int v) { g.add(u, v); }
    void tarjan(int u)
    {
        dfn_idx++;
        low[u] = dfn[u] = dfn_idx;
        sta.push_back(u);
        in_stk[u] = 1;
        for (auto& e : g[u])
        {
            int v = e.v;
            if (!dfn[v])
            {
                tarjan(v);
                low[u] = min(low[u], low[v]);
            }
            else if (in_stk[v]) low[u] = min(low[u], dfn[v]);
        }
        if (low[u] == dfn[u])
        {
            scc_cnt++;
            int t;
            do
            {
                t = sta.back();
                sta.pop_back();
                in_stk[t] = 0;
                bel[t] = scc_cnt;
            } while (t != u);
        }
    }
    void build()
    {
        for (int i = 1; i <= n; i++)
            if (!dfn[i]) tarjan(i);
    }
    // 构建缩点后的 DAG
    void build_dag()
    {
        for (int u = 1; u <= n; u++)
        {
            for (auto& e : g[u])
            {
                int v = e.v;
                if (bel[u] != bel[v]) dag.add(bel[u], bel[v]);
            }
        }
    }
    // 去重边 DAG
    void build_dag_unique()
    {
        VPII edges;
        for (int u = 1; u <= n; u++)
        {
            for (auto& e : g[u])
            {
                int v = e.v;
                if (bel[u] != bel[v]) edges.push_back({bel[u], bel[v]});
            }
        }
        sort(edges.begin(), edges.end());
        edges.erase(unique(edges.begin(), edges.end()), edges.end());
        for (auto& edge : edges)
            dag.add(edge.first, edge.second);
    }
};

// Usage: 
/*
const int MAXN = 500005;
const int MAXM = 1000005;
SCC graph(MAXN, MAXM);

int scc_val[MAXN]; 
int dp[MAXN]; // 记录到达每个 SCC 的最大权值和

void solve()
{
    int n, m;
    cin >> n >> m;
    
    graph.init(n);
    
    VI val(n + 1);
    for (int i = 1; i <= n; i++)
    {
        cin >> val[i];
        scc_val[i] = 0; 
        dp[i] = 0;
    }
    
    for (int i = 1; i <= m; i++)
    {
        int u, v;
        cin >> u >> v;
        graph.add_edge(u, v);
    }
    
    // 1. 跑 Tarjan 找 SCC
    graph.build();
    
    // 2. 将原图权值累加到对应的 SCC 新节点上
    for (int i = 1; i <= n; i++)
    {
        scc_val[graph.bel[i]] += val[i];
    }
    
    // 3. 构建 DAG
    graph.build_dag();
    
    // 4. DAG 上 DP (利用 Tarjan 自带的拓扑序)
    // 初始化 DP 数组为当前点权
    for (int i = 1; i <= graph.scc_cnt; i++)
    {
        dp[i] = scc_val[i];
    }
    
    // Tarjan 的编号(scc_cnt)越大，在拓扑序中越靠前(靠近源点)
    // 逆序遍历 scc_cnt，等价于正向拓扑排序遍历
    for (int u = graph.scc_cnt; u >= 1; u--)
    {
        for (auto& e : graph.dag[u])
        {
            int v = e.v;
            dp[v] = max(dp[v], dp[u] + scc_val[v]);
        }
    }
    
    // 统计整张图的最长路
    int ans = 0;
    for (int i = 1; i <= graph.scc_cnt; i++)
    {
        ans = max(ans, dp[i]);
    }
    cout << ans << endl;
}
*/