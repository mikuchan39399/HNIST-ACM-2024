#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

typedef long long LL;
using VI = vector<int>;
using VLL = vector<LL>;

struct TreeDiameter
{
    int n;
    int edge_cnt;
    VI head, to, nxt;
    VLL w;
    
    int farthest_node;
    LL max_dist;

    VI pre;
    VLL pre_w;
    VI path;
    vector<bool> in_path;
    VLL max_d;
    int len, com_len;
    TreeDiameter(int max_n)
    {
        int max_edges = (max_n - 1) * 2;
        
        head.assign(max_n + 10, 0);
        to.assign(max_edges + 10, 0);
        nxt.assign(max_edges + 10, 0);
        w.assign(max_edges + 10, 0);
        
        pre.assign(max_n + 10, 0);
        pre_w.assign(max_n + 10, 0);
        in_path.assign(max_n + 10, false);
        max_d.assign(max_n + 10, 0);
    }

    void init(int _n)
    {
        n = _n;
        edge_cnt = 0;
        path.clear();
        
        for (int i = 1; i <= n; i++)
        {
            head[i] = 0;
            in_path[i] = false;
        }
    }

    void add_edge(int u, int v, LL weight = 1)
    {
        edge_cnt++;
        to[edge_cnt] = v;
        w[edge_cnt] = weight;
        nxt[edge_cnt] = head[u];
        head[u] = edge_cnt;
    }

    void dfs(int u, int fa, LL cur_d)
    {
        if (cur_d > max_dist)
        {
            max_dist = cur_d;
            farthest_node = u;
        }

        for (int i = head[u]; i != 0; i = nxt[i])
        {
            int v = to[i];
            if (v == fa)
            {
                continue;
            }
            pre[v] = u;
            pre_w[v] = w[i];
            dfs(v, u, cur_d + w[i]);
        }
    }

    LL get_max_d(int u, int fa)
    {
        LL md = 0;
        for (int i = head[u]; i != 0; i = nxt[i])
        {
            int v = to[i];
            if (v == fa || in_path[v])
            {
                continue;
            }
            md = max(md, w[i] + get_max_d(v, u));
        }
        return md;
    }

    void build()
    {
        max_dist = -1;
        dfs(1, 0, 0);
        int A = farthest_node;

        max_dist = -1;
        pre[A] = 0; 
        dfs(A, 0, 0);
        int B = farthest_node;

        int cur = B;
        while (cur != 0)
        {
            path.push_back(cur);
            cur = pre[cur];
        }
        reverse(path.begin(), path.end()); 
        int k = path.size();
        for (int i = 0; i < k; i++)
        {
            in_path[path[i]] = true; 
        }
        
        VLL dist_A(k, 0);
        for (int i = 1; i < k; i++)
        {
            dist_A[i] = dist_A[i - 1] + pre_w[path[i]];
        }
        len = dist_A[k - 1];

        for (int i = 0; i < k; i++)
        {
            max_d[path[i]] = get_max_d(path[i], 0);
        }

        int L = 0;
        int R = k - 1;
        
        for (int i = 0; i < k; i++)
        {
            if (max_d[path[i]] == dist_A[i])
            {
                L = i; 
            }
        }
        
        for (int i = k - 1; i >= 0; i--)
        {
            if (max_d[path[i]] == len - dist_A[i]) 
            {
                R = i;
            }
        }
        if (R > L)
        {
            com_len = dist_A[R] - dist_A[L];
        }
        else
        {
            com_len = 0;
        }
    }
};

/*
 * 使用示例：
 * const int MAXN = 200005;
 * TreeDiameter tree(MAXN);
 * void solve()
 * {
    * int n;
    * cin >> n;
    * tree.init(n);
    * for (int i = 1; i < n; i++)
    * {
        * int u, v;
        * LL w;
        * cin >> u >> v >> w;
        * tree.add_edge(u, v, w);
        * tree.add_edge(v, u, w);
    * }
    * tree.build();
    * cout << tree.len << endl;
    * cout << tree.com_len << endl;
 * }
 */