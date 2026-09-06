// 树中心 vs 独立全源最短路, 小树逐项核对偏心距与全部中心
#include <cassert>
#include <limits>
#include <numeric>
#include <random>
#include <tuple>
#include "../树的中心.cpp"

using E = tuple<int, int, LL>;

template <class W>
void check(int n, const vector<E>& edges)
{
    static Graph<false, W> g(2100, 2100);
    static TreeCenter<Graph<false, W>> tc;
    g.clear();
    const LL unreachable = numeric_limits<LL>::max() / 4;
    vector<VLL> d(n + 1, VLL(n + 1, unreachable));
    for (int u = 1; u <= n; u++) d[u][u] = 0;
    for (auto [u, v, w] : edges)
    {
        if constexpr (is_same_v<W, Empty>)
        {
            g.add(u, v);
            w = 1;
        }
        else g.add(u, v, w);
        d[u][v] = d[v][u] = w;
    }
    for (int k = 1; k <= n; k++)
        for (int u = 1; u <= n; u++)
            for (int v = 1; v <= n; v++)
                d[u][v] = min(d[u][v], d[u][k] + d[k][v]);
    VLL ecc(n + 1);
    LL radius = unreachable, diameter = 0;
    for (int u = 1; u <= n; u++)
    {
        ecc[u] = *max_element(d[u].begin() + 1, d[u].end());
        radius = min(radius, ecc[u]);
        diameter = max(diameter, ecc[u]);
    }
    VI centers;
    for (int u = 1; u <= n; u++)
        if (ecc[u] == radius) centers.push_back(u);
    assert(tc.build(g, n) == radius);
    assert(tc.radius == radius && tc.diameter == diameter);
    assert(tc.ecc == ecc && tc.centers == centers);
    assert(1 <= tc.end_u && tc.end_u <= n && 1 <= tc.end_v && tc.end_v <= n);
    assert(d[tc.end_u][tc.end_v] == diameter);
}

void large_shapes()
{
    Graph<false> g(2000, 1999);
    TreeCenter<Graph<false>> tc;
    for (int n : {2000, 1, 2, 1999})
    {
        g.clear();
        for (int u = 2; u <= n; u++) g.add(u - 1, u);
        assert(tc.build(g, n) == n / 2);
        assert(tc.diameter == n - 1);
        VI expected{(n + 1) / 2};
        if (n % 2 == 0) expected.push_back(n / 2 + 1);
        assert(tc.centers == expected);
        for (int u = 1; u <= n; u++) assert(tc.ecc[u] == max(u - 1, n - u));
    }
    g.clear();
    for (int u = 2; u <= 2000; u++) g.add(1, u);
    assert(tc.build(g, 2000) == 1 && tc.centers == VI({1}));
    assert(tc.diameter == 2);
    for (int u = 2; u <= 2000; u++) assert(tc.ecc[u] == 2);
}

int main()
{
    check<Empty>(1, {});
    check<LL>(1, {});
    check<Empty>(4, {{1, 2, 1}, {2, 3, 1}, {3, 4, 1}});
    check<Empty>(5, {{1, 2, 1}, {2, 3, 1}, {3, 4, 1}, {4, 5, 1}});
    check<LL>(2, {{1, 2, 10}});
    check<LL>(5, {{1, 2, 0}, {1, 3, 0}, {1, 4, 0}, {1, 5, 0}});
    check<LL>(5, {{1, 2, 5}, {2, 3, 5}, {2, 4, 0}, {4, 5, 0}});
    check<LL>(3, {{1, 2, 1000000000000LL}, {2, 3, 1000000000000LL}});
    mt19937 rng(42);
    for (int tc = 0; tc < 400; tc++)
    {
        int n = 1 + (int)(rng() % 24);
        VI label(n);
        iota(label.begin(), label.end(), 1);
        shuffle(label.begin(), label.end(), rng);
        vector<E> edges;
        for (int v = 1; v < n; v++)
            edges.emplace_back(label[v], label[rng() % v], rng() % 21);
        shuffle(edges.begin(), edges.end(), rng);
        check<Empty>(n, edges);
        check<LL>(n, edges);
    }
    large_shapes();
    cout << "Tree center checks passed\n";
}
