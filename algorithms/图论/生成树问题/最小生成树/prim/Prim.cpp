// zoi: prim
#ifndef Z_OI_PRIM
#define Z_OI_PRIM

#include "../../../图的存储/Graph.cpp"

// 逐点扫描选连向森林的最轻边, 适合稠密图; 接无向 Graph, 权值为 int/LL, n >= 1
// weight 为森林总权, components 为分量数, edges 存选中边的偶数半边编号
// 允许负权/重边/自环, 累加须在 LL 内; 工作表每点 16 B, 不含图与返回边
struct Prim
{
    LL weight = 0;
    int components = 0;
    VI edges;

    // 重建 1..n 的最小生成森林并保存结果, 返回是否连通, 不修改图
    // 时间 O(n^2 + m) | 额外空间 O(n), 图 clear 后旧边编号失效
    template <class G>
    bool build(G& g, int n)
    {
        weight = 0;
        components = 0;
        edges.clear();
        VLL dis(n + 1);
        VI from(n + 1, -1), vis(n + 1);
        for (int i = 1; i <= n; i++)
        {
            int t = 0;
            for (int j = 1; j <= n; j++)
                if (!vis[j] && (!t || (from[j] != -1 && (from[t] == -1 || dis[j] < dis[t])))) t = j;
            if (from[t] == -1) components++; // 当前分量已收完, 从未访问点开新分量
            else
            {
                weight += dis[t];
                edges.push_back(from[t]);
            }
            vis[t] = 1;
            for (auto& e : g[t])
                if (!vis[e.v] && (from[e.v] == -1 || e.w < dis[e.v]))
                {
                    dis[e.v] = e.w;
                    from[e.v] = g.id(e) & ~1;
                }
        }
        return components <= 1;
    }
};
#endif

/* Usage
int main()
{
    Graph<false, LL> g(4, 4);
    g.add(1, 2, -2); g.add(2, 3, 5); g.add(1, 3, 4); g.add(3, 4, 1);
    Prim mst;
    cout << mst.build(g, 4) << ' ' << mst.weight << '\n'; // 1 3
    Graph<false, LL> tree(4, 3);
    for (int id : mst.edges)
        tree.add(g.edges[id ^ 1].v, g.edges[id].v, g.edges[id].w);
    g.clear(); g.add(1, 2, -5);
    cout << mst.build(g, 4) << ' ' << mst.components << ' ' << mst.weight << '\n'; // 0 3 -5
}
*/
