// ==========================================
// Tarjan 求强连通分量 (SCC)
// 时间复杂度：O(N + M)
// 空间复杂度：O(N + M)
// ==========================================
struct TarjanSCC
{
    int n;
    int idx;
    int scc_cnt;
    
    vector<vector<int>> edges;
    vector<int> dfn;
    vector<int> low;
    vector<int> belong; // belong[i] 表示节点 i 所属的 SCC 编号 (1 ~ scc_cnt)
    vector<vector<int>> scc_nodes; // scc_nodes[i] 存储第 i 个 SCC 中的所有节点
    stack<int> sta;

    // 构造函数，传入节点总数进行初始化
    TarjanSCC(int _n)
    {
        n = _n;
        idx = 0;
        scc_cnt = 0;
        edges.assign(n + 1, vector<int>());
        dfn.assign(n + 1, 0);
        low.assign(n + 1, 0);
        belong.assign(n + 1, 0);
        scc_nodes.clear();
        scc_nodes.push_back({}); // 占位，使得 SCC 编号从 1 开始
        while (!sta.empty())
        {
            sta.pop();
        }
    }

    // 添加有向边 u -> v
    void add_edge(int u, int v)
    {
        edges[u].push_back(v);
    }

    // 核心 Tarjan DFS 逻辑
    void tarjan(int x)
    {
        dfn[x] = low[x] = ++idx;
        sta.push(x);

        for (int y : edges[x])
        {
            if (!dfn[y])
            {
                tarjan(y);
                low[x] = min(low[x], low[y]);
            }
            else if (!belong[y])
            {
                low[x] = min(low[x], dfn[y]);
            }
        }

        // 发现强连通分量
        if (low[x] == dfn[x])
        {
            scc_cnt++;
            scc_nodes.push_back(vector<int>());
            
            int t;
            do
            {
                t = sta.top();
                sta.pop();
                belong[t] = scc_cnt;
                scc_nodes[scc_cnt].push_back(t);
            } while (t != x);

            // 如果需要 SCC 内部节点有序，可保留此排序
            // sort(scc_nodes[scc_cnt].begin(), scc_nodes[scc_cnt].end());
        }
    }

    // 外部调用接口，处理整个图
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
};