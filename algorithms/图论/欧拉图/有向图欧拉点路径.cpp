// zoi: eulerPointDirected
#include <vector>

using namespace std;
using VI = vector<int>;

// 组装: edges[u] 存邻居; 1-based 加边后 dfs(起点), ans 逆序即欧拉路径点序列
vector<VI> edges;
VI ans;

// 回溯式剥边求有向图欧拉路径点序列 (Hierholzer)
// 时间: O(n + m) | 空间: O(n + m)
void dfs(int u)
{
    while (edges[u].size())
    {
        int v = edges[u].back();
        edges[u].pop_back();
        dfs(v);
    }
    ans.push_back(u);
}

/* 要字典序: 每个 edges[u] 按邻居编号从大到小排序; ans 逆序输出 */