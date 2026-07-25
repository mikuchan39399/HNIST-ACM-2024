#include <iostream>
#include <vector>

using namespace std;

struct ForwardStar
{
    int edge_cnt;
    vector<int> head;  
    vector<int> to;    
    vector<int> nxt;    
    vector<int> weight; 

    ForwardStar(int n, int m)
    {
        edge_cnt = 0;
        head.assign(n + 1, 0);
        to.assign(m + 1, 0);
        nxt.assign(m + 1, 0);
        weight.assign(m + 1, 0);
    }

    void add_edge(int u, int v, int w = 0)
    {
        edge_cnt++;
        to[edge_cnt] = v;
        weight[edge_cnt] = w;
        nxt[edge_cnt] = head[u]; 
        head[u] = edge_cnt;      
    }
};

/*
 * 遍历示例：
 * void traverse(int u, const ForwardStar& graph)
 * {
    * for (int i = graph.head[u]; i != 0; i = graph.nxt[i])
    * {
        * int v = graph.to[i];
        * int w = graph.weight[i];
    * }
 * }
 */

// 去重
/*
    int n, m; cin >> n >> m;
    vector<PII> edges;
    for (int i = 1; i <= m; i++)
    {
        int u, v; cin >> u >> v;
        if (u > v) swap(u, v);
        if (u != v) edges.push_back({u, v});
    }
    sort(edges.begin(), edges.end());
    edges.erase(unique(edges.begin(), edges.end()), edges.end());
    EBCC ebcc{n, (int)edges.size() * 2};
    for (auto& e : edges)
    {
        ebcc.add_edge(e.first, e.second);
        ebcc.add_edge(e.second, e.first);
    }
*/