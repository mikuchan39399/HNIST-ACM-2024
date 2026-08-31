#include <iostream>
#include <vector>

using namespace std;
using VI = vector<int>;

struct TarjanLCA
{
    int n, m;
    int edge_cnt, q_cnt;
    
    // 树边前向星
    VI head;
    VI to;
    VI nxt;
    
    // 询问前向星
    VI q_head;
    VI q_to;
    VI q_nxt;
    VI q_id;

    VI fa;
    VI vis;
    VI ans;

    TarjanLCA(int _n, int _m) : n(_n), m(_m),
        edge_cnt(0), q_cnt(0),
        head(n + 10, 0), to(2 * n + 10, 0), nxt(2 * n + 10, 0),
        q_head(n + 10, 0), q_to(m + 10, 0), q_nxt(m + 10, 0), q_id(m + 10, 0),
        fa(n + 10, 0), vis(n + 10, 0), ans(m + 10, 0)
    {
        for (int i = 1; i <= n; i++)
        {
            fa[i] = i;
        }
    }

    void init(int _n, int _m)
    {
        n = _n;
        m = _m;
        edge_cnt = 0;
        q_cnt = 0;
        
        for (int i = 1; i <= n; i++)
        {
            head[i] = 0;
            q_head[i] = 0;
            fa[i] = i;
            vis[i] = 0;
        }
    }

    void add_edge(int u, int v)
    {
        edge_cnt++;
        to[edge_cnt] = v;
        nxt[edge_cnt] = head[u];
        head[u] = edge_cnt;
    }

    void add_query(int u, int v, int id)
    {
        q_cnt++;
        q_to[q_cnt] = v;
        q_id[q_cnt] = id;
        q_nxt[q_cnt] = q_head[u];
        q_head[u] = q_cnt;
    }

    void build()
    {
        for (int i = 1; i <= n; i++)
        {
            if (!vis[i])
            {
                tarjan(i);
            }
        }
    }
private:
    int find(int x)
    {
        if (fa[x] == x) 
        {
            return x;
        }
        return fa[x] = find(fa[x]);
    }

    void tarjan(int u)
    {
        vis[u] = 1;
        for (int i = head[u]; i; i = nxt[i])
        {
            int v = to[i];
            if (vis[v]) 
            {
                continue;
            }
            tarjan(v);
            fa[v] = u;
        }
        
        for (int i = q_head[u]; i; i = q_nxt[i])
        {
            int v = q_to[i];
            if (vis[v])
            {
                ans[q_id[i]] = find(v);
            }
        }
    }
};

/* 
void solve()
{
    int n, m, root; 
    cin >> n >> m >> root;
    
    // 显式传入 2 * m
    TarjanLCA lca(n, 2 * m);
    
    for (int i = 1; i < n; i++)
    {
        int u, v; 
        cin >> u >> v;
        lca.add_edge(u, v);
        lca.add_edge(v, u);
    }
    
    for (int i = 1; i <= m; i++)
    {
        int u, v; 
        cin >> u >> v;
        lca.add_query(u, v, i);
        lca.add_query(v, u, i);
    }
    
    lca.build();
    
    for (int i = 1; i <= m; i++)
    {
        cout << lca.ans[i] << '\n';
    }
}
*/