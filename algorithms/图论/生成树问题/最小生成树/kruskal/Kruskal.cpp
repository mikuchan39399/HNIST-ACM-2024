// zoi: kruskal
#ifndef Z_OI_KRUSKAL
#define Z_OI_KRUSKAL

#include "../../../图的存储/Graph.cpp"
#include "../../../../数据结构/并查集/DSU.cpp"

// 无向图最小生成森林, n>=1, int/LL 权; 可负权/重边/自环, 总权须在 LL 内
// weight 为总权, components 为分量数, edges 为选中边的偶数半边号(图 clear 后失效)
struct Kruskal
{
    LL weight = 0;
    int components = 0;
    VI edges;

    // 重建结果, 连通返回 true. 时间 O(n+m log(m+1)+m alpha(n)), 额外空间 O(n+m)
    template <class G>
    bool build(G& g, int n)
    {
        weight = 0;
        components = n;
        edges.clear();
        DSU ds(n);
        VI order(g.edge_cnt());
        iota(order.begin(), order.end(), 0);
        for (int& id : order) id *= 2;
        sort(order.begin(), order.end(), [&](int a, int b)
        {
            if (g.edges[a].w != g.edges[b].w) return g.edges[a].w < g.edges[b].w;
            return a < b;
        });
        for (int id : order)
        {
            int u = ds.find(g.edges[id ^ 1].v), v = ds.find(g.edges[id].v);
            if (u == v) continue;
            if (ds.sz[u] < ds.sz[v]) swap(u, v);
            ds.merge(u, v);
            weight += g.edges[id].w;
            edges.push_back(id);
            if (--components == 1) break;
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
    Kruskal mst;
    cout << mst.build(g, 4) << ' ' << mst.weight << '\n'; // 1 3
    Graph<false, LL> tree(4, 3);
    for (int id : mst.edges)
        tree.add(g.edges[id ^ 1].v, g.edges[id].v, g.edges[id].w);
    g.clear(); g.add(1, 2, -5);
    cout << mst.build(g, 4) << ' ' << mst.components << ' ' << mst.weight << '\n'; // 0 3 -5
}
*/
