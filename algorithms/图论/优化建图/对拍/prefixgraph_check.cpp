// 前后缀优化建图: 三种模式独立小图对拍、空段/中继/复位与 20 万点百万操作压力
#include "../前缀优化建图.cpp"
#include "../../最短路问题/单源最短路径/dijkstra_heap.cpp"
#include "../../连通性相关/Tarjan_SCC.cpp"

namespace prefixgraph_check
{
constexpr LL BIG = 4000000000000000000LL;

void floyd(VVLL& d)
{
    int n = (int)d.size() - 1;
    for (int k = 1; k <= n; k++)
        for (int i = 1; i <= n; i++) if (d[i][k] != BIG)
            for (int j = 1; j <= n; j++) if (d[k][j] != BIG)
                d[i][j] = min(d[i][j], d[i][k] + d[k][j]);
}

template <class W, bool P, bool S>
void compare(PrefixGraph<W, P, S>& pg, const VI& id, VVLL expected)
{
    int count = (int)id.size() - 1;
    expected.resize(count + 1);
    for (auto& row : expected) row.resize(count + 1);
    floyd(expected);
    if constexpr (is_same_v<W, Empty>)
    {
        SCC scc(pg.tot);
        scc.build(pg.g, pg.tot);
        for (int s = 1; s <= count; s++)
        {
            VI seen(pg.tot + 1), queue{id[s]};
            seen[id[s]] = 1;
            for (size_t at = 0; at < queue.size(); at++)
                for (auto& e : pg.g[queue[at]]) if (!seen[e.v])
                {
                    seen[e.v] = 1;
                    queue.push_back(e.v);
                }
            for (int v = 1; v <= count; v++)
            {
                assert(bool(seen[id[v]]) == (expected[s][v] != BIG));
                bool mutual = expected[s][v] != BIG && expected[v][s] != BIG;
                assert((scc.bel[id[s]] == scc.bel[id[v]]) == mutual);
            }
        }
    }
    else
    {
        Dijkstra dij(pg.tot);
        for (int s = 1; s <= count; s++)
        {
            dij.run(id[s], pg.g, pg.tot);
            for (int v = 1; v <= count; v++)
                assert(dij.dist[id[v]] == (expected[s][v] == BIG ? INF : expected[s][v]));
        }
    }
}

template <class W>
void small()
{
    mt19937 rng(42);
    PrefixGraph<W> pg(32, 0, 32);
    for (int tc = 0; tc < 400; tc++)
    {
        int n = 1 + (int)(rng() % 12);
        pg.build(n);
        assert(pg.tot == 3 * n - 2 && pg.g.edge_cnt() == 4 * n - 4);
        VI id(n + 1);
        iota(id.begin(), id.end(), 0);
        VVLL d(n + 33, VLL(n + 33, BIG));
        for (int i = 1; i < n + 33; i++) d[i][i] = 0;
        auto edge = [&](int u, int v, LL w) { d[u][v] = min(d[u][v], w); };
        if (tc < 12) compare(pg, id, d); // 骨架自身不能让不同原点互达
        for (int op = 0; op < 32; op++)
        {
            int count = (int)id.size() - 1;
            int u = 1 + (int)(rng() % count), v = 1 + (int)(rng() % count);
            int r = (int)(rng() % (n + 1)), r2 = (int)(rng() % (n + 1));
            LL w = (int)(rng() % 20);
            if constexpr (is_same_v<W, LL>) if (op % 9 == 0) w += 1000000000000LL;
            if constexpr (is_same_v<W, Empty>) w = 0;
            W weight{};
            if constexpr (!is_same_v<W, Empty>) weight = (W)w;
            int type = op % 6, before = pg.g.edge_cnt(), oldtot = pg.tot;
            if (type == 0)
            {
                pg.add_p2p(id[u], id[v], weight);
                edge(u, v, w);
                assert(pg.g.edge_cnt() == before + 1);
            }
            else if (type == 1)
            {
                pg.add_p2pre(id[u], r, weight);
                for (int j = 1; j <= r; j++) edge(u, j, w);
                assert(pg.g.edge_cnt() == before + (r != 0));
            }
            else if (type == 2)
            {
                pg.add_pre2p(r, id[v], weight);
                for (int i = 1; i <= r; i++) edge(i, v, w);
                assert(pg.g.edge_cnt() == before + (r != 0));
            }
            else if (type == 3)
            {
                pg.add_pre2pre(r, r2, weight);
                for (int i = 1; i <= r; i++)
                    for (int j = 1; j <= r2; j++) edge(i, j, w);
                assert(pg.g.edge_cnt() == before + (r != 0 && r2 != 0));
                assert(pg.tot == oldtot);
            }
            else if (type == 4)
            {
                int p = pg.add_p2new(id[u], weight);
                assert(p == oldtot + 1);
                id.push_back(p);
                edge(u, count + 1, w);
            }
            else
            {
                int p = pg.add_pre2new(r, weight);
                assert(p == oldtot + 1);
                id.push_back(p);
                for (int i = 1; i <= r; i++) edge(i, count + 1, w);
                assert(pg.g.edge_cnt() == before + (r != 0));
            }
        }
        compare(pg, id, d);
    }
    for (int n : {32, 1, 2, 17, 32})
    {
        pg.build(n);
        for (int i = 0; i < 32; i++)
        {
            int p = pg.add_pre2new(0);
            assert(p == 3 * n - 1 + i && pg.g.head[p] == -1);
        }
        assert(pg.g.edge_cnt() == 4 * n - 4);
    }
    PrefixGraph<W> noextra(1, 0, 0), default_extra(3);
    noextra.build(1);
    noextra.add_pre2pre(1, 1);
    default_extra.build(1);
    for (int i = 0; i < 3; i++) default_extra.add_p2new(1);
}

template <class T>
concept HasPrefix = requires(T g) { g.add_p2pre(1, 1); };
template <class T>
concept HasSuffix = requires(T g) { g.add_p2suf(1, 1); };
template <class T>
concept HasMixed = requires(T g) { g.add_pre2suf(1, 1); g.add_suf2pre(1, 1); };
static_assert(HasPrefix<PrefixGraph<>> && !HasSuffix<PrefixGraph<>> && !HasMixed<PrefixGraph<>>);
static_assert(!HasPrefix<SuffixGraph<>> && HasSuffix<SuffixGraph<>> && !HasMixed<SuffixGraph<>>);
static_assert(HasPrefix<PrefixSuffixGraph<>> && HasSuffix<PrefixSuffixGraph<>> && HasMixed<PrefixSuffixGraph<>>);

template <class W, bool Mixed>
void small_suffix()
{
    mt19937 rng(314159 + Mixed);
    PrefixGraph<W, Mixed, true> pg(32, 0, 48);
    constexpr int directions = 1 + Mixed;
    for (int tc = 0; tc < 400; tc++)
    {
        int n = 1 + (int)(rng() % 12);
        pg.build(n);
        assert(pg.tot == n + directions * 2 * (n - 1));
        assert(pg.g.edge_cnt() == directions * 4 * (n - 1));
        VI id(n + 1);
        iota(id.begin(), id.end(), 0);
        VVLL d(n + 49, VLL(n + 49, BIG));
        for (int i = 1; i < n + 49; i++) d[i][i] = 0;
        auto edge = [&](int u, int v, LL w) { d[u][v] = min(d[u][v], w); };
        if (tc < 12) compare(pg, id, d);
        for (int op = 0; op < 48; op++)
        {
            int count = (int)id.size() - 1;
            int u = 1 + (int)(rng() % count), v = 1 + (int)(rng() % count);
            int l = 1 + (int)(rng() % (n + 1)), l2 = 1 + (int)(rng() % (n + 1));
            int r = (int)(rng() % (n + 1)), r2 = (int)(rng() % (n + 1));
            LL w = rng() % 20;
            if constexpr (is_same_v<W, LL>) if (op % 9 == 0) w += 1000000000000LL;
            if constexpr (is_same_v<W, Empty>) w = 0;
            W weight{};
            if constexpr (!is_same_v<W, Empty>) weight = (W)w;
            int before = pg.g.edge_cnt(), oldtot = pg.tot;
            int type = op % (Mixed ? 12 : 6), added = 0, new_id = 0;
            if (type == 0)
            {
                pg.add_p2p(id[u], id[v], weight);
                edge(u, v, w);
                added = 1;
            }
            else if (type == 1)
            {
                pg.add_p2suf(id[u], l, weight);
                for (int j = l; j <= n; j++) edge(u, j, w);
                added = l <= n;
            }
            else if (type == 2)
            {
                pg.add_suf2p(l, id[v], weight);
                for (int i = l; i <= n; i++) edge(i, v, w);
                added = l <= n;
            }
            else if (type == 3)
            {
                pg.add_suf2suf(l, l2, weight);
                for (int i = l; i <= n; i++)
                    for (int j = l2; j <= n; j++) edge(i, j, w);
                added = l <= n && l2 <= n;
            }
            else if (type == 4)
            {
                new_id = pg.add_p2new(id[u], weight);
                edge(u, count + 1, w);
                added = 1;
            }
            else if (type == 5)
            {
                new_id = pg.add_suf2new(l, weight);
                for (int i = l; i <= n; i++) edge(i, count + 1, w);
                added = l <= n;
            }
            else if constexpr (Mixed)
            {
                if (type == 6)
                {
                    pg.add_pre2suf(r, l, weight);
                    for (int i = 1; i <= r; i++)
                        for (int j = l; j <= n; j++) edge(i, j, w);
                    added = r && l <= n;
                }
                else if (type == 7)
                {
                    pg.add_suf2pre(l, r, weight);
                    for (int i = l; i <= n; i++)
                        for (int j = 1; j <= r; j++) edge(i, j, w);
                    added = l <= n && r;
                }
                else if (type == 8)
                {
                    pg.add_p2pre(id[u], r, weight);
                    for (int j = 1; j <= r; j++) edge(u, j, w);
                    added = r != 0;
                }
                else if (type == 9)
                {
                    pg.add_pre2p(r, id[v], weight);
                    for (int i = 1; i <= r; i++) edge(i, v, w);
                    added = r != 0;
                }
                else if (type == 10)
                {
                    pg.add_pre2pre(r, r2, weight);
                    for (int i = 1; i <= r; i++)
                        for (int j = 1; j <= r2; j++) edge(i, j, w);
                    added = r && r2;
                }
                else
                {
                    new_id = pg.add_pre2new(r, weight);
                    for (int i = 1; i <= r; i++) edge(i, count + 1, w);
                    added = r != 0;
                }
            }
            assert(pg.g.edge_cnt() == before + added);
            assert(pg.tot == oldtot + (new_id != 0));
            if (new_id)
            {
                assert(new_id == oldtot + 1);
                id.push_back(new_id);
            }
        }
        compare(pg, id, d);
    }
    for (int n : {32, 1, 2, 17, 32})
    {
        pg.build(n);
        int base = pg.tot;
        for (int i = 0; i < 48; i++)
        {
            int p = pg.add_suf2new(n + 1);
            assert(p == base + 1 + i && pg.g.head[p] == -1);
        }
        assert(pg.g.edge_cnt() == directions * 4 * (n - 1));
    }
    PrefixGraph<W, Mixed, true> noextra(1, 0, 0), default_extra(3);
    noextra.build(1);
    noextra.add_suf2suf(1, 1);
    if constexpr (Mixed)
    {
        noextra.add_pre2suf(1, 1);
        noextra.add_suf2pre(1, 1);
    }
    assert(noextra.tot == 1);
    default_extra.build(1);
    for (int i = 0; i < 3; i++) default_extra.add_suf2new(2);
}

void negative_and_boundary()
{
    PrefixGraph<LL> pg(7, 0, 3);
    pg.build(7);
    pg.add_pre2p(3, 5, -7);
    int p = pg.add_pre2new(4, -3);
    pg.add_p2p(p, 6, -9);
    pg.add_pre2p(6, 7, -1);
    VVLL d(pg.tot + 1, VLL(pg.tot + 1, BIG));
    for (int i = 1; i <= pg.tot; i++)
    {
        d[i][i] = 0;
        for (auto& e : pg.g[i]) d[i][e.v] = min(d[i][e.v], e.w);
    }
    floyd(d);
    for (int i = 1; i <= 4; i++) assert(d[i][6] == -12 && d[i][7] == -13);
    for (int i = 1; i <= 3; i++) assert(d[i][5] == -7);
    assert(d[5][6] == BIG && d[7][1] == BIG);
    pg.build(2);
    pg.add_p2pre(2, 1, INF - 1);
    Dijkstra dij(pg.tot);
    dij.run(2, pg.g, pg.tot);
    assert(dij.dist[1] == INF - 1);

    PrefixSuffixGraph<LL> both(7, 0, 0);
    both.build(7);
    both.add_pre2suf(2, 6, -4);
    both.add_suf2p(6, 4, -3);
    both.add_suf2pre(7, 3, 10); // 返边保证环总权非负
    VVLL dist(both.tot + 1, VLL(both.tot + 1, BIG));
    for (int i = 1; i <= both.tot; i++)
    {
        dist[i][i] = 0;
        for (auto& e : both.g[i]) dist[i][e.v] = min(dist[i][e.v], e.w);
    }
    floyd(dist);
    assert(dist[1][6] == -4 && dist[2][4] == -7 && dist[7][3] == 10);
    assert(dist[4][1] == BIG && dist[3][6] == BIG);
}

void stress()
{
    constexpr int N = 200000, Q = 1000000;
    PrefixGraph<int> pg(N, 4 * N + Q, N);
    for (int n : {N, 1, 257, N})
    {
        pg.build(n);
        int base = pg.tot;
        pg.add_p2pre(n, n - 1, 7);
        for (int i = 0; i < Q; i++)
        {
            int r = i % n + 1;
            switch (i % 5)
            {
                case 0: pg.add_p2p(n, r, 11); break;
                case 1: pg.add_p2pre(n, r, 7); break;
                case 2: pg.add_pre2p(r, n, 19); break;
                case 3: pg.add_pre2pre(r, n, 13); break;
                case 4: pg.add_pre2new(r, 3); break;
            }
        }
        assert(pg.g.edge_cnt() == 4 * n - 4 + Q + (n > 1));
        assert(pg.tot == base + Q / 5);
        Dijkstra dij(pg.tot);
        dij.run(n, pg.g, pg.tot);
        for (int v = 1; v < n; v++) assert(dij.dist[v] == 7);
        assert(dij.dist[n] == 0);
        for (int p = base + 1; p <= pg.tot; p++)
        {
            int r = (5 * (p - base) - 1) % n + 1;
            assert(dij.dist[p] == (r == n ? 3 : 10));
        }
    }
    pg.build(N);
    for (int i = 2; i <= N; i++) pg.add_p2pre(i, i - 1, 1);
    Dijkstra dij(pg.tot);
    dij.run(N, pg.g, pg.tot);
    for (int i = 1; i < N; i++) assert(dij.dist[i] == 1);
    pg.build(N);
    for (int i = 2; i <= N; i++) pg.add_pre2p(i - 1, i, 2);
    dij.run(1, pg.g, pg.tot);
    for (int i = 2; i <= N; i++) assert(dij.dist[i] == 2);
}

template <bool Mixed>
void stress_suffix()
{
    constexpr int N = 200000, Q = 1000000, directions = 1 + Mixed;
    PrefixGraph<int, Mixed, true> pg(N, directions * 4 * N + Q, Q / 8);
    for (int n : {N, 1, 257, N})
    {
        pg.build(n);
        int base = pg.tot;
        pg.add_p2suf(1, 2, 7);
        for (int i = 0; i < Q; i++)
        {
            int l = i % n + 1;
            switch (i % 8)
            {
                case 0: pg.add_p2p(1, l, 11); break;
                case 1: pg.add_p2suf(1, l, 7); break;
                case 2: pg.add_suf2p(l, 1, 19); break;
                case 3: pg.add_suf2suf(l, 1, 13); break;
                case 4: pg.add_suf2new(l, 3); break;
                case 5:
                    if constexpr (Mixed) pg.add_pre2suf(l, 1, 13);
                    else pg.add_suf2suf(l, 1, 13);
                    break;
                case 6:
                    if constexpr (Mixed) pg.add_suf2pre(l, n, 13);
                    else pg.add_suf2p(l, 1, 19);
                    break;
                case 7:
                    if constexpr (Mixed) pg.add_p2pre(1, l, 7);
                    else pg.add_p2suf(1, l, 7);
                    break;
            }
        }
        assert(pg.g.edge_cnt() == directions * 4 * (n - 1) + Q + (n > 1));
        assert(pg.tot == base + Q / 8);
        Dijkstra dij(pg.tot);
        dij.run(1, pg.g, pg.tot);
        assert(dij.dist[1] == 0);
        for (int v = 2; v <= n; v++) assert(dij.dist[v] == 7);
        for (int p = base + 1; p <= pg.tot; p++)
        {
            int l = (8 * (p - base - 1) + 4) % n + 1;
            assert(dij.dist[p] == (l == 1 ? 3 : 10));
        }
    }
}
}

int main()
{
    prefixgraph_check::small<int>();
    prefixgraph_check::small<LL>();
    prefixgraph_check::small<Empty>();
    prefixgraph_check::small_suffix<int, false>();
    prefixgraph_check::small_suffix<LL, false>();
    prefixgraph_check::small_suffix<Empty, false>();
    prefixgraph_check::small_suffix<int, true>();
    prefixgraph_check::small_suffix<LL, true>();
    prefixgraph_check::small_suffix<Empty, true>();
    prefixgraph_check::negative_and_boundary();
    prefixgraph_check::stress();
    prefixgraph_check::stress_suffix<false>();
    prefixgraph_check::stress_suffix<true>();
    cout << "prefixgraph_check PASS: 3600 independent cases, 3 modes, boundaries, rebuilds, 200000 points / 1000000 operations per mode" << endl;
}
