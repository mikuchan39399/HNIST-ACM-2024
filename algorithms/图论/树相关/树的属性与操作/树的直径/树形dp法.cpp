#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

typedef long long LL;
using VI = vector<int>;
using VLL = vector<LL>;

struct TreeDiameterDP
{
    int n;
    int edge_cnt;
    VI head, to, nxt;
    VLL w;
    LL ans;

    TreeDiameterDP(int _n, int _m)
    {
        int max_edges = (_n - 1) * 2;
        head.assign(_n + 10, 0);
        to.assign(_m + 10, 0);
        nxt.assign(_m + 10, 0);
        w.assign(_m + 10, 0);
    }

    void init(int _n)
    {
        n = _n;
        edge_cnt = 0;
        ans = -1e18;
        for (int i = 1; i <= n; i++)
        {
            head[i] = 0;
        }
    }

    void add_edge(int u, int v, LL w = 1)
    {
        edge_cnt++;
        to[edge_cnt] = v;
        w[edge_cnt] = w;
        nxt[edge_cnt] = head[u];
        head[u] = edge_cnt;
    }

    LL dfs(int u, int fa)
    {
        LL down = 0;
        for (int i = head[u]; i != 0; i = nxt[i])
        {
            int v = to[i];
            if (v == fa) 
            {
                continue;
            }
            LL cur_subl = dfs(v, u) + w[i];
            ans = max(ans, down + cur_subl);
            down = max(down, cur_subl);
        }
        ans = max(ans, down);
        return down;
    }

    LL build()
    {
        dfs(1, 0);
        return ans;
    }
};

/*
 * 使用示例：
 * void solve()
 * {
    * int n;
    * cin >> n;
    * TreeDiameterDP tree(n); 
    * for (int i = 1; i < n; i++) 
    * {
        * int u, v, w;
        * cin >> u >> v >> w;
        * tree.add_edge(u, v, w);
        * tree.add_edge(v, u, w);
    * }
    * cout << tree.build() << endl;
 * }
 */