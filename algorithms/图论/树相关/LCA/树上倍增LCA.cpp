#include <iostream>
#include <vector>

using namespace std;

using VI = vector<int>;
using VVI = vector<vector<int>>;

struct LCA
{
    int n;
    const VI& head;
    const VI& to;
    const VI& nxt;
    VVI f;
    VI dep;
    LCA(int _n, const VI& _head, const VI& _to, const VI& _nxt) : n(_n), 
        head(_head), to(_to), nxt(_nxt), 
        f(n + 10, VI(25, 0)), dep(n + 10, 0)
    {

    }
    void dfs(int u, int fa)
    {
        f[u][0] = fa;
        dep[u] = dep[fa] + 1;
        
        for (int i = 1; i <= 20; i++)
        {
            f[u][i] = f[f[u][i - 1]][i - 1];
        }
        
        for (int i = head[u]; i; i = nxt[i])
        {
            int v = to[i];
            if (v == fa) continue;
            dfs(v, u);
        }
    }
    void init()
    {
        dfs(root, 0);
    }

    int get(int u, int v)
    {
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
};