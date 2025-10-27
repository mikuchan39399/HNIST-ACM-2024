/**
 * n -- 图的顶点数
 * m -- 图的边数
 * N -- 顶点数上限
 * M -- 边数上限
 * edges[N] -- VPII邻接表,存储 {邻居, 边ID}
 * visit[M] -- 标记边是否被删
 * ans -- VI欧拉路径边序列
 */

void dfs(int u)
{
    while(edges[u].size())
    {
        int v = edges[u].back().first
        int id = edges[u].back().second;
        edges[u].pop_back();
        if(visit[id]) continue;
        visit[id] = true;  
        dfs(v);
        ans.push_back(id);
    }
}

/**
 * 如果想要字典序，则需要对edges[1~n]每个VI按编号从大到小排序
 * 序列答案需要逆序
 */