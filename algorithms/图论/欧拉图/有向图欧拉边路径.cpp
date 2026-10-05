// zoi: eulerDirected
#ifndef Z_OI_EULER_DIR
#define Z_OI_EULER_DIR

#include "../../杂项/utils/utils.cpp"

using namespace std;
using VPII = vector<PII>;

// edges[u] 存 {邻居, 边ID}; 先分配邻接表, 自行判存在性并选起点
// dfs 后 ans 逆序为边序列, 邻接表被清空
vector<VPII> edges;
VI ans;

// Hierholzer, 时空 O(n+m), 递归深度最坏 m
void dfs(int u)
{
    while (edges[u].size())
    {
        int v = edges[u].back().first;
        int id = edges[u].back().second;
        edges[u].pop_back();
        dfs(v);
        ans.push_back(id);
    }
}
#endif

// 要字典序则邻接按邻居降序排序
