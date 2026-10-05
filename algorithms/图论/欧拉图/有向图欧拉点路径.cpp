// zoi: eulerPointDirected
#include <vector>

using namespace std;
using VI = vector<int>;

// edges[u] 存邻居; 先分配邻接表, 自行判存在性并选起点
// dfs 后 ans 逆序为点序列, 邻接表被清空
vector<VI> edges;
VI ans;

// Hierholzer, 时空 O(n+m), 递归深度最坏 m
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

// 要字典序则邻接按邻居降序排序
