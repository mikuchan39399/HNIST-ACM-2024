#include <vector>
#include <algorithm>
#include <iostream>

using namespace std;

using VI = vector<int>;
using VVI = vector<vector<int>>;

template<typename... Cs>
void z_fill_n(int n, int val, Cs&... containers) 
{
    (fill(containers.begin(), containers.begin() + min((size_t)(n + 10), containers.size()), val), ...);
}

// 记得对边去重
struct EBCC
{
    struct Edge
    {
        int u;
        int v;
        int id;
    };
    
    int n, m;
    int dfn_idx, ebcc_cnt, edge_cnt, t_edge_cnt;
    
    VI head, to, nxt;
    VI t_head, t_to, t_nxt;
    VI dfn, low, bel, sta;
    
    VVI ebcc_points;                    // ebcc 中的原图点集合                                      
    vector<vector<Edge>> ebcc_edges;    // ebcc 中的原图边集合

    EBCC(int _n, int _m) : n(_n), m(_m), 
        head(n + 10, 0), to(m + 10, 0), nxt(m + 10, 0), 
        t_head(n + 10, 0), t_to(m + 10, 0), t_nxt(m + 10, 0), 
        dfn(n + 10, 0), low(n + 10, 0), bel(n + 10, 0), 
        ebcc_points(1, VI{}), ebcc_edges(1, vector<Edge>{})
    {
        dfn_idx = ebcc_cnt = t_edge_cnt = 0;
        edge_cnt = 1;
    }

    void init(int _n)
    {
        n = _n;
        z_fill_n(n, 0, head, t_head, dfn, low, bel);
        dfn_idx = ebcc_cnt = t_edge_cnt = 0;
        edge_cnt = 1;
        
        sta.clear();
        ebcc_points.assign(1, VI{});
        ebcc_edges.assign(1, vector<Edge>{});
    }
    
    void add_edge(int u, int v)
    {
        edge_cnt++;
        to[edge_cnt] = v;
        nxt[edge_cnt] = head[u];
        head[u] = edge_cnt;
    }

    void t_add_edge(int u, int v)
    {
        t_edge_cnt++;
        t_to[t_edge_cnt] = v;
        t_nxt[t_edge_cnt] = t_head[u];
        t_head[u] = t_edge_cnt;
    }

    void tarjan(int u, int in_edge)
    {
        dfn_idx++;
        dfn[u] = low[u] = dfn_idx;
        sta.push_back(u);

        for (int i = head[u]; i; i = nxt[i])
        {
            int v = to[i];
            
            if (i == (in_edge ^ 1)) 
            {
                continue;
            }

            if (!dfn[v])
            {
                tarjan(v, i);
                low[u] = min(low[u], low[v]);
            }
            else
            {
                low[u] = min(low[u], dfn[v]);
            }
        }

        if (low[u] == dfn[u])
        {
            ebcc_cnt++;
            ebcc_points.push_back(VI{});
            ebcc_edges.push_back(vector<Edge>{});
            
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

    void build(int root = -1)
    {
        if (root != -1)
        {
            tarjan(root, 0);
            return;
        }
        for (int i = 1; i <= n; i++)
        {
            if (!dfn[i])
            {
                tarjan(i, 0);
            }
        }
    }

    void build_tree()
    {
        for (int u = 1; u <= n; u++)
        {
            for (int i = head[u]; i; i = nxt[i])
            {
                int v = to[i];
                if (bel[u] != bel[v])
                {
                    t_add_edge(bel[u], bel[v]);
                }
                else if (i % 2 == 0) 
                {
                    ebcc_edges[bel[u]].push_back({u, v, i / 2}); 
                }
            }
        }
    }

    VI get_bridge_edges()
    {
        VI bridges;
        for (int u = 1; u <= n; u++)
        {
            for (int i = head[u]; i; i = nxt[i])
            {
                int v = to[i];
                if (bel[u] != bel[v] && i % 2 == 0)
                {
                    bridges.push_back(i / 2);
                }
            }
        }
        return bridges;
    }
};

// Usage:
/*
const int MAXN = 500005;
const int MAXM = 1000005;

EBCC graph(MAXN, MAXM * 2); // 用户控制 无向边 / 有向边

void dfs_tree(int u, int fa, VI& vis)
{
    vis[u] = 1;
    int point_cnt = graph.ebcc_points[u].size();
    
    // 直接通过结构体成员访问
    for (int i = 0; i < graph.ebcc_edges[u].size(); i++)
    {
        int from = graph.ebcc_edges[u][i].u;
        int to = graph.ebcc_edges[u][i].v;
        int id = graph.ebcc_edges[u][i].id;
        // ... 二次处理 ...
    }

    for (int i = graph.t_head[u]; i; i = graph.t_nxt[i])
    {
        int v = graph.t_to[i];
        if (v == fa)
        {
            continue;
        }
        dfs_tree(v, u, vis);
    }
}

void solve()
{
    int n, m; 
    cin >> n >> m;
    
    // graph.init(n); 
    
    for (int i = 1; i <= m; i++)
    {
        int u, v; 
        cin >> u >> v;
        graph.add_edge(u, v);
        graph.add_edge(v, u);
    }
    
    graph.build();
    graph.build_tree();
    
    VI bridges = graph.get_bridge_edges();
    
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