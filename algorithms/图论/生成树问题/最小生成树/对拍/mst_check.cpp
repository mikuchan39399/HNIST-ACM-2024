#include "../kruskal/Kruskal.cpp"
#include "../prim/Prim.cpp"
#include "../kruskal/Kruskal.cpp"
#include "../prim/Prim.cpp"
#include "../../../../杂项/128位整数/128int.cpp"

struct TestEdge { int u, v; LL w; };
// 逐次全表重染色, 不调用 DSU 或任何 MST 模板
static bool paint(VI& color, int u, int v)
{
    int a = color[u], b = color[v];
    if (a == b) return false;
    for (int& c : color) if (c == b) c = a;
    return true;
}
static pair<LL, int> brute_forest(int n, const vector<TestEdge>& es)
{
    VI base(n + 1); iota(base.begin(), base.end(), 0);
    int components = n;
    for (auto e : es) components -= paint(base, e.u, e.v);
    int needed = n - components, m = es.size();
    i128 best = i128(1) << 120;
    for (unsigned mask = 0; mask < (1U << m); mask++)
    {
        if (popcount(mask) != needed) continue;
        VI color(n + 1); iota(color.begin(), color.end(), 0);
        i128 sum = 0; bool valid = true;
        for (int i = 0; i < m; i++) if ((mask >> i) & 1)
        {
            auto e = es[i];
            if (!paint(color, e.u, e.v)) { valid = false; break; }
            sum += e.w;
        }
        if (valid) best = min(best, sum); // 森林有 n-components 条边即已跨满原分量
    }
    assert(best >= LLONG_MIN && best <= LLONG_MAX);
    return {LL(best), components};
}

template <class Solver, class W>
static void verify(Solver& mst, Graph<false, W>& g, int n, const vector<TestEdge>& es, LL expected, int components)
{
    vector<TestEdge> snapshot;
    for (int u = 1; u <= n; u++) for (auto& e : g[u]) snapshot.push_back({u, e.v, e.w});
    bool connected = mst.build(g, n);
    assert(connected == (components == 1));
    assert(mst.weight == expected && mst.components == components);
    assert(int(mst.edges.size()) == n - components);
    VI color(n + 1); iota(color.begin(), color.end(), 0);
    i128 sum = 0;
    for (int id : mst.edges)
    {
        assert(id >= 0 && id % 2 == 0 && id / 2 < int(es.size()));
        auto e = es[id / 2];
        assert(paint(color, e.u, e.v));
        sum += e.w;
    }
    assert(sum == expected);
    for (auto e : es) assert(color[e.u] == color[e.v]);
    size_t pos = 0;
    for (int u = 1; u <= n; u++) for (auto& e : g[u])
    {
        auto old = snapshot[pos++];
        assert(old.u == u && old.v == e.v && old.w == e.w);
    }
    assert(pos == snapshot.size());
}

template <class W>
static void small()
{
    mt19937 rng(42);
    Graph<false, W> g(8, 14);
    Kruskal k; Prim p;
    for (int tc = 0; tc < 800; tc++)
    {
        int n = 1 + rng() % 7, m = rng() % 13;
        g.clear(); vector<TestEdge> es;
        for (int i = 0; i < m; i++)
        {
            int u = 1 + rng() % n, v = 1 + rng() % n;
            LL w = tc % 4 == 0 ? 0 : int(rng() % 31) - 15;
            g.add(u, v, W(w)); es.push_back({u, v, w});
        }
        auto [weight, components] = brute_forest(n, es);
        verify(k, g, n, es, weight, components);
        verify(p, g, n, es, weight, components);
        verify(k, g, n, es, weight, components);
        verify(p, g, n, es, weight, components);
    }
}

static void boundaries()
{
    Graph<false, LL> g(4, 10);
    Kruskal k; Prim p;
    for (LL w : {LLONG_MIN, -1LL, 0LL, INF, LLONG_MAX})
    {
        g.clear(); g.add(1, 2, w); g.add(1, 1, LLONG_MIN);
        vector<TestEdge> es{{1, 2, w}, {1, 1, LLONG_MIN}};
        verify(k, g, 2, es, w, 1); verify(p, g, 2, es, w, 1);
    }
    g.clear();
    vector<TestEdge> es{{1, 2, 4000000000000000000LL}, {2, 3, 4000000000000000000LL}, {1, 3, 4100000000000000000LL}};
    for (auto e : es) g.add(e.u, e.v, e.w);
    verify(k, g, 4, es, 8000000000000000000LL, 2);
    verify(p, g, 4, es, 8000000000000000000LL, 2);
    g.clear(); es.clear();
    verify(k, g, 1, es, 0, 1); verify(p, g, 1, es, 0, 1);
}

// 大图用已知最优骨架与正差替代边给出独立答案, 再线性检查返回森林
template <class Solver>
static void large(Solver& mst, int n, int shape)
{
    Graph<false, LL> g(n, 3 * n);
    LL sum = 0;
    int components = 1;
    for (int v = 2; v <= n; v++)
    {
        if (shape == 2 && v % 257 == 1) { components++; continue; }
        int u = shape == 1 ? 1 : v - 1;
        LL w = shape == 3 ? 0 : -LL(v);
        sum += w;
        g.add(u, v, w);
        g.add(u, v, w + 1);
        g.add(v, v, LLONG_MIN); // 负自环不能混入生成树
    }
    assert(mst.build(g, n) == (components == 1));
    assert(mst.weight == sum && mst.components == components && int(mst.edges.size()) == n - components);
    vector<vector<int>> adj(n + 1);
    i128 got = 0;
    for (int id : mst.edges)
    {
        assert(id >= 0 && id % 2 == 0 && id < int(g.edges.size()));
        int u = g.edges[id ^ 1].v, v = g.edges[id].v;
        got += g.edges[id].w; adj[u].push_back(v); adj[v].push_back(u);
    }
    assert(got == sum);
    VI color(n + 1); int count = 0;
    for (int s = 1; s <= n; s++) if (!color[s])
    {
        count++; VI q{s}; color[s] = count;
        for (size_t j = 0; j < q.size(); j++) for (int v : adj[q[j]])
            if (!color[v]) { color[v] = count; q.push_back(v); }
    }
    assert(count == components);
    for (int u = 1; u <= n; u++) for (auto& e : g[u]) assert(color[u] == color[e.v]);
}

static void scale()
{
    Kruskal k; Prim p;
    for (int n : {200000, 1, 257, 200000})
        for (int shape = 0; shape < 4; shape++) large(k, n, shape);
    for (int n : {2000, 1, 257, 2000})
        for (int shape = 0; shape < 4; shape++) large(p, n, shape);
    // 完全图 w(u,v)=|u-v|, 任何 n-1 条边至少 n-1, 相邻链取到下界
    for (int n : {2000, 1, 257, 2000})
    {
        Graph<false, int> g(n, n * (n - 1) / 2);
        for (int u = 1; u <= n; u++) for (int v = u + 1; v <= n; v++) g.add(u, v, v - u);
        for (int rep = 0; rep < 2; rep++)
        {
            assert(p.build(g, n) && p.weight == n - 1 && int(p.edges.size()) == n - 1);
            assert(k.build(g, n) && k.weight == n - 1 && int(k.edges.size()) == n - 1);
            for (const VI* chosen : {&k.edges, &p.edges})
            {
                VI ids = *chosen;
                sort(ids.begin(), ids.end());
                assert(adjacent_find(ids.begin(), ids.end()) == ids.end());
                for (int id : ids) assert(id >= 0 && id % 2 == 0 && id < int(g.edges.size()) && g.edges[id].w == 1);
            }
        }
    }
}

int main()
{
    small<int>(); small<LL>(); boundaries(); scale();
    cout << "[PASS] MST: 1600 exhaustive edge-subset oracles, LL extrema, 200000 sparse and 2000 dense, reuse and returned forests\n";
}
