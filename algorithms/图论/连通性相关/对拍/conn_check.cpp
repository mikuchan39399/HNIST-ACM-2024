// ============ conn_check 连通性四件套回归 ============
// 覆盖: SCC(划分 vs Warshall 传递闭包, 去重缩点 DAG 边集) | EBCC(桥 vs 逐边
//       删边判连通, 点划分 vs 去桥后连通块, 桥树边数, 成员表) | VBCC/BCT(割点
//       vs 逐点删除断连, 点双成员表 vs 子集枚举法, 圆方树邻接/边数,
//       get_bel_vbccs/get_cuts_vbcc); BCT 同场共编即幂等证明
// 纪律: 改动 Tarjan_SCC/EBCC/VBCC/BCT/Graph, 必重跑本套件
// 跑法: g++ -std=c++20 -Wall -Wextra -O2 conn_check.cpp -o conn_check && ./conn_check
#include <iostream>
#include <vector>
#include <cassert>
#include <algorithm>
#include <set>
#include <map>
#include <random>
#include "../Tarjan_SCC.cpp"
#include "../Tarjan_EBCC.cpp"
#include "../Tarjan_VBCC.cpp"
#include "../圆方树/BCT.cpp"

using namespace std;

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

static void test_scc(mt19937& rng)
{
    static SCC scc(30, 100);
    for (int tc = 0; tc < 300; tc++)
    {
        int n = 1 + rng() % 10;
        int m = rng() % 16;
        VPII es;
        scc.init(n);
        for (int i = 0; i < m; i++)
        {
            int u = 1 + rng() % n, v = 1 + rng() % n;   // 允许自环
            scc.add_edge(u, v);
            es.push_back({u, v});
        }
        scc.build();

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

        // 去重缩点 DAG 边集
        scc.build_dag_unique();
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

static void test_ebcc(mt19937& rng)
{
    static EBCC ebcc(30, 100);
    for (int tc = 0; tc < 300; tc++)
    {
        int n = 1 + rng() % 10;
        int m = rng() % 16;
        VPII es;                                       // u != v, 允许重边
        ebcc.init(n);
        for (int i = 0; i < m; i++)
        {
            int u = 1 + rng() % n, v = 1 + rng() % n;
            if (u == v) v = u % n + 1;
            ebcc.add_edge(u, v, i + 1);
            es.push_back({u, v});
        }
        ebcc.build();

        // 暴力桥: 删第 i 条边后两端不连通
        VI exp_bridges;
        for (int i = 0; i < m; i++)
        {
            VI del(m, 0);
            del[i] = 1;
            VI lab = undirected_comp(n, es, 0, del);
            if (lab[es[i].first] != lab[es[i].second]) exp_bridges.push_back(i + 1);
        }
        VI got_bridges = ebcc.get_bridges();
        sort(exp_bridges.begin(), exp_bridges.end());
        sort(got_bridges.begin(), got_bridges.end());
        assert(got_bridges == exp_bridges);

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
        ebcc.build_tree();
        assert(ebcc.tree.edge_cnt() == (int)exp_bridges.size());
        map<int, VI> by_comp;
        for (int u = 1; u <= n; u++) by_comp[lab_exp[u]].push_back(u);
        vector<VI> expect_sets;
        for (auto& [k, vec] : by_comp) expect_sets.push_back(vec);
        vector<VI> got_sets(ebcc.ebcc_points.begin() + 1, ebcc.ebcc_points.end());
        assert(canonical_sets(got_sets) == canonical_sets(expect_sets));
    }
}

static void test_vbcc(mt19937& rng)
{
    static VBCC vbcc(30, 100);
    for (int tc = 0; tc < 300; tc++)
    {
        int n = 1 + rng() % 10;
        int m = rng() % 16;
        VPII es;                                       // u != v, 允许重边(契约: 自环不入图)
        vbcc.init(n);
        for (int i = 0; i < m; i++)
        {
            int u = 1 + rng() % n, v = 1 + rng() % n;
            if (n > 1)
            {
                if (u == v) v = u % n + 1;
                vbcc.add_edge(u, v);
                es.push_back({u, v});
            }
        }
        vbcc.build();

        // 暴力割点: 删 v 后块数 > 原块数 - 1
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

int main()
{
    mt19937 rng(42);
    test_scc(rng);
    test_ebcc(rng);
    test_vbcc(rng);
    cout << "conn_check passed: SCC / EBCC / VBCC(BCT) all tests ok\n";
    return 0;
}