#include <iostream>
#include <vector>
#include <cmath>

using namespace std;

using VI = vector<int>;
using VVI = vector<vector<int>>;

struct LCA
{
    int n;
    int idx = 0;
    const VI& head;  
    const VI& to; 
    const VI& nxt;
    VI dep, pre;
    VVI rmq;
    LCA(int _n, const VI& _head, const VI& _to, const VI& _nxt) : n(_n), 
        head(_head), to(_to), nxt(_nxt)
    {
        dep.assign(n + 1, 0);
        pre.assign(n + 1, 0);
        rmq.assign(2 * n + 1, VI(floor(log2(2 * n)) + 10, 0));
    }
    void build(int root = 1)
    {
        idx = 0;
        dfs(root, 0);
        for (int k = 1; (1 << k) <= idx; k++)
        {
            for (int i = 1; i + (1 << k) - 1 <= idx; i++)
            {
                int u = rmq[i][k - 1];
                int v = rmq[i + (1 << (k - 1))][k - 1];
                rmq[i][k] = dep[u] < dep[v] ? u : v; 
            }
        }
    }

    int lca(int u, int v)
    {
        if (u == v) return u;
        int l = pre[u], r = pre[v];
        if (l > r) swap(l, r);
        int k = floor(log2(r - l + 1));
        u = rmq[l][k];
        v = rmq[r - (1 << k) + 1][k];
        return dep[u] < dep[v] ? u : v;
    }
private:
    void dfs(int u, int fa)
    {
        dep[u] = dep[fa] + 1;
        idx++;
        rmq[idx][0] = u;
        pre[u] = idx;
        for (int i = head[u]; i; i = nxt[i])
        {
            int v = to[i];
            if (v == fa) continue;
            dfs(v, u);
            idx++;
            rmq[idx][0] = u;
        }
    }
};