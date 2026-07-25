#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

typedef long long LL;

struct TreeCentroid
{
    int n;
    int edge_cnt;
    vector<int> head;
    vector<int> to;
    vector<int> nxt;
    
    vector<LL> point;
    vector<LL> sz;
    vector<int> centroids;  
    LL min_max_part;        
    LL total_weight;        // 整棵树的点权总和

    TreeCentroid(int max_n)
    {
        int max_edges = (max_n - 1) * 2;
        
        head.assign(max_n + 10, 0);
        to.assign(max_edges + 10, 0);
        nxt.assign(max_edges + 10, 0);
        point.assign(max_n + 10, 1); 
        sz.assign(max_n + 10, 0);
    }

    void init(int _n)
    {
        n = _n;
        edge_cnt = 0;
        min_max_part = 9e18;
        total_weight = 0;
        centroids.clear();
        
        for (int i = 1; i <= n; i++)
        {
            head[i] = 0;
            point[i] = 1; // 默认重置点权为 1
        }
    }

    void add_edge(int u, int v)
    {
        edge_cnt++;
        to[edge_cnt] = v;
        nxt[edge_cnt] = head[u];
        head[u] = edge_cnt;
    }

    void dfs(int u, int fa)
    {
        sz[u] = point[u];
        LL max_part = 0; 

        for (int i = head[u]; i != 0; i = nxt[i])
        {
            int v = to[i];
            if (v == fa) 
            {
                continue;
            }
            
            dfs(v, u);
            sz[u] += sz[v];
            max_part = max(max_part, sz[v]); 
        }

        max_part = max(max_part, total_weight - sz[u]); 

        if (max_part < min_max_part)
        {
            min_max_part = max_part;
            centroids.clear();
            centroids.push_back(u);
        }
        else if (max_part == min_max_part)
        {
            centroids.push_back(u);
        }
    }

    void build()
    {
        total_weight = 0;
        for (int i = 1; i <= n; i++)
        {
            total_weight += point[i];
        }
        
        dfs(1, 0); 
        sort(centroids.begin(), centroids.end()); 
    }

    void fill_point()
    {
        for (int i = 1; i <= n; i++)
        {
            cin >> point[i];
        }
    }
};

/*
 * 使用示例：
 * const int MAXN = 200005;
 * TreeCentroid tree(MAXN);
 * void solve()
 * {
    * int n;
    * cin >> n;
    * tree.init(n);
    * // 如果题目带点权，在此处调用 tree.fill_point();
    * for (int i = 1; i < n; i++)
    * {
        * int u, v;
        * cin >> u >> v;
        * tree.add_edge(u, v);
        * tree.add_edge(v, u);
    * }
    * tree.build();
    * for (int i = 0; i < tree.centroids.size(); i++)
    * {
        * cout << tree.centroids[i] << (i == tree.centroids.size() - 1 ? "" : " ");
    * }
    * cout << "\n";
 * }
 */