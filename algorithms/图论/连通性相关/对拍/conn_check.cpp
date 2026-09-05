// 连通性回归：穷举小图 + 多种固定种子 + 结构边界；BCT 副本由 conn_bct_check 单独运行。
#include <iostream>
#include <vector>
#include <cassert>
#include <algorithm>
#include <set>
#include <map>
#include <random>
#include "../Tarjan_SCC.cpp"
#include "../Tarjan_EBCC.cpp"
#ifndef CONN_BCT_ONLY
#include "../Tarjan_VBCC.cpp"
#include "../圆方树/BCT.cpp" // 两个独立类型同场编译
#endif

using namespace std;
#ifdef CONN_BCT_ONLY
using CheckedBlock = BCT;
#else
using CheckedBlock = VBCC;
#endif
static_assert(!is_same_v<BCT, VBCC>);


// 无向图连通块标记: lab[u] = u 所在块内最小编号; skip_v 删点, del[i]=1 删第 i 条边
static VI undirected_comp(int n, const VPII& es, int skip_v, const VI& del)
{
    VVI adj(n + 1);
    for (int i = 0; i < (int)es.size(); i++)
    {
        if (del[i]) continue;
        auto [u, v] = es[i];
        if (u == skip_v || v == skip_v) continue;
        adj[u].push_back(v);
        adj[v].push_back(u);
    }
    VI lab(n + 1, 0);
    for (int s = 1; s <= n; s++)
    {
        if (s == skip_v || lab[s]) continue;
        VI sta{s};
        lab[s] = s;
        while (!sta.empty())
        {
            int u = sta.back();
            sta.pop_back();
            for (int v : adj[u])
            {
                if (v != skip_v && !lab[v])
                {
                    lab[v] = s;
                    sta.push_back(v);
                }
            }
        }
    }
    return lab;
}

// 集合成员表 -> 排序后的多重集 (供比对)
static vector<VI> canonical_sets(vector<VI> s)
{
    for (auto& x : s) sort(x.begin(), x.end());
    sort(s.begin(), s.end());
    return s;
}


// 小图穷举、指定形态、多种固定种子；无向点双不接收自环。
static vector<pair<int, VPII>> cases(bool directed)
{
    vector<pair<int, VPII>> out;
    for (int n = 1; n <= (directed ? 3 : 5); n++)
    {
        VPII slots;
        for (int u = 1; u <= n; u++)
            for (int v = directed ? 1 : u + 1; v <= n; v++) slots.push_back({u, v});
        for (int mask = 0; mask < (1 << slots.size()); mask++)
        {
            VPII es;
            for (size_t i = 0; i < slots.size(); i++) if (mask >> i & 1) es.push_back(slots[i]);
            out.push_back({n, es});
        }
    }
    for (int n : {10, 1, 10, 2, 9})
    {
        out.push_back({n, {}});
        VPII chain, star, cycle, dense;
        for (int u = 2; u <= n; u++) { chain.push_back({u, u - 1}); star.push_back({1, u}); }
        cycle = chain;
        if (n > 1) cycle.push_back({1, n});
        for (int u = 1; u <= n; u++)
            for (int v = directed ? 1 : u + 1; v <= n; v++) dense.push_back({u, v});
        for (auto es : {chain, star, cycle, dense})
        {
            out.push_back({n, es});
            auto copy = es;
            es.insert(es.end(), copy.begin(), copy.end());
            out.push_back({n, es});
        }
    }
    for (unsigned seed : {42u, 20260906u, 998244353u})
    {
        mt19937 rng(seed);
        for (int tc = 0; tc < 400; tc++)
        {
            int n = 1 + rng() % 10, m = rng() % 31;
            VPII es;
            for (int i = 0; i < m; i++)
            {
                int u = 1 + rng() % n, v = 1 + rng() % n;
                if (!directed && u == v) continue;
                es.push_back({u, v});
            }
            out.push_back({n, es});
        }
    }
    return out;
}

static void test_scc()
{
    static SCC scc(30, 300);
    static Graph<true, Empty> ge(30, 300);
    static Graph<true, LL> gw(30, 300);   // 带权形态: SegGraph 注入通道
    static SCC sccw(30, 300);
    for (auto [n, es] : cases(true))
    {
        scc.init(n); sccw.init(n); ge.clear(); gw.clear();
        for (auto [u, v] : es) { ge.add(u, v); gw.add(u, v, -1LL * u); }
        scc.build(ge, n);
        sccw.build(gw, n);                // 带权鸭子等价
        for (int u = 1; u <= n; u++) assert(sccw.bel[u] == scc.bel[u]);

        // 暴力: Warshall 传递闭包, 类标记 = 类内最小编号
        VVI reach(n + 1, VI(n + 1, 0));
        for (int i = 1; i <= n; i++) reach[i][i] = 1;
        for (auto& [u, v] : es) reach[u][v] = 1;
        for (int k = 1; k <= n; k++)
            for (int i = 1; i <= n; i++)
                if (reach[i][k])
                    for (int j = 1; j <= n; j++)
                        if (reach[k][j]) reach[i][j] = 1;
        VI exp(n + 1, 0);
        for (int u = 1; u <= n; u++)
            for (int w = 1; w <= n; w++)
            {
                if (reach[u][w] && reach[w][u])
                {
                    exp[u] = w;
                    break;
                }
            }
        // bel 规范化: got[u] = 同 bel 类内最小编号
        VI got(n + 1, 0);
        for (int u = 1; u <= n; u++)
        {
            int mn = n + 1;
            for (int w = 1; w <= n; w++)
                if (scc.bel[w] == scc.bel[u]) mn = min(mn, w);
            got[u] = mn;
        }
        assert(got == exp);
        set<int> classes(exp.begin() + 1, exp.end());
        assert(scc.scc_cnt == (int)classes.size());

        // 非去重缩点保留每条跨分量边；带权输入的输出仍无权。
        scc.build_dag(gw);
        multiset<PII> raw_expect, raw_got;
        for (auto [u, v] : es) if (scc.bel[u] != scc.bel[v]) raw_expect.insert({scc.bel[u], scc.bel[v]});
        for (int u = 1; u <= scc.scc_cnt; u++)
            for (auto e : scc.dag[u]) { raw_got.insert({u, e.v}); assert(u > e.v); }
        assert(raw_expect == raw_got);
        scc.dag.clear();
        // 去重缩点 DAG 边集
        scc.build_dag_unique(ge);
        VI canon(scc.scc_cnt + 1, 0);
        for (int u = 1; u <= n; u++) canon[scc.bel[u]] = got[u];
        set<PII> expect;
        for (auto& [u, v] : es)
            if (exp[u] != exp[v]) expect.insert({exp[u], exp[v]});
        set<PII> gotE;
        for (int u = 1; u <= scc.scc_cnt; u++)
            for (auto& e : scc.dag[u])
                gotE.insert({canon[u], canon[e.v]});
        assert(gotE == expect);
        assert(scc.dag.edge_cnt() == (int)expect.size());
    }
}

static void test_ebcc()
{
    static EBCC ebcc(30);
    static Graph<false, Empty> g(30, 300);
    auto inputs = cases(false);
    inputs.push_back({1, {{1, 1}, {1, 1}}});
    inputs.push_back({4, {{1, 1}, {2, 1}, {2, 3}, {2, 3}, {4, 4}}});
    for (auto [n, es] : inputs)
    {
        int m = es.size();
        ebcc.init(n); g.clear();
        for (auto [u, v] : es) g.add(u, v);
        ebcc.build(g, n);

        // 暴力桥: 删第 i 条边后两端不连通
        VI exp_bridges;
        for (int i = 0; i < m; i++)
        {
            VI del(m, 0);
            del[i] = 1;
            VI lab = undirected_comp(n, es, 0, del);
            if (lab[es[i].first] != lab[es[i].second]) exp_bridges.push_back(i + 1);
        }
        VI ids = ebcc.get_bridges(g), want_ids;
        for (int b : exp_bridges) want_ids.push_back(2 * (b - 1));
        assert(ids == want_ids);
        // 半边编号和桥端点分别核对
        vector<PII> gotp, expp;
        for (int h : ebcc.get_bridges(g))
        {
            int a = g.edges[g.rev(h)].v, b = g.edges[h].v;
            gotp.push_back({min(a, b), max(a, b)});
        }
        for (int b : exp_bridges)
        {
            int a1 = es[b - 1].first, b1 = es[b - 1].second;
            expp.push_back({min(a1, b1), max(a1, b1)});
        }
        sort(gotp.begin(), gotp.end());
        sort(expp.begin(), expp.end());
        assert(gotp == expp);

        // 点划分: 去所有桥后的连通块
        VI del(m, 0);
        for (int b : exp_bridges) del[b - 1] = 1;
        VI lab_exp = undirected_comp(n, es, 0, del);
        VI got(n + 1, 0);
        for (int u = 1; u <= n; u++)
        {
            int mn = n + 1;
            for (int w = 1; w <= n; w++)
                if (ebcc.bel[w] == ebcc.bel[u]) mn = min(mn, w);
            got[u] = mn;
        }
        for (int u = 1; u <= n; u++) assert(got[u] == lab_exp[u]);

        // 桥树边数 == 桥数; 成员表 vs 连通块成员
        ebcc.build_tree(g);
        assert(ebcc.tree.edge_cnt() == (int)exp_bridges.size());
        multiset<PII> tree_expect, tree_got;
        for (int b : exp_bridges)
        {
            auto [u, v] = es[b - 1];
            int a = ebcc.bel[u], c = ebcc.bel[v];
            tree_expect.insert({min(a, c), max(a, c)});
        }
        for (size_t i = 0; i < ebcc.tree.edges.size(); i += 2)
        {
            int a = ebcc.tree.edges[i].v, c = ebcc.tree.edges[i ^ 1].v;
            tree_got.insert({min(a, c), max(a, c)});
        }
        assert(tree_got == tree_expect);
        map<int, VI> by_comp;
        for (int u = 1; u <= n; u++) by_comp[lab_exp[u]].push_back(u);
        vector<VI> expect_sets;
        for (auto& [k, vec] : by_comp) expect_sets.push_back(vec);
        vector<VI> got_sets(ebcc.ebcc_points.begin() + 1, ebcc.ebcc_points.end());
        assert(canonical_sets(got_sets) == canonical_sets(expect_sets));
    }
}

static void test_vbcc()
{
    static CheckedBlock vbcc(30);
    static Graph<false, Empty> g(30, 300);
    for (auto [n, es] : cases(false))
    {
        int m = es.size();
        vbcc.init(n); g.clear();
        for (auto [u, v] : es) g.add(u, v);
        vbcc.build(g, n);

        // 暴力割点: 删 v 后块数 > 原块数
        VI lab0 = undirected_comp(n, es, 0, VI(m, 0));
        VI cut_exp(n + 1, 0);
        for (int v = 1; v <= n; v++)
        {
            VI labv = undirected_comp(n, es, v, VI(m, 0));
            set<int> c0, cv;
            for (int w = 1; w <= n; w++)
            {
                if (w != v && labv[w]) cv.insert(labv[w]);
                if (lab0[w]) c0.insert(lab0[w]);
            }
            cut_exp[v] = (int)cv.size() > (int)c0.size();
        }
        for (int v = 1; v <= n; v++) assert(vbcc.cut[v] == cut_exp[v]);

        // 暴力点双: 枚举极大 2-点连通点集(诱导子图连通 且 删任一内部点仍连通)
        //           + 孤立点单点 VBCC, 与 vbcc_cir 比对
        auto induced_ok = [&](const VI& verts, int ban) -> bool
        {
            vector<char> in(n + 1, 0);
            for (int v : verts) in[v] = 1;
            int src = 0, total = 0;
            for (int v : verts)
            {
                if (v == ban) continue;
                total++;
                if (!src) src = v;
            }
            if (total <= 1) return true;
            vector<char> vis(n + 1, 0);
            VI sta{src};
            vis[src] = 1;
            int seen = 1;
            while (!sta.empty())
            {
                int u = sta.back();
                sta.pop_back();
                for (auto& [a, b] : es)
                {
                    int w = 0;
                    if (a == u && in[b] && b != ban) w = b;
                    if (b == u && in[a] && a != ban) w = a;
                    if (w && !vis[w])
                    {
                        vis[w] = 1;
                        seen++;
                        sta.push_back(w);
                    }
                }
            }
            return seen == total;
        };
        vector<VI> valid;
        for (int mask = 3; mask < (1 << n); mask++)
        {
            if (__builtin_popcount(mask) < 2) continue;
            VI verts;
            for (int v = 1; v <= n; v++)
                if (mask >> (v - 1) & 1) verts.push_back(v);
            if (!induced_ok(verts, 0)) continue;
            bool two_conn = true;
            for (int w : verts)
            {
                if (!induced_ok(verts, w))
                {
                    two_conn = false;
                    break;
                }
            }
            if (two_conn) valid.push_back(verts);
        }
        vector<VI> exp_sets;
        for (size_t i = 0; i < valid.size(); i++)
        {
            bool sub = false;
            for (size_t j = 0; j < valid.size() && !sub; j++)
            {
                if (i == j) continue;
                if (valid[j].size() > valid[i].size()
                    && includes(valid[j].begin(), valid[j].end(), valid[i].begin(), valid[i].end()))
                    sub = true;
            }
            if (!sub) exp_sets.push_back(valid[i]);
        }
        VI deg(n + 1, 0);
        for (auto& [a, b] : es)
        {
            deg[a]++;
            deg[b]++;
        }
        for (int v = 1; v <= n; v++)
            if (!deg[v]) exp_sets.push_back({v});
        vector<VI> got_sets(vbcc.vbcc_cir.begin() + 1, vbcc.vbcc_cir.end());
        assert(canonical_sets(got_sets) == canonical_sets(exp_sets));

        for (int root : {1, n})
        {
            CheckedBlock part(n);
            part.init(n); part.build(g, n, root);
            vector<VI> restricted;
            for (auto block : exp_sets) if (lab0[block[0]] == lab0[root]) restricted.push_back(block);
            vector<VI> actual(part.vbcc_cir.begin() + 1, part.vbcc_cir.end());
            assert(canonical_sets(actual) == canonical_sets(restricted));
            for (int u = 1; u <= n; u++)
            {
                assert(bool(part.dfn[u]) == (lab0[u] == lab0[root]));
                assert(part.cut[u] == (lab0[u] == lab0[root] ? cut_exp[u] : 0));
            }
        }
        assert(vbcc.get_cuts_vbcc(0).empty());
        assert(vbcc.get_cuts_vbcc(vbcc.vbcc_cnt + 1).empty());
        // 圆方树: 方点邻接 == 成员表; 边数 == Σ|S_i|; 反查接口
        vbcc.build_tree();
        int total_edges = 0;
        for (int i = 1; i <= vbcc.vbcc_cnt; i++) total_edges += (int)vbcc.vbcc_cir[i].size();
        assert(vbcc.tree.edge_cnt() == total_edges);
        for (int i = 1; i <= vbcc.vbcc_cnt; i++)
        {
            VI adjv;
            for (auto& e : vbcc.tree[n + i]) adjv.push_back(e.v);
            VI expect = vbcc.vbcc_cir[i];
            sort(adjv.begin(), adjv.end());
            sort(expect.begin(), expect.end());
            assert(adjv == expect);
        }
        for (int u = 1; u <= n; u++)
        {
            VI bels = vbcc.get_bel_vbccs(u);
            VI expect;
            for (int i = 1; i <= vbcc.vbcc_cnt; i++)
                for (int v : vbcc.vbcc_cir[i])
                {
                    if (v == u)
                    {
                        expect.push_back(i);
                        break;
                    }
                }
            sort(bels.begin(), bels.end());
            assert(bels == expect);
        }
        for (int i = 1; i <= vbcc.vbcc_cnt; i++)
        {
            VI cuts = vbcc.get_cuts_vbcc(i);
            VI expect;
            for (int v : vbcc.vbcc_cir[i])
                if (cut_exp[v]) expect.push_back(v);
            assert(cuts == expect);
        }
    }
}


// 构造容量取实际峰值；链验证每个点/桥/块，星和空图检查极端分量数。
static void test_capacity()
{
    const int N = 2000;
    SCC scc(N, N);
    EBCC ebcc(N);
    CheckedBlock vbcc(N);
    Graph<true> dg(N, N);
    Graph<false, LL> ug(N, N); // 无向两件也接带权图，只忽略权值
    auto caps = make_tuple(dg.edges.capacity(), ug.edges.capacity(), scc.dag.edges.capacity(), ebcc.tree.edges.capacity(), vbcc.tree.edges.capacity());
    for (int n : {N, 1, N, 2, N})
        for (int kind = 0; kind < 4; kind++)
        {
            scc.init(n); ebcc.init(n); vbcc.init(n); dg.clear(); ug.clear();
            if (kind != 0)
                for (int u = 2; u <= n; u++)
                {
                    int v = kind == 2 ? 1 : u - 1;
                    dg.add(u, v); ug.add(u, v);
                }
            if (kind == 3 && n > 1) { dg.add(1, n); ug.add(1, n); }
            scc.build(dg, n); scc.build_dag(dg);
            ebcc.build(ug, n); ebcc.build_tree(ug);
            vbcc.build(ug, n); vbcc.build_tree();
            assert(caps == make_tuple(dg.edges.capacity(), ug.edges.capacity(), scc.dag.edges.capacity(), ebcc.tree.edges.capacity(), vbcc.tree.edges.capacity()));
            bool cycle = kind == 3 && n > 1;
            assert(scc.scc_cnt == (cycle ? 1 : n));
            assert(scc.dag.edge_cnt() == (cycle || kind == 0 ? 0 : n - 1));
            assert(ebcc.ebcc_points.size() == size_t(ebcc.ebcc_cnt + 1));
            assert(ebcc.ebcc_cnt == (cycle ? 1 : n));
            assert(ebcc.get_bridges(ug).size() == size_t(cycle || kind == 0 ? 0 : n - 1));
            assert(ebcc.tree.edge_cnt() == (cycle || kind == 0 ? 0 : n - 1));
            int blocks = kind == 0 ? n : (cycle || n == 1 ? 1 : n - 1);
            assert(vbcc.vbcc_cnt == blocks);
            assert(vbcc.tree.edge_cnt() == (kind == 0 || cycle || n == 1 ? n : 2 * (n - 1)));
            for (int u = 1; u <= n; u++)
            {
                int cut = 0;
                if (kind == 1) cut = u > 1 && u < n;
                if (kind == 2) cut = u == 1 && n > 2;
                assert(vbcc.cut[u] == cut);
                assert(scc.bel[u] >= 1 && scc.bel[u] <= scc.scc_cnt);
                assert(ebcc.bel[u] >= 1 && ebcc.bel[u] <= ebcc.ebcc_cnt);
                assert(vbcc.get_bel_vbccs(u).size() == size_t(kind == 0 || cycle || n == 1 ? 1 : (kind == 2 ? (u == 1 ? n - 1 : 1) : (cut ? 2 : 1))));
            }
        }
    // 点边预算均用满的稠密图；SCC 的 DAG 容量也恰好容纳全部跨分量边。
    const int K = 20, M = K * (K - 1) / 2;
    SCC full(K, M); Graph<true> g(K, M);
    full.init(K);
    for (int u = 1; u <= K; u++) for (int v = u + 1; v <= K; v++) g.add(u, v);
    full.build(g, K); full.build_dag(g);
    assert(full.scc_cnt == K && full.dag.edge_cnt() == M);
    // 暴露实际 ABI 的容量账目，方便复核注释中的字节估算。
    cout << "sizeof(int)=" << sizeof(int) << ", vector=" << sizeof(VI)
         << ", unweighted-edge=" << sizeof(Graph<false>::Edge) << "\n";
}

static void test_coexist()
{
    Graph<false> g(3, 2);
    g.add(1, 2); g.add(2, 3);
    VBCC vbcc(3);
    BCT bct(3);
    vbcc.init(3); bct.init(3);
    vbcc.build(g, 3); bct.build(g, 3);
    vbcc.build_tree(); bct.build_tree();
    assert(vbcc.cut[2] && bct.cut[2]);
    assert(vbcc.tree.edge_cnt() == 4 && bct.tree.edge_cnt() == 4);
    bct.init(1);
    assert(vbcc.vbcc_cnt == 2 && vbcc.cut[2]);
    assert(bct.vbcc_cnt == 0 && bct.tree.edge_cnt() == 0);
}

int main()
{
    test_coexist();
    test_scc();
    test_ebcc();
    test_vbcc();
    test_capacity();
    cout << "cases: directed=" << cases(true).size() << ", undirected=" << cases(false).size() << "\n";
#ifdef CONN_BCT_ONLY
    cout << "BCT.cpp implementation passed\n";
#else
    cout << "Tarjan_VBCC.cpp implementation passed\n";
#endif
    return 0;
}