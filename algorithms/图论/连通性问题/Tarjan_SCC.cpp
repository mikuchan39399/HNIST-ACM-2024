#include <vector>
#include <algorithm>
#include <iostream>

using namespace std;

using VI = vector<int>;
using VVI = vector<vector<int>>;

struct SCC
{
    int n;
    int dfn_idx;
    int scc_cnt;
    int edge_cnt;
    int dag_edge_cnt;

    VI head, to, nxt;
    VI dag_head, dag_to, dag_nxt;

    VI dfn;
    VI low;
    VI bel;
    VI in_stk;
    VI sta;

    SCC(int max_n, int max_m)
    {
        head.assign(max_n + 10, 0);
        to.assign(max_m + 10, 0);
        nxt.assign(max_m + 10, 0);

        dag_head.assign(max_n + 10, 0);
        dag_to.assign(max_m + 10, 0);
        dag_nxt.assign(max_m + 10, 0);

        dfn.assign(max_n + 10, 0);
        low.assign(max_n + 10, 0);
        bel.assign(max_n + 10, 0);
        in_stk.assign(max_n + 10, 0);

        sta.reserve(max_n + 10);
    }

    void init(int _n)
    {
        n = _n;
        dfn_idx = 0;
        scc_cnt = 0;
        edge_cnt = 0;
        dag_edge_cnt = 0;
        
        sta.clear();
        for (int i = 1; i <= n; i++)
        {
            head[i] = 0;
            dag_head[i] = 0;
            dfn[i] = 0;
            low[i] = 0;
            bel[i] = 0;
            in_stk[i] = 0;
        }
    }

    void add_edge(int u, int v)
    {
        edge_cnt++;
        to[edge_cnt] = v;
        nxt[edge_cnt] = head[u];
        head[u] = edge_cnt;
    }

    void add_dag_edge(int u, int v)
    {
        dag_edge_cnt++;
        dag_to[dag_edge_cnt] = v;
        dag_nxt[dag_edge_cnt] = dag_head[u];
        dag_head[u] = dag_edge_cnt;
    }

    void tarjan(int u)
    {
        dfn_idx++;
        low[u] = dfn[u] = dfn_idx;
        sta.push_back(u);
        in_stk[u] = 1;

        for (int i = head[u]; i; i = nxt[i])
        {
            int v = to[i];
            if (!dfn[v])
            {
                tarjan(v);
                low[u] = min(low[u], low[v]);
            }
            else if (in_stk[v])
            {
                low[u] = min(low[u], dfn[v]);
            }
        }

        if (low[u] == dfn[u])
        {
            scc_cnt++;
            int t;
            do
            {
                t = sta.back();
                sta.pop_back();
                in_stk[t] = 0;
                bel[t] = scc_cnt;
            } while (t != u);
        }
    }

    void build()
    {
        for (int i = 1; i <= n; i++)
        {
            if (!dfn[i])
            {
                tarjan(i);
            }
        }
    }

    void build_dag()
    {
        for (int u = 1; u <= n; u++)
        {
            for (int i = head[u]; i; i = nxt[i])
            {
                int v = to[i];
                if (bel[u] != bel[v])
                {
                    add_dag_edge(bel[u], bel[v]);
                }
            }
        }
    }
};

/*
const int MAXN = 500005;
const int MAXM = 1000005;

SCC graph(MAXN, MAXM);

// 用于存储新 DAG 每个节点的权值或大小
int scc_val[MAXN]; 
int in_degree[MAXN]; // 记录入度，用于拓扑排序

void solve()
{
    int n, m;
    cin >> n >> m;
    
    graph.init(n);
    
    // 假设原图点有权值
    VI val(n + 1);
    for (int i = 1; i <= n; i++)
    {
        cin >> val[i];
        scc_val[i] = 0; 
        in_degree[i] = 0;
    }
    
    for (int i = 1; i <= m; i++)
    {
        int u, v;
        cin >> u >> v;
        graph.add_edge(u, v);
    }
    
    // 1. 跑 Tarjan 找 SCC
    graph.build();
    
    // 2. 将原图权值累加到对应的 SCC 新节点上
    for (int i = 1; i <= n; i++)
    {
        scc_val[graph.bel[i]] += val[i];
    }
    
    // 3. 构建 DAG
    graph.build_dag();
    
    // 4. 统计入度（如果是跑拓扑排序等算法）
    for (int u = 1; u <= graph.scc_cnt; u++)
    {
        for (int i = graph.dag_head[u]; i; i = graph.dag_nxt[i])
        {
            int v = graph.dag_to[i];
            in_degree[v]++;
        }
    }
    
    // 后续可以在 DAG 上跑拓扑排序求最长路 / 记忆化搜索...
}
*/