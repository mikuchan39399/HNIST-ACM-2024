// 完整重建不先 init; 离线输入、点权与共享版本必须保留
#include "../../杂项/utils/utils.cpp"
#include "../图的存储/Graph.cpp"
#include "../连通性相关/Tarjan_SCC.cpp"
#include "../连通性相关/Tarjan_EBCC.cpp"
#include "../连通性相关/Tarjan_VBCC.cpp"
#include "../连通性相关/圆方树/BCT.cpp"
#include "../../数据结构/线段树/泛型线段树.cpp"
#include "../../数据结构/线段树/泛型插件/区间加区间和.cpp"
#include "../../数据结构/线段树/可持久化线段树/主席树.cpp"
#include "../树上问题/树的重心/树的重心.cpp"
#include "../树上问题/最近公共祖先/DFN_LCA.cpp"
#include "../树上问题/最近公共祖先/HLD_LCA.cpp"
#include "../树上问题/树链剖分/HLD.cpp"
#include "../最短路问题/单源最短路径/dijkstra_heap.cpp"
#include "../最短路问题/单源最短路径/dijkstra.cpp"
#include "../最短路问题/单源最短路径/spfa 存图.cpp"
#include "../最短路问题/单源最短路径/Bellman-Ford存图.cpp"
#include "../最短路问题/判断负环/bf_-ring.cpp"
#include "../最短路问题/判断负环/spfa_-ring.cpp"
namespace BinaryLegacy {
#include "../树上问题/最近公共祖先/树上倍增LCA.cpp"
}
namespace EulerLegacy {
#include "../树上问题/最近公共祖先/欧拉序LCA.cpp"
}
namespace OfflineLegacy {
#include "../树上问题/最近公共祖先/tarjan.cpp"
}

template<class T> concept OldBuild = requires(T& t, Graph<false>& g) { t.build(g); };
template<class T> concept OldRootBuild = requires(T& t, Graph<false>& g) { t.build(g, 1); };
template<class T> concept NewRootBuild = requires(T& t, Graph<false>& g) { t.build(g, 1, -1); };
template<class T> concept OldRun = requires(T& t, Graph<true, LL>& g) { t.run(1, g); };
template<class T> concept NewRun = requires(T& t, Graph<true, LL>& g) { t.run(1, g, 1); };
template<class T> concept OldRing = requires(T& t, Graph<true, LL>& g) { t.run(g); };
static_assert(!OldBuild<LCA> && OldRootBuild<LCA>);
static_assert(!OldBuild<HLD> && !OldRootBuild<HLD> && NewRootBuild<HLD>);
static_assert(!OldBuild<HLD_LCA> && !OldRootBuild<HLD_LCA> && NewRootBuild<HLD_LCA>);
static_assert(!OldRun<Dijkstra> && !OldRun<DijkstraN> && !OldRun<SPFA> && !OldRun<BellmanFord>);
static_assert(NewRun<Dijkstra> && NewRun<DijkstraN> && NewRun<SPFA> && NewRun<BellmanFord>);
static_assert(!OldRing<BFRing> && !OldRing<SPFARing>);

int brute_lca(int u, int v, const VI& parent, const VI& dep)
{
    while (dep[u] > dep[v]) u = parent[u];
    while (dep[v] > dep[u]) v = parent[v];
    while (u != v) { u = parent[u]; v = parent[v]; }
    return u;
}

void legacy_lca()
{
    constexpr int CAP = 256;
    VI head(CAP + 1), to(2 * CAP + 1), nxt(2 * CAP + 1);
    BinaryLegacy::LCA binary(CAP, head, to, nxt);
    EulerLegacy::LCA euler(CAP, head, to, nxt);
    OfflineLegacy::TarjanLCA offline(CAP, 2 * CAP * CAP);
    mt19937 rng(42);
    for (int tc = 0; tc < 350; ++tc)
    {
        int n = tc < 4 ? (tc % 2 ? 1 : CAP) : 1 + rng() % 28;
        fill(head.begin(), head.end(), 0);
        int ec = 0;
        auto add = [&](int u, int v) { to[++ec] = v; nxt[ec] = head[u]; head[u] = ec; };
        VI parent(n + 1), dep(n + 1);
        vector<VI> adj(n + 1);
        dep[1] = 1;
        offline.init(n, 2 * n * n); // 此阶段丢弃旧输入, 之后重新加边与询问
        for (int v = 2; v <= n; ++v)
        {
            int p = 1 + rng() % (v - 1);
            parent[v] = p; dep[v] = dep[p] + 1;
            add(p, v); add(v, p);
            adj[p].push_back(v); adj[v].push_back(p);
            offline.add_edge(p, v); offline.add_edge(v, p);
        }
        for (int u = 1; u <= n; ++u) for (int v = 1; v <= n; ++v)
        {
            int id = (u - 1) * n + v;
            offline.add_query(u, v, id); offline.add_query(v, u, id);
        }
        for (int repeat = 0; repeat < 2; ++repeat)
        {
            binary.build(n); euler.build(n, 1); offline.build();
            assert(offline.edge_cnt == 2 * (n - 1) && offline.q_cnt == 2 * n * n);
            for (int u = 1; u <= n; ++u) for (int v = 1; v <= n; ++v)
            {
                int want = brute_lca(u, v, parent, dep);
                assert(binary.lca(u, v) == want && euler.lca(u, v) == want);
                assert(offline.ans[(u - 1) * n + v] == want);
            }
        }
        // 欧拉序旧接口明确根, 换根后用独立 BFS 父表核对
        int root = 1 + rng() % n;
        fill(parent.begin(), parent.end(), 0); fill(dep.begin(), dep.end(), 0);
        VI q{root}; dep[root] = 1;
        for (size_t i = 0; i < q.size(); ++i)
            for (int v : adj[q[i]]) if (!dep[v])
            { parent[v] = q[i]; dep[v] = dep[q[i]] + 1; q.push_back(v); }
        euler.build(n, root);
        for (int u = 1; u <= n; ++u) for (int v = 1; v <= n; ++v)
            assert(euler.lca(u, v) == brute_lca(u, v, parent, dep));
    }
}

void rebuilt_outputs()
{
    constexpr int CAP = 1000;
    Graph<true> dg(CAP);
    Graph<false> ug(CAP);
    SCC s(CAP); EBCC e(CAP); VBCC v(CAP); BCT b(CAP);
    for (int tc = 0; tc < 350; ++tc)
    {
        int n = tc < 4 ? (tc % 2 ? 1 : CAP) : 1 + tc % 31;
        dg.clear(); ug.clear();
        for (int u = 2; u <= n; ++u) { dg.add(1, u); dg.add(1, u); ug.add(1, u); }
        for (int repeat = 0; repeat < 2; ++repeat)
        {
            s.build(dg, n); e.build(ug, n); v.build(ug, n); b.build(ug, n);
            for (int j = 0; j < 2; ++j)
            {
                s.build_dag(dg); assert(s.dag.edge_cnt() == 2 * (n - 1));
                s.build_dag_unique(dg); assert(s.dag.edge_cnt() == n - 1);
                e.build_tree(ug); assert(e.tree.edge_cnt() == n - 1);
                v.build_tree(); b.build_tree();
                int edges = n == 1 ? 1 : 2 * (n - 1);
                assert(v.tree.edge_cnt() == edges && b.tree.edge_cnt() == edges);
            }
        }
        assert(dg.edge_cnt() == 2 * (n - 1) && ug.edge_cnt() == n - 1);
    }
}

void segment_rebuild()
{
    using Info = SegAdd::Info;
    using Tag = SegAdd::Tag;
    constexpr int CAP = 200000;
    SegTree<Info, Tag> seg(CAP);
    mt19937 rng(42);
    for (int tc = 0; tc < 350; ++tc)
    {
        int n = tc < 4 ? (tc % 2 ? 1 : CAP) : 1 + rng() % 80;
        vector<Info> a(n + 1);
        for (int i = 1; i <= n; ++i) a[i] = Info(i);
        seg.build(a); // 上一轮留着全域懒标记, 这里不调用 init
        assert(seg.n == n && seg.query(1, n).sum == 1LL * n * (n + 1) / 2);
        assert(seg.query(n, n).sum == n && seg.query(1, 1).sum == 1);
        seg.modify(1, n, Tag{100});
    }
    seg.build(vector<Info>(1)); assert(seg.n == 0 && seg.find_first(1, [](auto){return true;}) == -1);
    seg.build(vector<Info>{Info(), Info(7)}); assert(seg.query(1, 1).sum == 7);
    // 同池同值域新建另一个根, 历史根和分支不能被重建清除
    PersSegTree<Info, Tag> p(3, 100);
    int a = p.build(vector<Info>{Info(), Info(1), Info(2), Info(3)});
    int branch = p.modify(a, 1, 3, Tag{10});
    int other = p.build(vector<Info>{Info(), Info(4), Info(5), Info(6)});
    assert(p.query(a, 1, 3).sum == 6 && p.query(branch, 1, 3).sum == 36 && p.query(other, 1, 3).sum == 15);
    // 点权是输入, build 必须保留它, init 才把点权恢复成 1
    Graph<false> g(3); g.add(1, 2); g.add(2, 3);
    TreeCentroid<Graph<false>> c(3); c.pt[1] = 100;
    c.build(g, 3); assert(c.centroids == VI{1});
    c.build(g, 3); assert(c.centroids == VI{1} && c.pt[1] == 100);
    c.init(3); c.build(g, 3); assert(c.centroids == VI{2});
}

int main()
{
    legacy_lca(); rebuilt_outputs(); segment_rebuild();
    cout << "lifecycle_check passed: 350 rounds per group, repeated builds, inputs and versions preserved\n";
}
