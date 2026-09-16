// 共享图多组映射与跨结构连边, 独立展开/Floyd/原问题穷举, 默认含大图复用
#include "../建图上下文.cpp"
#include "../前缀优化建图.cpp"
#include "../线段树优化建图.cpp"
#include "../树上倍增优化建图.cpp"
#include "../../最短路问题/单源最短路径/dijkstra_heap.cpp"
#include "../../连通性相关/Tarjan_SCC.cpp"

namespace graphbuilder_check
{
constexpr LL BIG = 4000000000000000000LL;
struct Forest
{
    VI dep, parent;
    VVI fa, adj;
    Forest(int n, mt19937& rng) : dep(n + 1, 1), parent(n + 1),
        fa(20, VI(n + 1)), adj(n + 1)
    {
        for (int u = 2; u <= n; u++)
        {
            int p = (int)(rng() % u);
            parent[u] = fa[0][u] = p;
            if (p) dep[u] = dep[p] + 1, adj[u].push_back(p), adj[p].push_back(u);
        }
        for (int k = 1; k < 20; k++)
            for (int u = 1; u <= n; u++) fa[k][u] = fa[k - 1][fa[k - 1][u]];
    }
    int lca(int u, int v)
    {
        while (dep[u] > dep[v]) u = parent[u];
        while (dep[v] > dep[u]) v = parent[v];
        while (u != v) u = parent[u], v = parent[v];
        return u ? u : -1;
    }
    VI path(int u, int v) // 独立 BFS, 不使用 lca/fa/dep
    {
        VI prev(adj.size(), -1), q{u};
        prev[u] = 0;
        for (size_t i = 0; i < q.size(); i++)
            for (int x : adj[q[i]]) if (prev[x] == -1) prev[x] = q[i], q.push_back(x);
        if (prev[v] == -1) return {};
        VI res;
        for (int x = v; x; x = prev[x]) res.push_back(x);
        return res;
    }
};

template <class W>
void compare(GraphBuilder<W>& b, const VI& ids, VVLL d)
{
    int n = (int)ids.size() - 1;
    for (int k = 1; k <= n; k++)
        for (int i = 1; i <= n; i++) if (d[i][k] != BIG)
            for (int j = 1; j <= n; j++) if (d[k][j] != BIG) d[i][j] = min(d[i][j], d[i][k] + d[k][j]);
    if constexpr (is_same_v<W, Empty>)
    {
        SCC scc(b.tot);
        scc.build(b.g, b.tot);
        for (int s = 1; s <= n; s++)
        {
            VI seen(b.tot + 1), q{ids[s]};
            seen[ids[s]] = 1;
            for (size_t at = 0; at < q.size(); at++)
                for (auto& e : b.g[q[at]]) if (!seen[e.v]) seen[e.v] = 1, q.push_back(e.v);
            for (int v = 1; v <= n; v++)
            {
                assert(bool(seen[ids[v]]) == (d[s][v] != BIG));
                assert((scc.bel[ids[s]] == scc.bel[ids[v]]) == (d[s][v] != BIG && d[v][s] != BIG));
            }
        }
    }
    else
    {
        Dijkstra dij(b.tot);
        for (int s = 1; s <= n; s++)
        {
            dij.run(ids[s], b.g, b.tot);
            for (int v = 1; v <= n; v++) assert(dij.dist[ids[v]] == (d[s][v] == BIG ? INF : d[s][v]));
        }
    }
}

template <class W>
void random_mixed()
{
    mt19937 rng(6378);
    GraphBuilder<W> b(2048, 0);
    for (int tc = 0; tc < 400; tc++)
    {
        constexpr int N = 12;
        b.init(N);
        int w = 1 + (int)(rng() % 9);
        VI src(w + 1), dst(w + 1);
        for (int i = 1; i <= w; i++) src[i] = 1 + (int)(rng() % N), dst[i] = 1 + (int)(rng() % N);
        auto pi = prefix_in(b, src), po = prefix_out(b, dst);
        auto si = suffix_in(b, src), so = suffix_out(b, dst);
        SegLinks<W> seg(b);
        seg.build(src, dst);
        Forest forest(w, rng);
        TreeLinks<W> tree(b);
        tree.build(forest, src, dst);
        SegLinks<W, true, false> only_in(b);
        SegLinks<W, false, true> only_out(b);
        only_in.build(src); only_out.build(dst);
        TreeLinks<W, true, false> ti(b);
        TreeLinks<W, false, true> to(b);
        ti.build(forest, src); to.build(forest, dst);
        VI ids(N + 1);
        iota(ids.begin(), ids.end(), 0);
        VVLL d(40, VLL(40, BIG));
        for (int i = 1; i < 40; i++) d[i][i] = 0;
        compare(b, ids, d); // 任意映射和多组骨架也不应自动连通不同业务点
        auto part = [&](const VI& a, int l, int r)
        {
            VI res;
            for (int i = l; i <= r; i++) res.push_back(a[i]);
            return res;
        };
        auto path = [&](const VI& a, int u, int v)
        {
            VI res;
            for (int i : forest.path(u, v)) res.push_back(a[i]);
            return res;
        };
        for (int op = 0; op < 32; op++)
        {
            int r = (int)(rng() % (w + 1)), l = 1 + (int)(rng() % (w + 1));
            int u = 1 + (int)(rng() % w), v = 1 + (int)(rng() % w);
            VI from, into, left, right;
            switch (op % 8)
            {
                case 0: from = {pi[r]}; into = {so[l]}; left = part(src, 1, r); right = part(dst, l, w); break;
                case 1: from = {si[l]}; into = {po[r]}; left = part(src, l, w); right = part(dst, 1, r); break;
                case 2: from = seg.in_cover(l, r); into = {po[r]}; left = part(src, l, r); right = part(dst, 1, r); break;
                case 3:
                {
                    auto cover = tree.in_cover(u, v, forest);
                    from.assign(cover.begin(), cover.end()); into = seg.out_cover(l, r);
                    left = path(src, u, v); right = part(dst, l, r); break;
                }
                case 4:
                {
                    auto cover = to.out_cover(u, v, forest);
                    from = only_in.in_cover(l, r); into.assign(cover.begin(), cover.end());
                    left = part(src, l, r); right = path(dst, u, v); break;
                }
                case 5:
                {
                    auto cover = ti.in_cover(u, v, forest);
                    from.assign(cover.begin(), cover.end()); into = only_out.out_cover(l, r);
                    left = path(src, u, v); right = part(dst, l, r); break;
                }
                case 6:
                {
                    int p = b.new_node();
                    ids.push_back(p);
                    from = {src[u]}; into = {p}; left = {src[u]}; right = {(int)ids.size() - 1}; break;
                }
                case 7:
                    from = {ids.back()}; into = {dst[v]}; left = {(int)ids.size() - 1}; right = {dst[v]}; break;
            }
            LL weight = (int)(rng() % 21);
            if constexpr (is_same_v<W, Empty>) weight = 0;
            if constexpr (is_same_v<W, LL>) if (op % 7 == 0) weight += 1000000000000LL;
            W value{};
            if constexpr (!is_same_v<W, Empty>) value = (W)weight;
            int oldtot = b.tot, oldedges = b.g.edge_cnt();
            b.link(from, into, value);
            if (left.empty() || right.empty()) assert(b.tot == oldtot && b.g.edge_cnt() == oldedges);
            int a = 0, c = 0;
            for (int x : from) a += x != 0;
            for (int x : into) c += x != 0;
            assert(b.tot == oldtot + (a > 1 && c > 1));
            for (int x : left) for (int y : right) d[x][y] = min(d[x][y], weight);
            if (tc < 30) compare(b, ids, d);
        }
        compare(b, ids, d);
    }
}

void riddle()
{
    mt19937 rng(3513);
    for (int tc = 0; tc < 1000; tc++)
    {
        int n = 1 + (int)(rng() % 8);
        VI order(n); iota(order.begin(), order.end(), 1);
        shuffle(order.begin(), order.end(), rng);
        VVI groups(1);
        for (int u : order)
        {
            if (!groups.back().empty() && rng() % 3 == 0) groups.push_back({});
            groups.back().push_back(u);
        }
        GraphBuilder<Empty> b(4 * n, 0);
        b.init(2 * n);
        VPII edges;
        for (int u = 1; u <= n; u++) for (int v = u; v <= n; v++) if (rng() % 4 == 0)
        {
            edges.push_back({u, v}); b.add(u + n, v); b.add(v + n, u);
        }
        for (auto& group : groups)
        {
            VI neg{0};
            for (int u : group) neg.push_back(u + n);
            auto pre = prefix_out(b, neg), suf = suffix_out(b, neg);
            int w = (int)group.size();
            for (int i = 1; i <= w; i++) b.add(group[i - 1], pre[i - 1]), b.add(group[i - 1], suf[i + 1]);
        }
        SCC scc(b.tot); scc.build(b.g, b.tot);
        bool got = true, brute = false;
        for (int u = 1; u <= n; u++) if (scc.bel[u] == scc.bel[u + n]) got = false;
        for (int mask = 0; mask < (1 << n); mask++)
        {
            bool ok = true;
            for (auto& group : groups)
            {
                int count = 0;
                for (int u : group) count += (mask >> (u - 1)) & 1;
                if (count != 1) ok = false;
            }
            for (auto [u, v] : edges) if (!(mask >> (u - 1) & 1) && !(mask >> (v - 1) & 1)) ok = false;
            brute |= ok;
        }
        assert(got == brute);
    }
}

void boundary()
{
    GraphBuilder<LL> tight(5, 0);
    tight.init(4);
    tight.link(VI{0, 1, 0}, VI{2, 0, 3, 4}, 9);
    assert(tight.tot == 4 && tight.g.edge_cnt() == 3);
    Dijkstra dij(5);
    dij.run(1, tight.g, tight.tot);
    for (int v = 2; v <= 4; v++) assert(dij.dist[v] == 9);
    tight.init(4);
    tight.link(VI{1, 0, 2, 3}, VI{0, 4}, 9);
    assert(tight.tot == 4 && tight.g.edge_cnt() == 3);
    for (int u = 1; u <= 3; u++)
    {
        dij.run(u, tight.g, tight.tot);
        assert(dij.dist[4] == 9);
    }
    tight.init(4);
    tight.link(VI{1, 2}, VI{3, 4}, 9);
    assert(tight.tot == 5 && tight.g.edge_cnt() == 4);
    for (int u = 1; u <= 2; u++)
    {
        dij.run(u, tight.g, tight.tot);
        assert(dij.dist[3] == 9 && dij.dist[4] == 9);
    }
    GraphBuilder<LL> b(10, 0); b.init(4);
    auto empty = prefix_out(b, VI{0});
    auto suffix = suffix_in(b, VI{0});
    assert(empty[0] == 0 && suffix[1] == 0 && b.tot == 4);
    b.link(VI{0}, VI{1, 2}, 3);
    b.link(VI{1, 2}, VI{0}, 3);
    assert(b.tot == 4 && b.g.edge_cnt() == 0);
    b.link(VI{1}, VI{2}, -7); // 只有一条逻辑边, 无中继, 允许负权但不运行 Dijkstra
    assert(b.tot == 4 && b.g.edges.back().w == -7);
    for (int i = 0; i < 6; i++) b.new_node();
    assert(b.tot == 10);
    b.init(1); assert(b.tot == 1 && b.g.edge_cnt() == 0);
    SegLinks<LL> seg(b); seg.build(VI{0});
    assert(seg.in_cover(1, 0).empty() && seg.out_cover(1, 0).empty());
    b.init(4); seg.build(VI{0, 4, 2, 1});
    int before = b.g.edge_cnt();
    seg.build(VI{0, 3}); // 只重建索引, 旧图仍保留
    assert(b.g.edge_cnt() == before && seg.out_cover(1, 1) == VI{3});
}

void stress()
{
    constexpr int N = 200000, Q = 1000000;
    GraphBuilder<int> b(5 * N, 8 * N + Q);
    for (int n : {N, 1, 257, N})
    {
        b.init(2 * n);
        VI pos(n + 1), neg(n + 1);
        for (int i = 1; i <= n; i++) pos[i] = i, neg[i] = i + n;
        auto pre = prefix_out(b, neg), suf = suffix_out(b, neg);
        SegLinks<int, true, false> seg(b); seg.build(pos);
        b.add(1, pre[n], 7);
        for (int i = 0; i < Q; i++)
        {
            int r = i % n + 1;
            if (i % 2) b.add(1, suf[r], 7);
            else b.link(seg.in_cover(1, n), VI{pre[r]}, 7);
        }
        assert(b.tot == 5 * n - 3);
        assert(b.g.edge_cnt() == 6 * (n - 1) + Q + 1);
        Dijkstra dij(b.tot); dij.run(1, b.g, b.tot);
        for (int i = 1; i <= n; i++)
        {
            assert(dij.dist[n + i] == 7);
            assert(dij.dist[i] == (i == 1 ? 0 : INF));
        }
    }
}

void single_side_stress()
{
    constexpr int N = 200000, Q = 1000000;
    GraphBuilder<int> b(N, 2 * Q); // 全部点容量都是已有点, 无额外中继预算
    b.init(N);
    array<int, 1> one{1};
    array<int, 3> many{N / 2, 0, N};
    for (int i = 0; i < Q; i++)
        if (i & 1) b.link(many, one, 9);
        else b.link(one, many, 7);
    assert(b.tot == N && b.g.edge_cnt() == 2 * Q);
    Dijkstra dij(N); dij.run(1, b.g, b.tot);
    assert(dij.dist[N / 2] == 7 && dij.dist[N] == 7);
    for (int i = 2; i <= N; i++) if (i != N / 2 && i != N) assert(dij.dist[i] == INF);
}
}
int main()
{
    graphbuilder_check::random_mixed<int>();
    graphbuilder_check::random_mixed<LL>();
    graphbuilder_check::random_mixed<Empty>();
    graphbuilder_check::riddle();
    graphbuilder_check::boundary();
    graphbuilder_check::stress();
    graphbuilder_check::single_side_stress();
    cout << "graphbuilder_check PASS: 1200 mapped compositions, 1000 exact-one Riddle cases, 200000 variables / 1000000 links, zero-extra single-side stress" << endl;
}
