#include "../树上倍增优化建图.cpp"
#include "../../树上问题/最近公共祖先/DFN_LCA.cpp"
#include "../../最短路问题/单源最短路径/dijkstra_heap.cpp"

struct Oracle
{
    vector<VI> tree;
    VVLL edges;
    Oracle(int n, int cap) : tree(n + 1), edges(cap + 1, VLL(cap + 1, INF)) {}
    VI path(int s, int t)
    {
        VI parent(tree.size(), -1), que{s};
        parent[s] = 0;
        for (int i = 0; i < (int)que.size(); i++)
            for (int v : tree[que[i]])
                if (parent[v] == -1) { parent[v] = que[i]; que.push_back(v); }
        if (parent[t] == -1) return {};
        VI res;
        for (int u = t; u; u = parent[u]) res.push_back(u);
        return res;
    }
    void add(int u, int v, LL w) { edges[u][v] = min(edges[u][v], w); }
};

template <class W>
void verify(TreeGraph<W>& tg, Oracle& ref, const VI& ids)
{
    int n = (int)ids.size() - 1;
    VVLL d = ref.edges;
    for (int i = 1; i <= n; i++) d[i][i] = 0;
    for (int k = 1; k <= n; k++)
        for (int i = 1; i <= n; i++)
            for (int j = 1; j <= n; j++)
                if (d[i][k] != INF && d[k][j] != INF)
                    d[i][j] = min(d[i][j], d[i][k] + d[k][j]);
    for (int s = 1; s <= n; s++)
    {
        if constexpr (is_same_v<W, Empty>)
        {
            VI seen(tg.tot + 1), que{ids[s]};
            seen[ids[s]] = 1;
            for (int i = 0; i < (int)que.size(); i++)
                for (auto& e : tg.g[que[i]])
                    if (!seen[e.v]) { seen[e.v] = 1; que.push_back(e.v); }
            for (int v = 1; v <= n; v++) assert(seen[ids[v]] == (d[s][v] != INF));
        }
        else
        {
            Dijkstra dij(tg.tot);

            dij.run(ids[s], tg.g, tg.tot);
            for (int v = 1; v <= n; v++) assert(dij.dist[ids[v]] == d[s][v]);
        }
    }
    for (int u = 1; u <= tg.tot; u++)
        for (auto& e : tg.g[u]) assert(e.v >= 1 && e.v <= tg.tot);
}

template <class W>
void random_cases()
{
    mt19937 rng(42);
    TreeGraph<W> tg(18, 0, 65);
    LCA lca(18);
    Graph<false, int> forest(18, 18);
    for (int round = 0; round < 400; round++)
    {
        int n = 1 + rng() % 18;
        forest.clear();
        Oracle ref(n, n + 65);
        for (int u = 2; u <= n; u++)
            if (rng() % 4)
            {
                int v = 1 + rng() % (u - 1);
                forest.add(u, v, 1);
                ref.tree[u].push_back(v); ref.tree[v].push_back(u);
            }
        lca.build(forest, n);
        tg.build(lca, n);
        VI ids(n + 1); iota(ids.begin(), ids.end(), 0);
        verify(tg, ref, ids); // 树只定义路径, 不自动成为可走的边
        for (int op = 0; op < 65; op++)
        {
            int kind = op % 6;
            int a = 1 + rng() % n, b = 1 + rng() % n;
            int c = 1 + rng() % n, d = 1 + rng() % n;
            if (rng() % 5 == 0) b = a;
            if (rng() % 5 == 0) d = c;
            VI from = ref.path(a, b), to = ref.path(c, d);
            int u = 1 + rng() % (ids.size() - 1), v = 1 + rng() % (ids.size() - 1);
            LL w = rng() % 101;
            if constexpr (is_same_v<W, LL>) if (op % 7 == 0) w *= 10000000000LL;
            W weight{};
            if constexpr (!is_same_v<W, Empty>) weight = W(w);
            else w = 1;
            int old_nodes = tg.tot, old_edges = tg.g.edge_cnt();
            auto save = [&](int p)
            {
                assert(p == old_nodes + 1 && p == tg.tot);
                ids.push_back(p);
                return (int)ids.size() - 1;
            };
            if (kind == 0)
            {
                tg.add_p2p(ids[u], ids[v], weight); ref.add(u, v, w);
                assert(tg.g.edge_cnt() - old_edges == 1);
            }
            if (kind == 1)
            {
                bool ok = tg.add_p2path(ids[u], a, b, lca, weight);
                assert(ok == !from.empty());
                for (int x : from) ref.add(u, x, w);
            }
            if (kind == 2)
            {
                bool ok = tg.add_path2p(a, b, ids[v], lca, weight);
                assert(ok == !from.empty());
                for (int x : from) ref.add(x, v, w);
            }
            if (kind == 3)
            {
                int p = save(tg.add_p2new(ids[u], weight));
                ref.add(u, p, w);
            }
            if (kind == 4)
            {
                int p = tg.add_path2new(a, b, lca, weight);
                if (from.empty()) assert(p == -1 && tg.tot == old_nodes && tg.g.edge_cnt() == old_edges);
                else { int x = save(p); for (int y : from) ref.add(y, x, w); }
            }
            if (kind == 5)
            {
                int p = tg.add_path2path(a, b, c, d, lca, weight);
                if (from.empty() || to.empty())
                    assert(p == -1 && tg.tot == old_nodes && tg.g.edge_cnt() == old_edges);
                else
                {
                    int x = save(p);
                    for (int y : from) ref.add(y, x, 0);
                    for (int y : to) ref.add(x, y, w);
                }
            }
            assert(tg.g.edge_cnt() - old_edges <= (kind == 5 ? 8 : 4));
            if (kind <= 2) assert(tg.tot == old_nodes);
            if ((kind == 1 || kind == 2) && from.empty()) assert(tg.g.edge_cnt() == old_edges);
            if (op == 20 || op == 64) verify(tg, ref, ids);
        }
    }
}

// 深链的祖先和 LCA 有闭式答案, 独立于库内递归 DFS 的栈容量
struct ChainLCA
{
    VI dep;
    VVI fa;
    ChainLCA(int n) : dep(n + 1), fa(__lg(n) + 1, VI(n + 1))
    {
        for (int u = 1; u <= n; u++)
        {
            dep[u] = u;
            for (int k = 0; k <= __lg(n); k++) fa[k][u] = max(0, u - (1 << k));
        }
    }
    int lca(int u, int v) { return min(u, v); }
};

void deterministic()
{
    ChainLCA chain(40);
    TreeGraph<int> tg(40, 0, 3);
    for (int n : {40, 1, 2, 17, 40})
    {
        tg.build(chain, n);
        Oracle ref(n, n + 3);
        VI ids(n + 1); iota(ids.begin(), ids.end(), 0);
        for (int u = 2; u <= n; u++) { ref.tree[u].push_back(u-1); ref.tree[u-1].push_back(u); }
        int p = tg.add_path2new(n, 1, chain, 3); ids.push_back(p);
        for (int u = 1; u <= n; u++) ref.add(u, n + 1, 3);
        int q = tg.add_p2new(p, 4); ids.push_back(q); ref.add(n + 1, n + 2, 4);
        tg.add_p2path(q, n, 1, chain, 5);
        for (int u = 1; u <= n; u++) ref.add(n + 2, u, 5);
        p = tg.add_path2path(n, 1, n, 1, chain, 0); ids.push_back(p);
        for (int u = 1; u <= n; u++) { ref.add(u, n + 3, 0); ref.add(n + 3, u, 0); }
        verify(tg, ref, ids);
    }
    TreeGraph<LL> big(40, 0, 1);
    big.build(chain, 40);
    big.add_path2path(1, 7, 19, 40, chain, INF - 1);
    Dijkstra dij(big.tot); dij.run(1, big.g, big.tot);
    for (int u = 19; u <= 40; u++) assert(dij.dist[u] == INF - 1);
    assert(dij.dist[2] == INF && dij.dist[18] == INF);
    big.build(chain, 40);
    big.add_path2path(1, 7, 19, 40, chain, -9);
    VLL dis(big.tot + 1, INF); dis[1] = 0;
    for (int pass = 1; pass <= big.tot; pass++)
        for (int u = 1; u <= big.tot; u++)
            if (dis[u] != INF)
                for (auto& e : big.g[u]) dis[e.v] = min(dis[e.v], dis[u] + e.w);
    for (int u = 19; u <= 40; u++) assert(dis[u] == -9);
    assert(dis[2] == INF && dis[18] == INF);
}

void large_chain()
{
    const int n = 50000, q = 1000000;
    ChainLCA chain(n);
    TreeGraph<int> tg(n, 4 * n * __lg(n) + 8 * q, q);
    for (int size : {n, 1, 17, n})
    {
        tg.build(chain, size);
        if (size != n) continue;
        int base = tg.tot, before = tg.g.edge_cnt();
        for (int i = 1; i <= q; i++)
        {
            int p;
            if (i % 3 == 0) p = tg.add_path2path(size, 1, 1, size, chain, 7);
            else if (i % 3 == 1) p = tg.add_path2new(1, size, chain, 2);
            else p = tg.add_p2new(1, 3);
            assert(p == base + i);
        }
        assert(tg.g.edge_cnt() - before <= 8 * q);
        Dijkstra dij(tg.tot); dij.run(1, tg.g, tg.tot);
        for (int u = 2; u <= size; u++) assert(dij.dist[u] == 7);
        for (int i = 1; i <= q; i++) assert(dij.dist[base+i] == (i%3 == 0 ? 0 : i%3 == 1 ? 2 : 3));
    }
}

void large_forest()
{
    const int n = 200000;
    Graph<false, int> forest(n, n);
    LCA lca(n);
    TreeGraph<int> tg(n, 4 * n * __lg(n) + 8 * n, n);
    for (int mode = 0; mode < 3; mode++)
    {
        forest.clear();
        for (int u = 2; u <= n; u++)
            if (mode < 2) forest.add(u, mode == 0 ? 1 : u / 2, 1);
        lca.build(forest, n); tg.build(lca, n);
        int base = tg.tot;
        for (int u = 1; u <= n; u++)
        {
            int p = tg.add_path2path(1, 1, u, u, lca, 5);
            assert(p == base + u); // 两条路径可分属不同连通块
        }
        Dijkstra dij(tg.tot); dij.run(1, tg.g, tg.tot);
        for (int u = 2; u <= n; u++) assert(dij.dist[u] == 5);
        assert(dij.dist[1] == 0);
    }
    TreeGraph<Empty> empty(1, 2*n, n);
     forest.clear(); lca.build(forest, 1); empty.build(lca, 1);
    int p = 1;
    for (int i = 1; i <= n; i++) p = empty.add_p2new(p);
    assert(p == n+1 && empty.g.edge_cnt() == n);
    int u = 1, count = 0;
    while (u != p) { assert(empty.g.deg[u] == 1); u = empty.g.edges[empty.g.head[u]].v; count++; }
    assert(count == n);
}

int main()
{
    random_cases<int>(); random_cases<LL>(); random_cases<Empty>();
    deterministic();
    large_chain();
    large_forest();
    cout << "treegraph_check passed: 3 x 400 random forests, boundaries, 50000/1000000 chain, 200000 forests and relay capacity\n";
}
