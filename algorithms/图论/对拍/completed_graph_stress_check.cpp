// 已完成项补验: Graph/Topo/直径双实现/重心/中心/HLD/SCC/EBCC/VBCC/BCT
// 默认 20 万点浅结构及 HLD 独立暴力; --deep N 用单链, 由 CI 设置 256 MiB 栈
#include "../图的存储/Graph.cpp"
#include "../拓扑排序/拓扑排序.cpp"
#include "../树上问题/树的直径/两次dfs.cpp"
#include "../树上问题/树的直径/树形dp法.cpp"
#include "../树上问题/树的重心/树的重心.cpp"
#include "../树上问题/树的中心/树的中心.cpp"
#include "../树上问题/树链剖分/HLD.cpp"
#include "../连通性相关/Tarjan_SCC.cpp"
#include "../连通性相关/Tarjan_EBCC.cpp"
#include "../连通性相关/Tarjan_VBCC.cpp"
#include "../连通性相关/圆方树/BCT.cpp"
#include <cassert>
#include <cstdlib>
#include <numeric>
#include <random>

struct AuditTag
{
    LL add = 0;
    void apply(const AuditTag& t) { add += t.add; }
    void clear() { add = 0; }
    bool has_tag() const { return add != 0; }
};
struct AuditInfo
{
    LL len = 0, sum = 0, mx = numeric_limits<LL>::min();
    bool break_cond(const AuditTag&) const { return false; }
    bool tag_cond(const AuditTag&) const { return true; }
    void apply(const AuditTag& t) { if (len) { sum += len * t.add; mx += t.add; } }
    friend AuditInfo operator+(const AuditInfo& a, const AuditInfo& b)
    {
        return {a.len + b.len, a.sum + b.sum, max(a.mx, b.mx)};
    }
};

template <bool Dir, class W>
static void graph_bulk(int n)
{
    Graph<Dir, W> g(n, 0);
    VVI ids(n + 1);
    VI to;
    VLL weight;
    for (int i = 0; i < 3 * n; i++)
    {
        int u = i % n + 1, v = i % 5 == 0 ? u : (i * 17LL + 7) % n + 1;
        LL w = i % 3 == 0 ? numeric_limits<LL>::min() : i % 3 == 1 ? numeric_limits<LL>::max() : i;
        int id;
        if constexpr (is_same_v<W, Empty>) id = g.add(u, v);
        else id = g.add(u, v, w);
        assert(id == (int)to.size());
        ids[u].push_back(id); to.push_back(v); weight.push_back(w);
        if constexpr (!Dir) { ids[v].push_back(id + 1); to.push_back(u); weight.push_back(w); }
    }
    auto verify = [&](auto& x)
    {
        VI in(n + 1);
        for (int v : to) in[v]++;
        assert(x.edge_cnt() == 3 * n && x.node_cnt() == n);
        for (int u = 1; u <= n; u++)
        {
            assert(x.deg[u] == (int)ids[u].size());
            if constexpr (Dir) assert(x.in_deg[u] == in[u]);
            size_t k = ids[u].size();
            for (auto& e : x[u])
            {
                assert(k > 0);
                int id = ids[u][--k];
                assert(x.id(e) == id && e.v == to[id]);
                if constexpr (!is_same_v<W, Empty>) assert(e.w == weight[id]);
                if constexpr (!Dir) assert(x.edges[x.rev(id)].v == u);
            }
            assert(k == 0);
        }
    };
    verify(g);
    Graph<Dir, W> copy(g), moved(move(copy));
    verify(moved);
    moved.clear();
    assert(moved.edge_cnt() == 0 && moved.used.empty());
    verify(g);
    size_t cap = g.edges.capacity();
    g.clear(); g.clear();
    assert(g.edges.capacity() == cap);
    for (int u = 1; u <= n; u++) assert(g.head[u] == -1 && g.deg[u] == 0);
    for (int i = 0; i < 3 * n; i++)
    {
        if constexpr (is_same_v<W, Empty>) g.add(1, n);
        else g.add(1, n, 1);
    }
    assert(g.edge_cnt() == 3 * n);
}

static void topo_bulk(int n)
{
    Graph<true> g(n, 3 * n);
    TopoSort t;
    for (int count : {n, 0, 1, n})
    {
        g.clear();
        for (int u = 2; u <= count; u++)
        {
            g.add(u / 2, u);
            if (u % 7 == 0) g.add(u / 2, u);
        }
        VI before = g.in_deg;
        assert(t.build(g, count));
        assert(g.in_deg == before && (int)t.get().size() == count);
        VI pos(count + 1, -1);
        for (int i = 0; i < count; i++)
        {
            int u = t.get()[i];
            assert(u >= 1 && u <= count && pos[u] == -1); pos[u] = i;
        }
        for (int u = 2; u <= count; u++) assert(pos[u / 2] < pos[u]);
        if (count) { g.add(count, 1); assert(!t.build(g, count)); }
    }
}

// 等长多臂树: 点 1 为中心, 后续每 arm_len 个编号形成一条臂, 最后一臂可短
static void properties(int cap, bool deep)
{
    Graph<false, LL> g(cap, cap);
    TreeDiameter<decltype(g)> dfs;
    TreeDiameterDP<decltype(g)> dp;
    TreeCenter<decltype(g)> center;
    TreeCentroid<decltype(g)> centroid;
    for (int n : {cap, 1, 2, cap}) for (int arm_len : {1, deep ? cap : 64})
    {
        int arms = n == 1 ? 0 : (n - 2) / arm_len + 1;
        auto arm = [&](int u) { return u == 1 ? -1 : (u - 2) / arm_len; };
        auto depth = [&](int u) { return u == 1 ? 0 : (u - 2) % arm_len + 1; };
        auto length = [&](int a) { return a < 0 ? 0 : min(arm_len, n - 1 - a * arm_len); };
        auto distance = [&](int u, int v)
        {
            return arm(u) == arm(v) ? abs(depth(u) - depth(v)) : depth(u) + depth(v);
        };
        for (LL w : {0LL, 1000000000000LL})
        {
            g.clear();
            for (int u = 2; u <= n; u++) g.add(depth(u) == 1 ? 1 : u - 1, u, w);
            int diameter = arms > 1 ? length(0) + length(1) : arms ? length(0) : 0;
            dfs.build(g, n);
            assert(dfs.len == diameter * w && dp.build(g, n) == diameter * w);
            assert(distance(dfs.end_u, dfs.end_v) * w == dfs.len);
            assert(dfs.path.front() == dfs.end_u && dfs.path.back() == dfs.end_v);
            for (size_t i = 1; i < dfs.path.size(); i++) assert(distance(dfs.path[i - 1], dfs.path[i]) == 1);
            assert((LL)(dfs.path.size() - 1) * w == dfs.len);
            for (int u = 1; u <= n; u++) if (u != dfs.end_u)
            {
                int p = dfs.pre[u];
                assert(p >= 1 && p <= n && distance(u, p) == 1);
                assert(distance(p, dfs.end_u) + 1 == distance(u, dfs.end_u));
            }
            VLL ecc(n + 1);
            LL radius = numeric_limits<LL>::max();
            VI centers;
            for (int u = 1; u <= n; u++)
            {
                int a = arm(u), d = depth(u);
                int other = arms > 1 ? length(a == 0 ? 1 : 0) : 0;
                ecc[u] = (u == 1 ? length(0) : max(length(a) - d, d + other)) * w;
                radius = min(radius, ecc[u]);
            }
            for (int u = 1; u <= n; u++) if (ecc[u] == radius) centers.push_back(u);
            assert(center.build(g, n) == radius && center.centers == centers && center.ecc == ecc);
            assert(center.diameter == diameter * w);
            assert(distance(center.end_u, center.end_v) * w == center.diameter);
            for (auto& e : g.edges) e.w = -w;
            assert(dp.build(g, n) == 0); // 全负/全零允许单点路径
        }
        for (LL point_weight : {-7LL, 0LL, 1000000000000LL})
        {
            centroid.init(n);
            fill(centroid.pt.begin() + 1, centroid.pt.begin() + n + 1, point_weight);
            LL best = numeric_limits<LL>::max();
            VI expected;
            for (int u = 1; u <= n; u++)
            {
                LL value = numeric_limits<LL>::min();
                if (n == 1) value = 0;
                else if (u == 1)
                    value = (point_weight < 0 ? length(arms - 1) : length(0)) * point_weight;
                else
                {
                    int tail = length(arm(u)) - depth(u), parent_side = n - 1 - tail;
                    value = parent_side * point_weight;
                    if (tail) value = max(value, tail * point_weight);
                }
                if (value < best) best = value, expected = {u};
                else if (value == best) expected.push_back(u);
            }
            centroid.build(g, n);
            assert(centroid.min_max_part == best && centroid.centroids == expected);
        }
    }
    g.clear();
    for (int u = 3; u <= cap; u++) g.add(2, u, 1);
    dfs.build(g, cap);
    assert(dfs.len == 0 && dfs.path == VI{1} && dp.build(g, cap) == 0);
}

static void hld_small()
{
    mt19937 rng(42);
    HLD h(48); SegTree<AuditInfo, AuditTag> tree(48);
    for (int tc = 0; tc < 400; tc++)
    {
        int n = 1 + rng() % 40, root = 1 + rng() % n;
        Graph<false> g(n, n); VVI adj(n + 1), children(n + 1);
        for (int u = 2; u <= n; u++)
        {
            int p = 1 + rng() % (u - 1);
            g.add(p, u); adj[p].push_back(u); adj[u].push_back(p);
        }
        VI par(n + 1), depth(n + 1), order{root};
        for (size_t i = 0; i < order.size(); i++) for (int v : adj[order[i]]) if (v != par[order[i]])
        {
            par[v] = order[i]; depth[v] = depth[par[v]] + 1;
            children[par[v]].push_back(v); order.push_back(v);
        }
        h.init(n); h.build(g, root); tree.init(n);
        VLL ref(n + 1); vector<AuditInfo> data(n + 1);
        for (int u = 1; u <= n; u++)
        {
            ref[u] = (int)(rng() % 21) - 10;
            data[h.dfn[u]] = {1, ref[u], ref[u]};
            assert(h.fa[u] == par[u] && h.dep[u] == depth[u] + 1 && h.seg[h.dfn[u]] == u);
        }
        tree.build(data);
        for (int op = 0; op < 160; op++)
        {
            int u = 1 + rng() % n, v = 1 + rng() % n;
            VI points;
            bool subtree = op % 2;
            if (subtree)
            {
                points.push_back(u);
                for (size_t i = 0; i < points.size(); i++) for (int x : children[points[i]]) points.push_back(x);
                assert(h.sz[u] == (int)points.size());
                for (int x : points) assert(h.dfn[x] >= h.dfn[u] && h.dfn[x] < h.dfn[u] + h.sz[u]);
            }
            else
            {
                int a = u, b = v;
                while (a != b) { if (depth[a] < depth[b]) swap(a, b); points.push_back(a); a = par[a]; }
                points.push_back(a);
            }
            if (op % 4 < 2)
            {
                LL delta = (int)(rng() % 21) - 10;
                if (subtree) modify_subtree(h, tree, u, {delta}); else modify_path(h, tree, u, v, {delta});
                for (int x : points) ref[x] += delta;
            }
            AuditInfo expected;
            for (int x : points) expected = expected + AuditInfo{1, ref[x], ref[x]};
            auto result = subtree ? query_subtree(h, tree, u) : query_path(h, tree, u, v);
            assert(result.len == expected.len && result.sum == expected.sum && result.mx == expected.mx);
        }
    }
    h.init(0); Graph<false> empty; h.build(empty); assert(h.dfn_idx == 0);
}

static void hld_bulk(int cap, bool deep)
{
    HLD h(cap); SegTree<AuditInfo, AuditTag> t(cap); Graph<false> g(cap, cap);
    mt19937 rng(42);
    for (int n : {cap, 1, cap})
    {
        g.clear();
        for (int u = 2; u <= n; u++) g.add(deep ? u - 1 : u / 2, u);
        h.init(n); h.build(g); t.init(n);
        vector<AuditInfo> data(n + 1, {1, 0, 0}); t.build(data);
        VLL ref(n + 1); LL global = 0;
        for (int op = 0; op < 100000; op++)
        {
            int u = 1 + rng() % n, v = 1 + rng() % n;
            if (op % 10 == 0) { modify_subtree(h, t, 1, {3}); global += 3; }
            else if (!deep && op % 3 == 0)
            {
                modify_path(h, t, u, v, {-2});
                while (u != v) { if (u < v) swap(u, v); ref[u] -= 2; u /= 2; }
                ref[u] -= 2;
            }
            else
            {
                AuditInfo expected;
                if (deep) expected = {abs(u - v) + 1, (abs(u - v) + 1) * global, global};
                else
                {
                    int a = u, b = v;
                    while (a != b) { if (a < b) swap(a, b); expected = expected + AuditInfo{1, ref[a] + global, ref[a] + global}; a /= 2; }
                    expected = expected + AuditInfo{1, ref[a] + global, ref[a] + global};
                }
                auto got = query_path(h, t, u, v);
                assert(got.len == expected.len && got.sum == expected.sum && got.mx == expected.mx);
            }
        }
        for (int u = 1; u <= n; u++)
        {
            auto got = query_path(h, t, u, u);
            assert(got.len == 1 && got.sum == ref[u] + global && got.mx == got.sum);
        }
    }
}

template <class Block>
static void check_blocks(Block& b, int n, int kind, const VPII& edges, int width)
{
    b.build_tree();
    VVI expected;
    if (kind == 0) { for (int u = 1; u <= n; u++) expected.push_back({u}); }
    else if (kind == 3)
    {
        for (int start = 1; start <= n; start += width)
        {
            VI points; for (int u = start; u <= min(n, start + width - 1); u++) points.push_back(u);
            expected.push_back(points);
        }
    }
    else for (auto [u, v] : edges) expected.push_back({min(u, v), max(u, v)});
    if (n == 1 && expected.empty()) expected.push_back({1});
    VVI got;
    VI degree(n + 1);
    for (auto [u, v] : edges) degree[u]++, degree[v]++;
    size_t incidences = 0;
    for (int i = 1; i <= b.vbcc_cnt; i++)
    {
        VI points = b.vbcc_cir[i]; sort(points.begin(), points.end());
        got.push_back(points); incidences += points.size();
        VI neighbors; for (auto& e : b.tree[n + i]) neighbors.push_back(e.v);
        sort(neighbors.begin(), neighbors.end()); assert(points == neighbors);
    }
    sort(got.begin(), got.end()); sort(expected.begin(), expected.end());
    assert(got == expected && b.tree.edge_cnt() == (int)incidences);
    for (int u = 1; u <= n; u++) assert(bool(b.cut[u]) == (kind != 0 && kind != 3 && degree[u] > 1));
}

static void connectivity(int cap, bool deep)
{
    SCC s(cap, 2 * cap); EBCC e(cap); VBCC v(cap); BCT b(cap);
    Graph<true> dg(cap, 2 * cap); Graph<false> ug(cap, 2 * cap);
    for (int n : {cap, 1, cap}) for (int kind = 0; kind < 4; kind++)
    {
        int width = deep ? n : 64;
        s.init(n); e.init(n); v.init(n); b.init(n); dg.clear(); ug.clear();
        VPII edges;
        if (kind) for (int u = 2; u <= n; u++)
        {
            if (kind == 3 && (u - 1) % width == 0) continue;
            int p = kind == 1 ? 1 : kind == 2 && !deep ? u / 2 : u - 1;
            edges.push_back({p, u});
        }
        for (auto [a, z] : edges) { dg.add(a, z); ug.add(a, z); }
        if (kind == 3) for (int start = 1; start <= n; start += width)
        {
            int last = min(n, start + width - 1);
            if (start != last) { dg.add(last, start); ug.add(last, start); }
        }
        s.build(dg, n); s.build_dag(dg); e.build(ug, n); e.build_tree(ug);
        v.build(ug, n); b.build(ug, n);
        int groups = kind == 3 ? (n + width - 1) / width : n;
        assert(s.scc_cnt == groups && e.ebcc_cnt == groups);
        assert(s.dag.edge_cnt() == (kind == 3 ? 0 : (int)edges.size()));
        assert(e.tree.edge_cnt() == (kind == 3 ? 0 : (int)edges.size()));
        auto bridges = e.get_bridges(ug);
        assert(bridges.size() == (kind == 3 ? 0 : edges.size()));
        for (size_t i = 0; i < bridges.size(); i++) assert(bridges[i] == 2 * (int)i);
        VI seen_s(groups + 1), seen_e(groups + 1);
        for (int u = 1; u <= n; u++)
        {
            int representative = kind == 3 ? (u - 1) / width * width + 1 : u;
            assert(s.bel[u] == s.bel[representative] && e.bel[u] == e.bel[representative]);
            if (u == representative) { assert(!seen_s[s.bel[u]]++ && !seen_e[e.bel[u]]++); }
        }
        check_blocks(v, n, kind, edges, width); check_blocks(b, n, kind, edges, width);
        // 圆方森林接 HLD, 只给圆点记 1, 方点记 0
        HLD h(n + b.vbcc_cnt); h.build(b.tree);
        SegTree<AuditInfo, AuditTag> t(n + b.vbcc_cnt);
        vector<AuditInfo> data(n + b.vbcc_cnt + 1);
        for (int u = 1; u <= n + b.vbcc_cnt; u++) data[h.dfn[u]] = {1, u <= n ? 1LL : 0LL, u <= n ? 1LL : 0LL};
        t.build(data);
        for (int u = 1; u <= n; u += 997)
        {
            int root = kind == 0 ? u : kind == 3 ? (u - 1) / width * width + 1 : 1;
            int count = 1;
            if (kind == 1 || kind == 3) count = root == u ? 1 : 2;
            if (kind == 2) { int x = u; while (x != 1) { count++; x = deep ? x - 1 : x / 2; } }
            assert(query_path(h, t, root, u).sum == count);
        }
    }
}

int main(int argc, char** argv)
{
    int n = argc == 3 ? atoi(argv[2]) : 200000;
    bool deep = argc == 3 && string(argv[1]) == "--deep";
    assert(n > 0 && (argc == 1 || (argc == 3 && (deep || string(argv[1]) == "--large"))));
    if (!deep)
    {
        graph_bulk<true, Empty>(n); graph_bulk<false, Empty>(n);
        graph_bulk<true, LL>(n); graph_bulk<false, LL>(n);
        topo_bulk(n); hld_small();
    }
    properties(n, deep); cout << "properties passed\n" << flush;
    hld_bulk(n, deep); cout << "HLD passed\n" << flush;
    connectivity(n, deep); cout << "connectivity passed\n" << flush;
    cout << "completed_graph_stress_check passed\n";
}
