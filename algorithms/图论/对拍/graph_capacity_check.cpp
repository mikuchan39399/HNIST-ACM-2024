#include "../图的存储/Graph.cpp"
#include "../拓扑排序/拓扑排序.cpp"
#include "../连通性相关/Tarjan_SCC.cpp"
#include "../连通性相关/Tarjan_EBCC.cpp"
#include "../连通性相关/Tarjan_VBCC.cpp"
#include "../连通性相关/圆方树/BCT.cpp"
#include "../树上问题/最近公共祖先/DFN_LCA.cpp"
#include "../树上问题/最近公共祖先/HLD_LCA.cpp"
#include "../树上问题/树链剖分/HLD.cpp"
#include "../树上问题/虚树/二次排序.cpp"
#include "../树上问题/虚树/单调栈.cpp"
#include "../树上问题/树的直径/两次dfs.cpp"
#include "../树上问题/树的直径/树形dp法.cpp"
#include "../树上问题/树的中心/树的中心.cpp"
#include "../树上问题/树的重心/树的重心.cpp"
#include "../最短路问题/单源最短路径/dijkstra_heap.cpp"
#include "../最短路问题/单源最短路径/dijkstra.cpp"
#include "../最短路问题/单源最短路径/spfa 存图.cpp"
#include "../最短路问题/单源最短路径/Bellman-Ford存图.cpp"
#include "../最短路问题/判断负环/spfa_-ring.cpp"
#include "../最短路问题/判断负环/bf_-ring.cpp"
#include "../生成树问题/最小生成树/kruskal/Kruskal.cpp"
#include "../生成树问题/最小生成树/prim/Prim.cpp"
#include "../优化建图/线段树优化建图.cpp"
#include "../优化建图/树上倍增优化建图.cpp"

// 每个对象只构造一次, 模拟全局 20 万点容量、连续 1000 组小测。
// 所有外置 Graph 均省略边预留; 大编号触碰后 clear, 再按本轮 n 重建。
constexpr int CAP = 200000, ROUNDS = 1000, MARK = 123456789;
int points(int tc) { return 1 + (tc * 17) % 33; }

template<class G>
void star(G& g, int n)
{
    g.clear();
    for (int u = 2; u <= n; u++) g.add(1, u, 1);
}

void paths_and_mst()
{
    Graph<false, LL> g(CAP);
    const size_t initial = g.edges.capacity();
    g.add(CAP - 1, CAP, 5);
    g.clear();
    assert(g.head[CAP] == -1 && g.deg[CAP] == 0 && g.used.empty());
    Dijkstra heap(CAP); DijkstraN dense(CAP); SPFA spfa(CAP); BellmanFord bf(CAP);
    SPFARing sr(CAP); BFRing br(CAP); Kruskal kr; Prim pr;
    heap.dist[CAP] = dense.dist[CAP] = spfa.dist[CAP] = bf.dist[CAP] = MARK;
    sr.dist[CAP] = br.dist[CAP] = MARK;
    for (int tc = 0; tc < ROUNDS; tc++)
    {
        int n = points(tc);
        star(g, n);

        heap.run(1, g, n); dense.run(1, g, n); spfa.run(1, g, n); bf.run(1, g, n);
        for (int u = 1; u <= n; u++)
        {
            LL want = u == 1 ? 0 : 1;
            assert(heap.dist[u] == want && dense.dist[u] == want);
            assert(spfa.dist[u] == want && bf.dist[u] == want);
        }
        assert(!sr.run(g, n) && !br.run(g, n));
        assert(kr.build(g, n) && pr.build(g, n));
        assert(kr.weight == n - 1 && pr.weight == n - 1);
        g.add(1, 1, -1);
        assert(sr.run(g, n) && br.run(g, n));
        assert(heap.dist[CAP] == MARK && dense.dist[CAP] == MARK);
        assert(spfa.dist[CAP] == MARK && bf.dist[CAP] == MARK);
        assert(sr.dist[CAP] == MARK && br.dist[CAP] == MARK);
    }
    assert(g.edges.capacity() > initial);
}

void connectivity()
{
    Graph<true> dg(CAP);
    Graph<false, LL> ug(CAP);
    SCC s(CAP); EBCC e(CAP); VBCC v(CAP); BCT b(CAP); TopoSort topo;
    s.dfn[CAP] = e.dfn[CAP] = v.dfn[CAP] = b.dfn[CAP] = MARK;
    for (int tc = 0; tc < ROUNDS; tc++)
    {
        int n = points(tc);
        dg.clear(); star(ug, n);
        for (int u = 2; u <= n; u++)
        {
            dg.add(1, u);
            if (tc % 2) dg.add(u, 1);
        }
        s.build(dg, n); s.build_dag_unique(dg);
        assert(s.scc_cnt == (tc % 2 ? 1 : n));
        assert(topo.build(dg, n) == (tc % 2 == 0 || n == 1));
        assert(topo.in.size() == (size_t)n + 1);
        e.build(ug, n); e.build_tree(ug);
        assert(e.ebcc_cnt == n && e.tree.edge_cnt() == n - 1);
        assert((int)e.get_bridges(ug).size() == n - 1);
        v.build(ug, n); v.build_tree();
        b.build(ug, n); b.build_tree();
        assert(v.vbcc_cnt == max(1, n - 1) && b.vbcc_cnt == v.vbcc_cnt);
        assert(v.cut[1] == (n > 2) && b.cut[1] == (n > 2));
        assert(v.tree.edge_cnt() == (n == 1 ? 1 : 2 * (n - 1)));
        assert(b.tree.edge_cnt() == v.tree.edge_cnt());
        assert(s.dfn[CAP] == MARK && e.dfn[CAP] == MARK);
        assert(v.dfn[CAP] == MARK && b.dfn[CAP] == MARK);
    }
}

void trees()
{
    Graph<false, LL> g(CAP);
    LCA l(CAP); HLD_LCA hl(CAP); HLD h(CAP);
    VirtualTree vt(CAP); VirtualTreeStack vs(CAP);
    TreeDiameter<decltype(g)> d; TreeDiameterDP<decltype(g)> dp;
    TreeCenter<decltype(g)> c; TreeCentroid<decltype(g)> ct(CAP);
    l.dep[CAP] = hl.dep[CAP] = h.dep[CAP] = MARK;
    for (int tc = 0; tc < ROUNDS; tc++)
    {
        int n = points(tc);
        star(g, n);
        l.build(g, n); hl.build(g, n, -1); h.build(g, n, -1);
        for (int u = 1; u <= n; u++)
        {
            assert(l.lca(1, u) == 1 && hl.lca(1, u) == 1);
            assert(l.dist(1, u) == (u != 1) && hl.dist(1, u) == (u != 1));
            assert(h.fa[u] == (u == 1 ? 0 : 1));
        }
        VI chosen = tc % 3 ? VI{1, n, n} : VI{};
        vt.build(chosen, l); vs.build(chosen, l);
        assert(vt.tree.edge_cnt() == (chosen.empty() || n == 1 ? 0 : 1));
        assert(vs.tree.edge_cnt() == vt.tree.edge_cnt());
        d.build(g, n); LL want = min(n - 1, 2);
        assert(d.len == want && dp.build(g, n) == want);
        assert(c.build(g, n) == (n > 1));
        ct.init(n); ct.build(g, n);
        assert(ct.centroids == (n == 2 ? VI{1, 2} : VI{1}));
        assert(l.dep[CAP] == MARK && hl.dep[CAP] == MARK && h.dep[CAP] == MARK);
    }
}

void compressed_graphs()
{
    Graph<false, LL> tree(CAP);
    LCA l(CAP);
    SegGraph<Empty> sg(CAP, 0, 2);
    TreeGraph<Empty> tg(CAP, 0, 2);
    TopoSort topo;
    for (int tc = 0; tc < ROUNDS; tc++)
    {
        int n = points(tc);
        star(tree, n); l.build(tree, n);
        sg.build(n); tg.build(l, n);
        assert(sg.tot == 3 * n - 2);
        assert(tg.tot == 3 * n - 2); // 星形树只有长度 2 的非平凡倍增段
        assert(topo.build(sg.g, sg.tot) && topo.build(tg.g, tg.tot));
        int a = sg.add_p2new(1), b = tg.add_p2new(1);
        assert(a == 3 * n - 1 && b == a);
        if (n > 1)
        {
            sg.add_p2r(a, 2, n);
            tg.add_p2path(b, n, n, l);
        }
        assert(topo.build(sg.g, sg.tot) && topo.build(tg.g, tg.tot));
        assert(topo.in.size() == (size_t)tg.tot + 1);
    }
}

int main()
{
    paths_and_mst(); connectivity(); trees(); compressed_graphs();
    cout << "graph_capacity_check passed: 25 engines, 4 x 1000 small rounds, zero edge reserve\n";
}
