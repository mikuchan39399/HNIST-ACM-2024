// LCA/虚树四种组合: 小树全点对 + 独立删枝参照 + 大规模形态与多测复用
// 默认运行小树及 20 万点浅树; --large N 跑 N 点浅树, --deep N 单独探测 N 点链
// --probe-dfn/--probe-hld N 分别探测单引擎深链; 须记录栈限制, 不将增栈结果冒充默认通过
#include "../二次排序.cpp"
#include "../单调栈.cpp"
#include "../../最近公共祖先/DFN_LCA.cpp"
#include "../../最近公共祖先/HLD_LCA.cpp"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <random>
#include <tuple>

using Edge = tuple<int, int, LL>;

struct Ref
{
    int n;
    VI par, dep, rt, order, sz;
    VLL pw, dis;
    Ref(int n) : n(n), par(n + 1), dep(n + 1), rt(n + 1), sz(n + 1, 1), pw(n + 1), dis(n + 1) {}
    void finish()
    {
        for (int u : order)
        {
            int p = par[u];
            dep[u] = dep[p] + 1;
            dis[u] = dis[p] + pw[u];
            rt[u] = p ? rt[p] : u;
        }
        for (auto it = order.rbegin(); it != order.rend(); ++it)
            if (par[*it]) sz[par[*it]] += sz[*it];
    }
    int lca(int u, int v) const
    {
        if (rt[u] != rt[v]) return -1;
        while (dep[u] > dep[v]) u = par[u];
        while (dep[v] > dep[u]) v = par[v];
        while (u != v) { u = par[u]; v = par[v]; }
        return u;
    }
    int jump(int u, int v, int k) const
    {
        if (k <= 0) return u;
        int p = lca(u, v);
        if (p == -1) return -1;
        int a = dep[u] - dep[p], b = dep[v] - dep[p];
        if (k >= a + b) return v;
        if (k > a) { k = a + b - k; u = v; }
        while (k--) u = par[u];
        return u;
    }
    // 只按父树剪去无关键点的枝, 保留必选点及至少两个有效儿子的分叉点
    // 不调用任何 LCA, 不使用被测 DFS 序或单调栈
    vector<Edge> virtual_edges(const VI& keys, int root) const
    {
        if (keys.empty()) return {};
        for (int u : keys) if (rt[u] != rt[root]) return {};
        VI active(n + 1), branches(n + 1), keep(n + 1), nearest(n + 1);
        keep[root] = active[root] = 1;
        for (int u : keys) keep[u] = active[u] = 1;
        for (auto it = order.rbegin(); it != order.rend(); ++it)
        {
            int u = *it, p = par[u];
            if (branches[u] >= 2) keep[u] = 1;
            if (p && active[u]) { active[p] = 1; branches[p]++; }
        }
        vector<Edge> result;
        for (int u : order)
        {
            int p = nearest[par[u]];
            if (keep[u] && p) result.emplace_back(min(u, p), max(u, p), dis[u] - dis[p]);
            nearest[u] = keep[u] ? u : p;
        }
        sort(result.begin(), result.end());
        return result;
    }
};

static vector<Edge> collect(Graph<false, LL>& g)
{
    vector<Edge> result;
    for (size_t i = 0; i < g.edges.size(); i += 2)
    {
        int u = g.edges[i ^ 1].v, v = g.edges[i].v;
        assert(u != v);
        result.emplace_back(min(u, v), max(u, v), g.edges[i].w);
    }
    sort(result.begin(), result.end());
    return result;
}

struct Check
{
    Graph<false, LL> g;
    LCA dfn;
    HLD_LCA hld;
    VirtualTree sort_vt;
    VirtualTreeStack stack_vt;
    explicit Check(int cap) : g(cap, cap), dfn(cap), hld(cap), sort_vt(cap), stack_vt(cap) {}
    void build(const Ref& r, int hld_root = -1)
    {
        g.clear();
        dfn.init(r.n);
        hld.init(r.n);
        for (int u = 1; u <= r.n; u++) if (r.par[u]) g.add(u, r.par[u], r.pw[u]);
        dfn.build(g);
        hld.build(g, hld_root);
    }
    template <class L>
    void members(L& l, const Ref& r)
    {
        VI seen(r.n + 1);
        for (int u = 1; u <= r.n; u++)
        {
            assert(l.dep[u] == r.dep[u] && l.rt[u] == r.rt[u]);
            assert(l.dis[u] == r.dis[u] && l.sz[u] == r.sz[u]);
            assert(l.dfn[u] >= 1 && l.dfn[u] <= r.n && !seen[l.dfn[u]]++);
            if constexpr (is_same_v<L, LCA>)
            {
                assert(l.fa[0][u] == r.par[u]);
                assert(l.rnk[l.dfn[u]] == u);
            }
            else assert(l.fa[u] == r.par[u]);
        }
    }
    template <class L>
    void virtuals(L& l, const Ref& r, const VI& keys, int root)
    {
        auto expected = r.virtual_edges(keys, root);
        auto verify = [&](auto& vt)
        {
            VI input = keys;
            vt.build(input, l, root);
            assert(input == keys && collect(vt.tree) == expected);
        };
        verify(sort_vt);
        verify(stack_vt);
    }
    void query(const Ref& r, int u, int v, bool deep = false)
    {
        // 深链以编号闭式求答案, 避免用 O(n) 爬链暴力检验每个大规模查询
        int p = deep ? min(u, v) : r.lca(u, v);
        LL distance = p == -1 ? -1 : r.dis[u] + r.dis[v] - 2 * r.dis[p];
        assert(dfn.lca(u, v) == p && hld.lca(u, v) == p);
        assert(dfn.dist(u, v) == distance && hld.dist(u, v) == distance);
        int len = p == -1 ? 0 : r.dep[u] + r.dep[v] - 2 * r.dep[p];
        for (int k : {-1, 0, 1, len / 2, len, len + 1})
        {
            int expected = deep ? (k <= 0 ? u : u + (u < v ? 1 : -1) * min(k, len)) : r.jump(u, v, k);
            assert(dfn.jump(u, v, k) == expected);
        }
    }
};

static Ref reroot(const Ref& source, int root)
{
    Ref r(source.n);
    vector<vector<pair<int, LL>>> adj(r.n + 1);
    for (int u = 1; u <= r.n; u++) if (source.par[u])
    {
        adj[u].push_back({source.par[u], source.pw[u]});
        adj[source.par[u]].push_back({u, source.pw[u]});
    }
    r.order.push_back(root);
    for (size_t i = 0; i < r.order.size(); i++)
    {
        int u = r.order[i];
        for (auto [v, w] : adj[u]) if (v != r.par[u])
        {
            r.par[v] = u;
            r.pw[v] = w;
            r.order.push_back(v);
        }
    }
    assert((int)r.order.size() == r.n);
    r.finish();
    return r;
}

static void small()
{
    mt19937 rng(42);
    Check c(48);
    for (int tc = 0; tc < 400; tc++)
    {
        int n = 1 + rng() % 40;
        Ref r(n);
        bool connected = tc % 2 == 0;
        for (int u = 1; u <= n; u++)
        {
            r.order.push_back(u);
            if (u > 1 && (connected || rng() % 4))
            {
                r.par[u] = 1 + rng() % (u - 1);
                r.pw[u] = ((int)(rng() % 21) - 10) * 1000000000000LL;
            }
        }
        r.finish();
        c.build(r);
        c.members(c.dfn, r);
        c.members(c.hld, r);
        for (int u = 1; u <= n; u++) for (int v = 1; v <= n; v++) c.query(r, u, v);
        assert(c.dfn.lca(VI{}) == -1 && c.hld.lca(VI{}) == -1);
        for (int t = 0; t < 12; t++)
        {
            VI keys;
            int k = rng() % (2 * n + 1), root = 1 + rng() % n;
            int expected = -1;
            for (int i = 0; i < k; i++)
            {
                int u = 1 + rng() % n;
                keys.push_back(u);
                if (i == 0) expected = u;
                else if (expected != -1) expected = r.lca(expected, u);
            }
            assert(c.dfn.lca(keys) == expected && c.hld.lca(keys) == expected);
            c.virtuals(c.dfn, r, keys, root);
            c.virtuals(c.hld, r, keys, root);
        }
        if (connected)
        {
            int root = 1 + rng() % n;
            Ref rr = reroot(r, root);
            c.hld.init(n);
            c.hld.build(c.g, root);
            c.members(c.hld, rr);
            for (int u = 1; u <= n; u++) for (int v = 1; v <= n; v++)
            {
                int p = rr.lca(u, v);
                assert(c.hld.lca(u, v) == p);
                assert(c.hld.dist(u, v) == rr.dis[u] + rr.dis[v] - 2 * rr.dis[p]);
            }
            VI keys;
            for (int i = 0; i < 2 * n; i++) keys.push_back(1 + rng() % n);
            c.virtuals(c.hld, rr, keys, 1 + rng() % n);
        }
    }
    // 两边权和逼近 LL 两端, 同时保持所有中间算式在契约内
    for (LL w : {0LL, numeric_limits<LL>::max() / 2, numeric_limits<LL>::min() / 2})
    {
        Ref r(3);
        r.order = {1, 2, 3};
        r.par[2] = r.par[3] = 1;
        r.pw[2] = r.pw[3] = w;
        r.finish();
        c.build(r);
        for (int u = 1; u <= 3; u++) for (int v = 1; v <= 3; v++) c.query(r, u, v);
        c.virtuals(c.dfn, r, {3, 2, 3}, 2);
        c.virtuals(c.hld, r, {3, 2, 3}, 2);
    }
    c.g.clear();
    c.dfn.init(0); c.hld.init(0);
    c.dfn.build(c.g); c.hld.build(c.g);
    assert(c.dfn.idx == 0 && c.hld.dfn_idx == 0);
    assert(c.dfn.lca(VI{}) == -1 && c.hld.lca(VI{}) == -1);
    cout << "small: 400 weighted forests, all pairs, four combinations, HLD reroot and LL boundaries passed" << endl;
}

static void large(int n, bool deep)
{
    Check c(n);
    mt19937 rng(42);
    for (int shape = 0; shape < (deep ? 1 : 4); shape++)
    {
        Ref r(n);
        for (int u = 1; u <= n; u++)
        {
            r.order.push_back(u);
            if (u > 1)
            {
                r.par[u] = deep ? u - 1 : shape == 0 ? 1 : shape == 1 ? u / 2 : shape == 2 ? 1 + rng() % (u - 1) : (u % 64 == 1 ? 0 : u - 1);
                if (r.par[u]) r.pw[u] = ((u % 3) - 1) * 1000000000000LL;
            }
        }
        r.finish();
        cout << "begin n=" << n << " shape=" << (deep ? "chain" : to_string(shape)) << endl << flush;
        c.build(r);
        c.members(c.dfn, r);
        c.members(c.hld, r);
        for (int t = 0; t < 100000; t++) c.query(r, 1 + rng() % n, 1 + rng() % n, deep);
        VI keys;
        for (int u = 1; u <= n; u++) if (r.rt[u] == r.rt[1]) keys.push_back(u);
        // 全点及 2n 长重复输入, 随后空集/单点/稀疏点集, 检验大-小-大复用
        VI duplicated = keys;
        duplicated.insert(duplicated.end(), keys.begin(), keys.end());
        reverse(duplicated.begin(), duplicated.end());
        auto both = [&](const VI& nodes, int root)
        {
            c.virtuals(c.dfn, r, nodes, root);
            c.virtuals(c.hld, r, nodes, root);
        };
        both(duplicated, 1);
        both({}, 1);
        both({1, 1}, 1);
        both(keys, 1);
        VI sparse;
        for (size_t i = 0; i < keys.size(); i += 997) sparse.push_back(keys[i]);
        both(sparse, keys.back());
        if (shape == 3 && !deep) both({1, n}, 1);
        Ref singleton(1); singleton.order = {1}; singleton.finish();
        c.build(singleton);
        c.query(singleton, 1, 1);
        c.virtuals(c.dfn, singleton, {1}, 1);
        c.virtuals(c.hld, singleton, {}, 1);
        cout << "passed n=" << n << " shape=" << (deep ? "chain" : to_string(shape)) << endl;
    }
}

int main(int argc, char** argv)
{
    if (argc == 3)
    {
        int n = atoi(argv[2]);
        assert(n > 0);
        string mode = argv[1];
        if (mode == "--probe-dfn" || mode == "--probe-hld")
        {
            Graph<false> g(n, n);
            for (int u = 2; u <= n; u++) g.add(u - 1, u);
            cerr << "probe " << mode << " n=" << n << endl;
            if (mode == "--probe-dfn")
            {
                LCA l(n); l.build(g);
                assert(l.lca(max(1, n / 2), n) == max(1, n / 2));
            }
            else
            {
                HLD_LCA l(n); l.build(g);
                assert(l.lca(max(1, n / 2), n) == max(1, n / 2));
            }
            cout << "probe passed" << endl;
            return 0;
        }
        assert(mode == "--large" || mode == "--deep");
        large(n, mode == "--deep");
    }
    else
    {
        assert(argc == 1);
        small();
        large(200000, false);
    }
    cout << "lca_vt_stress_check passed" << endl;
}
