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
    int idx = 0;
    const VI& head;  
    const VI& to; 
    const VI& nxt;
    VI dep, dfn, fa, rt; // rt 数组记录所在连通块的根节点，用来判断森林情况的 LCA 合法性
    VVI rmq;
    
    LCA(int _n, const VI& _head, const VI& _to, const VI& _nxt) : n(_n), 
        head(_head), to(_to), nxt(_nxt)
    {
        dep.assign(n + 10, 0);
        dfn.assign(n + 10, 0);
        fa.assign(n + 10, 0);
        rt.assign(n + 10, 0); 
        rmq.assign(n + 10, VI((!n ? 0 : __lg(n)) + 10, 0));
    }
    
    void dfs(int u, int p, int root)
    {
        fa[u] = p;
        rt[u] = root;
        dep[u] = dep[p] + 1;
        idx++;
        dfn[u] = idx;
        rmq[idx][0] = u;
        for (int i = head[u]; i; i = nxt[i])
        {
            int v = to[i];
            if (v == p) continue;
            dfs(v, u, root);
        }
    }

    void build()
    {
        idx = 0;
        for (int i = 1; i <= n; i++)
        {
            if (!fa[i]) 
            {
                dfs(i, 0, i);
            }
        }
        
        int log_n = n == 0 ? 0 : __lg(n);
        for (int k = 1; k <= log_n; k++)
        {
            for (int i = 1; i + (1 << k) - 1 <= n; i++)
            {
                int u = rmq[i][k - 1];
                int v = rmq[i + (1 << (k - 1))][k - 1];
                rmq[i][k] = dep[u] < dep[v] ? u : v; 
            }
        }
    }

    int lca(int u, int v)
    {
        if (rt[u] != rt[v]) 
        {
            return inf;
        }
        
        if (u == v) return u;
        int l = dfn[u], r = dfn[v];
        if (l > r) swap(l, r);
        l++;
        int k = __lg(r - l + 1);
        int u_node = rmq[l][k];
        int v_node = rmq[r - (1 << k) + 1][k];
        int w = dep[u_node] < dep[v_node] ? u_node : v_node;
        return fa[w];
    }
};