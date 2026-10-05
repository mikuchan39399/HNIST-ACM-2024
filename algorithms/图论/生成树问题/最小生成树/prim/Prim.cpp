// zoi: prim
#ifndef Z_OI_PRIM
#define Z_OI_PRIM

#include "../../../图的存储/Graph.cpp"

// 无向图最小生成森林, n>=1, int/LL 权; 可负权/重边/自环, 总权须在 LL 内
// weight 为总权, components 为分量数, edges 为选中边的偶数半边号(图 clear 后失效)
struct Prim
{
    LL weight = 0;
    int components = 0;
    VI edges;

    // 重建结果, 连通返回 true. 时间 O(n²+m), 额外空间 O(n)
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
            if (from[t] == -1) components++;
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
