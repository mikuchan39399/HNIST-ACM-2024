#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

using VI = vector<int>;
using VVI = vector<vector<int>>;

const int inf = 0x3f3f3f3f;

struct LCA
{
    int n;
    const VI& head;
    const VI& to;
    const VI& nxt;
    VVI f;
    VI dep, rt;

    LCA(int _n, const VI& _head, const VI& _to, const VI& _nxt) : n(_n), 
        head(_head), to(_to), nxt(_nxt)
    {
        f.assign(n + 10, VI(25, 0));
        dep.assign(n + 10, 0);
        rt.assign(n + 10, 0);
    }
    
    void build()
    {
        for (int i = 1; i <= n; i++)
        {
            if (!rt[i])
            {
                dfs(i, 0, i);
            }
        }
    }

    int lca(int u, int v)
    {
        if (rt[u] != rt[v])
        {
            return inf;
        }

        if (dep[u] < dep[v]) swap(u, v);
        
        for (int i = 20; i >= 0; i--)
        {
            if (dep[f[u][i]] >= dep[v])
            {
                u = f[u][i];
            }
        }
        
        if (u == v) return u;

        for (int i = 20; i >= 0; i--)
        {
            if (f[u][i] != f[v][i])
            {
                u = f[u][i];
                v = f[v][i];
            }
        }
        return f[u][0];
    }
private:
    void dfs(int u, int p, int root)
    {
        f[u][0] = p;
        dep[u] = dep[p] + 1;
        rt[u] = root;
        
        for (int i = 1; i <= 20; i++)
        {
            f[u][i] = f[f[u][i - 1]][i - 1];
        }
        
        for (int i = head[u]; i; i = nxt[i])
        {
            int v = to[i];
            if (v == p) continue;
            dfs(v, u, root);
        }
    }
};