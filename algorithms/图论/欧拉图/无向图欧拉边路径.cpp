// zoi: eulerUndirected
#ifndef Z_OI_EULER_UND
#define Z_OI_EULER_UND

#include "../../../杂项/utils/utils.cpp"

using namespace std;
using VPII = vector<PII>;

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
#endif

/* 要字典序: 每个 edges[u] 按邻居编号从大到小排序; ans 逆序输出 */