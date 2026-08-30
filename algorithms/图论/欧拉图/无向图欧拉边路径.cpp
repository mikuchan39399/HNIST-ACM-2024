// zoi: eulerUndirected
#include <vector>
#include <utility>

using namespace std;
using VI = vector<int>;
using VPII = vector<pair<int, int>>;

// 组装: edges[u] 存 {邻居, 边ID}; visit[id] 标记边已用; 1-based 加边后
//       dfs(起点), ans 逆序即无向图欧拉路径边序列
vector<VPII> edges;
vector<bool> visit;
VI ans;

// 回溯式剥边求欧拉路径边序列 (Hierholzer)
// 时间: O(n + m) | 空间: O(n + m)
void dfs(int u)
{
    while (edges[u].size())
    {
        int v = edges[u].back().first;
        int id = edges[u].back().second;
        edges[u].pop_back();
        if (visit[id]) continue;
        visit[id] = true;
        dfs(v);
        ans.push_back(id);
    }
}

/* 要字典序: 每个 edges[u] 按邻居编号从大到小排序; ans 逆序输出 */