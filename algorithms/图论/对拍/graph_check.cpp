// ============ 图论家族回归套件 ============
// 覆盖: LCA×2(DFN_LCA/HLD_LCA) | 拓扑排序 | 直径×2(两次DFS/树形dp) | 重心
//       | 最短路×4(dij/dijN/spfa/bf, 含负边与多源) | 判负环×2(bfRing/spfaRing)
//       | Graph 赋值(拷贝/移动深独立, 自赋值, nothrow 契约)
// 纪律: 改动上述任一模板或 Graph 母版, 必重跑本套件
// 跑法: g++ -std=c++20 -Wall -Wextra -O2 graph_check.cpp -o graph_check && ./graph_check
#include <algorithm>
#include <cassert>
#include <functional>
#include <iostream>
#include <limits>
#include <random>
#include <set>
#include <utility>
#include <vector>
#include "../图的存储/Graph.cpp"
#include "../拓扑排序/拓扑排序.cpp"
#include "../树上问题/最近公共祖先/HLD_LCA.cpp"
#include "../树上问题/最近公共祖先/DFN_LCA.cpp"
#include "../树上问题/树的直径/两次dfs.cpp"
#include "../树上问题/树的直径/树形dp法.cpp"
#include "../树上问题/树的重心/树的重心.cpp"
#include "../最短路问题/单源最短路径/dijkstra_heap.cpp"
#include "../最短路问题/单源最短路径/dijkstra.cpp"
#include "../最短路问题/单源最短路径/spfa 存图.cpp"
#include "../最短路问题/单源最短路径/Bellman-Ford存图.cpp"
#include "../最短路问题/判断负环/bf_-ring.cpp"
#include "../最短路问题/判断负环/spfa_-ring.cpp"
using namespace std;
using LL = long long;
using VI = vector<int>;
using PII = pair<int, int>;

// ============ 段 1: LCA 双引擎 vs 爬父暴力 ============

// 独立暴力: 显式父表爬链
struct BruteLca
{
    int n;
    VI par, dep, root;
    BruteLca(int _n) : n(_n), par(_n + 10, 0), dep(_n + 10, 0), root(_n + 10, 0) {}
    void add(int v, int p) // p == 0 表示 v 为根
    {
        par[v] = p;
        root[v] = p ? root[p] : v;
        dep[v] = p ? dep[p] + 1 : 1;
    }
    int lca(int u, int v) const
    {
        if (root[u] != root[v]) return -1;
        while (dep[u] > dep[v]) u = par[u];
        while (dep[v] > dep[u]) v = par[v];
        while (u != v) { u = par[u]; v = par[v]; }
        return u;
    }
    int climb(int u, int k) const
    {
        while (k--) u = par[u];
        return u;
    }
    int sub_sz(int u) const
    {
        int s = 1;
        for (int v = 1; v <= n; v++)
            if (par[v] == u) s += sub_sz(v);
        return s;
    }
    LL dist(int u, int v) const
    {
        int l = lca(u, v);
        if (l == -1) return -1;
        return (LL)dep[u] + dep[v] - 2 * dep[l];
    }
    int jump(int u, int v, int k) const
    {
        if (k <= 0) return u;
        int l = lca(u, v);
        if (l == -1) return -1;
        int du = dep[u] - dep[l], dv = dep[v] - dep[l];
        if (du + dv < k) return v;
        if (k <= du) return climb(u, k);
        return climb(v, du + dv - k);
    }
};

void test_lca_engines()
{
    mt19937 rng(42);
    static Graph<false> g(60, 60);
    static HLD_LCA hl(60);    // static + init 复用, 覆盖多测路径
    static LCA dfn(60);
    for (int tc = 0; tc < 300; tc++)
    {
        int n = 1 + rng() % 50;
        g.clear();
        BruteLca br(n);
        hl.init(n);
        dfn.init(n);
        for (int v = 1; v <= n; v++)
        {
            int p = 0;
            if (v > 1 && rng() % 100 >= 15) p = 1 + rng() % (v - 1); // 15% 成根, 制造森林
            br.add(v, p);
            if (p) g.add(p, v);
        }
        hl.build(g);
        dfn.build(g);
        for (int u = 1; u <= n; u++) assert(dfn.sz[u] == br.sub_sz(u));
        for (int t = 0; t < 40; t++)
        {
            int u = 1 + rng() % n, v = 1 + rng() % n;
            int bl = br.lca(u, v);
            assert(hl.lca(u, v) == bl);
            assert(dfn.lca(u, v) == bl);
            assert(hl.dist(u, v) == br.dist(u, v));
            assert(dfn.dist(u, v) == br.dist(u, v));
            int k = (int)(rng() % (2 * n + 4)) - 2; // 覆盖 k<=0 / 恰好 / 超路长
            assert(dfn.jump(u, v, k) == br.jump(u, v, k));
        }
        int cnt = 1 + rng() % 6;
        VI nodes;
        for (int j = 0; j < cnt; j++) nodes.push_back(1 + rng() % n);
        int bm = br.lca(nodes[0], nodes[1 % cnt]);
        for (int j = 1; j < cnt && bm != -1; j++) bm = br.lca(bm, nodes[j]);
        assert(hl.lca(nodes) == bm);
        assert(dfn.lca(nodes) == bm);
    }
}
// ============ 段 2: 拓扑排序 vs 三色 DFS 判环 ============

// 独立暴力: 三色 DFS 判有环
bool brute_has_cycle(int n, const vector<PII>& es)
{
    vector<VI> adj(n + 1);
    for (auto& [u, v] : es) adj[u].push_back(v);
    VI color(n + 1, 0); // 0 = 白 1 = 灰 2 = 黑
    function<bool(int)> dfs = [&](int u) -> bool
    {
        color[u] = 1;
        for (int v : adj[u])
        {
            if (color[v] == 1) return true;
            if (color[v] == 0 && dfs(v)) return true;
        }
        color[u] = 2;
        return false;
    };
    for (int u = 1; u <= n; u++)
        if (color[u] == 0 && dfs(u)) return true;
    return false;
}

void test_topo_sort()
{
    mt19937 rng(42);
    static Graph<true> g(60, 240);      // static + clear 复用, 覆盖多测路径
    static TopoSort ts;
    for (int tc = 0; tc < 300; tc++)
    {
        int n = 1 + rng() % 40;
        int m = rng() % 121;
        g.clear();
        vector<PII> es(m);
        for (int i = 0; i < m; i++)
        {
            int u = 1 + rng() % n, v = 1 + rng() % n;
            es[i] = {u, v};
            g.add(u, v);
        }
        VI in_backup = g.in_deg;
        bool dag = ts.build(g, n);
        assert(g.in_deg == in_backup);  // 契约: 不动原图
        assert(dag == !brute_has_cycle(n, es));
        if (dag)
        {
            VI& ord = ts.get();
            assert((int)ord.size() == n);
            VI pos(n + 1, 0);
            for (int i = 0; i < n; i++)
            {
                assert(ord[i] >= 1 && ord[i] <= n);
                assert(pos[ord[i]] == 0);  // 每点恰一次
                pos[ord[i]] = i + 1;
            }
            for (auto& [u, v] : es)
                assert(pos[u] < pos[v]);   // 边方向与序一致
        }
        else
        {
            assert((int)ts.get().size() < n);  // 有环时收不满
        }
    }
}
// ============ 段 3: 直径×2 vs 枚举起点, 重心 vs 逐点删除连通块 ============

// 独立暴力: 扫描父表找邻边, 对每个起点或删点单独遍历, 总计 O(n^3)
struct BruteTree
{
    int n;
    VI par;
    vector<LL> pw, pt; // 父边权 / 点权
    BruteTree(int _n) : n(_n), par(_n + 10, 0), pw(_n + 10, 0), pt(_n + 10, 1) {}
    LL max_dist(int u, int p, LL d) const
    {
        LL mx = d;
        for (int v = 1; v <= n; v++)
            if (v != p && (par[v] == u || par[u] == v))
                mx = max(mx, max_dist(v, u, d + (par[v] == u ? pw[v] : pw[u])));
        return mx;
    }
    LL diameter() const
    {
        LL ans = 0;
        for (int u = 1; u <= n; u++) ans = max(ans, max_dist(u, 0, 0));
        return ans;
    }
    LL max_part(int x) const
    {
        if (n == 1) return 0;
        VI seen(n + 1);
        seen[x] = 1;
        LL mx = numeric_limits<LL>::min();
        for (int start = 1; start <= n; start++)
        {
            if (seen[start]) continue;
            VI q{start};
            seen[start] = 1;
            LL sum = 0;
            for (size_t i = 0; i < q.size(); i++)
            {
                int u = q[i];
                sum += pt[u];
                for (int v = 1; v <= n; v++)
                    if (!seen[v] && (par[v] == u || par[u] == v))
                    {
                        seen[v] = 1;
                        q.push_back(v);
                    }
            }
            mx = max(mx, sum);
        }
        return mx;
    }
    pair<VI, LL> centroids() const
    {
        VI res;
        LL best = numeric_limits<LL>::max();
        for (int x = 1; x <= n; x++)
        {
            LL mp = max_part(x);
            if (mp < best) best = mp, res = {x};
            else if (mp == best) res.push_back(x);
        }
        return {res, best};
    }
};

void test_tree_basic()
{
    mt19937 rng(42);
    static Graph<false> gu(60, 60);
    static Graph<false, LL> gw(60, 60);
    static TreeDiameter<Graph<false>> du;
    static TreeDiameter<Graph<false, LL>> dw;
    static TreeDiameterDP<Graph<false>> pu;
    static TreeDiameterDP<Graph<false, LL>> pw;
    static TreeCentroid<Graph<false>> tc(60);
    for (int tcas = 0; tcas < 300; tcas++)
    {
        int n = 1 + rng() % 50;
        BruteTree bt(n);
        gu.clear();
        gw.clear();
        bool nonneg = tcas % 3 != 1; // 三分之一用例带负边权, 只验 dp 版
        for (int v = 2; v <= n; v++)
        {
            int p = 1 + rng() % (v - 1);
            LL w = nonneg ? (LL)(rng() % 7) : (LL)(rng() % 11) - 5;
            bt.par[v] = p;
            bt.pw[v] = w;
            gu.add(p, v);
            gw.add(p, v, w);
        }
        for (int i = 1; i <= n; i++) bt.pt[i] = (int)(rng() % 9) - 2; // 含零/负点权; 先窄化再减, 防无符号下溢(Linux uint_fast32_t 8B 时 -2 变 1.8e19 溢出 LL)

        assert(pw.build(gw, n) == bt.diameter());
        if (nonneg)
        {
            dw.build(gw, n);
            assert(dw.len == bt.diameter());
            assert((int)dw.path.size() >= 1);
            assert(dw.path.front() == dw.end_u && dw.path.back() == dw.end_v);
            LL acc = 0;
            for (size_t i = 1; i < dw.path.size(); i++)
            {
                int a = dw.path[i - 1], b = dw.path[i];
                assert(bt.par[a] == b || bt.par[b] == a); // 相邻
                acc += bt.par[b] == a ? bt.pw[b] : bt.pw[a];
            }
            assert(acc == dw.len); // 路径权和 = 直径
            du.build(gu, n);
            BruteTree bu = bt;
            for (int i = 1; i <= n + 9; i++) bu.pw[i] = 1;
            assert(du.len == bu.diameter());
            assert(pu.build(gu, n) == bu.diameter());
        }

        tc.init(n); // 多测复位点权
        for (int i = 1; i <= n; i++) tc.pt[i] = bt.pt[i];
        tc.build(gu, n);
        auto [bc, bmp] = bt.centroids();
        assert(tc.centroids == bc);
        assert(tc.min_max_part == bmp);
    }
}

// 负点权下根没有父侧空块; 零点权下答案可以超过两个
void test_centroid_boundaries()
{
    Graph<false> g(6, 5);
    TreeCentroid<Graph<false>> tc(6);
    g.add(1, 2);
    tc.init(2);
    tc.pt[1] = tc.pt[2] = -1;
    tc.build(g, 2);
    assert(tc.centroids == VI({1, 2}) && tc.min_max_part == -1);

    g.clear();
    tc.init(1);
    tc.pt[1] = -7;
    tc.build(g, 1);
    assert(tc.centroids == VI({1}) && tc.min_max_part == 0);

    for (int u = 2; u <= 6; u++) g.add(u - 1, u);
    tc.init(6);
    for (int u = 1; u <= 6; u++) tc.pt[u] = 0;
    tc.build(g, 6);
    assert(tc.centroids == VI({1, 2, 3, 4, 5, 6}) && tc.min_max_part == 0);

    tc.init(6);
    tc.build(g, 6);
    assert(tc.centroids == VI({3, 4}) && tc.min_max_part == 3);
}

// ============ 段 5: Graph 本体 vs 逐边账本 ============
static void test_graph_core()
{
    mt19937 rng(777);
    for (int tc = 0; tc < 300; tc++)
    {
        int n = 1 + rng() % 10;
        int m = rng() % 16;
        Graph<true, Empty> dg(n, 40);
        Graph<false, Empty> ug(n, 40);
        vector<PII> de, ue;
        VI din(n + 1, 0), dout(n + 1, 0), udeg(n + 1, 0);
        for (int i = 0; i < m; i++)
        {
            int u = 1 + rng() % n, v = 1 + rng() % n;   // 允许自环/重边
            dg.add(u, v);
            de.push_back({u, v});
            dout[u]++;
            din[v]++;
            ug.add(u, v);
            ue.push_back({u, v});
            udeg[u]++;
            udeg[v]++;
        }
        assert(dg.edge_cnt() == (int)de.size());
        assert(ug.edge_cnt() == (int)ue.size());
        set<int> dtouch, utouch;
        for (auto& [u, v] : de)
        {
            dtouch.insert(u);
            dtouch.insert(v);
        }
        for (auto& [u, v] : ue)
        {
            utouch.insert(u);
            utouch.insert(v);
        }
        assert(dg.node_cnt() == (int)dtouch.size());
        assert(ug.node_cnt() == (int)utouch.size());
        for (int v = 1; v <= n; v++)
        {
            assert(dg.deg[v] == dout[v]);
            assert(dg.in_deg[v] == din[v]);
            assert(ug.deg[v] == udeg[v]);
        }
        // 邻接迭代 vs 账本 (无向图含双向半边)
        vector<PII> dgot, ugot, uexp;
        for (int v = 1; v <= n; v++)
        {
            for (auto& e : dg[v]) dgot.push_back({v, e.v});
            for (auto& e : ug[v]) ugot.push_back({v, e.v});
        }
        for (auto& [u, v] : ue)
        {
            uexp.push_back({u, v});
            uexp.push_back({v, u});
        }
        sort(de.begin(), de.end());
        sort(dgot.begin(), dgot.end());
        assert(dgot == de);
        sort(ue.begin(), ue.end());
        sort(ugot.begin(), ugot.end());
        sort(uexp.begin(), uexp.end());
        assert(ugot == uexp);
        // clear 复用: 账本清零后重建
        dg.clear();
        ug.clear();
        assert(dg.edge_cnt() == 0 && ug.edge_cnt() == 0 && dg.node_cnt() == 0);
        int v = min(n, 2); // n = 1 时用自环, 不借状态表余量访问点 2
        dg.add(1, v);
        ug.add(1, v);
        assert(dg.edge_cnt() == 1 && ug.edge_cnt() == 1);
        assert(dg.deg[1] == 1 && dg.in_deg[v] == 1);
        assert(ug.deg[1] == (v == 1 ? 2 : 1));
        if (v != 1) assert(ug.deg[v] == 1);
    }
}

// ============ 段 5: 最短路家族 ============
// 域 1 非负权: dij/dijN/spfa/bf 四引擎 dist 互拍 + 多源对拍双单源取 min
// 域 2 负边 DAG(无环必无负环): spfa/bf 互拍, 判环双引擎必 false
// 域 3 随机混合权: bfRing/spfaRing 互拍 | 定向: 埋负环必中, 无负环必过
static void test_shortest_path()
{
    mt19937 rng(424242);
    static Dijkstra dij{61};
    static DijkstraN dijn{61};
    static SPFA sp{61};
    static BellmanFord bf{61};
    static BFRing bfr{61};
    static SPFARing spr{61};
    for (int tc = 0; tc < 200; tc++)   // 域 1: 非负权四引擎互拍 + 多源
    {
        int n = 1 + rng() % 40, m = rng() % 120;
        Graph<true, LL> g{n, m};
        for (int i = 0; i < m; i++)
            g.add(1 + rng() % n, 1 + rng() % n, rng() % 21);
        int s = 1 + rng() % n, s2 = 1 + rng() % n;
        dij.init(n); dijn.init(n); sp.init(n); bf.init(n);
        dij.run(s, g);
        dijn.run(s, g);
        sp.run(s, g);
        bf.run(s, g);
        for (int i = 1; i <= n; i++)
            assert(dij.dist[i] == dijn.dist[i] && dij.dist[i] == sp.dist[i]
                   && dij.dist[i] == bf.dist[i]);
        VLL d1 = dij.dist;
        VLL d2 = [&]{ dij.init(n); dij.run(s2, g); return dij.dist; }();
        dij.init(n);
        dij.run(VI{s, s2}, g);
        for (int i = 1; i <= n; i++)
            assert(dij.dist[i] == min(d1[i], d2[i]));
    }
    for (int tc = 0; tc < 200; tc++)   // 域 2: 负边 DAG
    {
        int n = 1 + rng() % 40, m = rng() % 100;
        Graph<true, LL> g{n, m};
        for (int i = 0; i < m; i++)
        {
            int u = 1 + rng() % n, v = 1 + rng() % n;
            if (u == v) continue;
            if (u > v) swap(u, v);   // 只加 u<v, 图必为 DAG
            g.add(u, v, (LL)(rng() % 41) - 20);
        }
        int s = 1 + rng() % n;
        sp.init(n); bf.init(n);
        sp.run(s, g); bf.run(s, g);
        for (int i = 1; i <= n; i++) assert(sp.dist[i] == bf.dist[i]);
        bfr.init(n); spr.init(n);
        assert(!bfr.run(g) && !spr.run(g));
    }
    for (int tc = 0; tc < 200; tc++)   // 域 3: 混合权判环互拍
    {
        int n = 2 + rng() % 20, m = rng() % 60;
        Graph<true, LL> g{n, m};
        for (int i = 0; i < m; i++)
            g.add(1 + rng() % n, 1 + rng() % n, (LL)(rng() % 41) - 20);
        bfr.init(n); spr.init(n);
        assert(bfr.run(g) == spr.run(g));
    }
    {
        Graph<true, LL> g{4, 4};    // 定向: 环 1->2->3->1 总权 -3
        g.add(1, 2, 1); g.add(2, 3, -5); g.add(3, 1, 1); g.add(3, 4, 2);
        bfr.init(4); spr.init(4);
        assert(bfr.run(g) && spr.run(g));
    }
    {
        Graph<true, LL> g{4, 4};    // 定向: 正环 + 悬挂负边, 无负环
        g.add(1, 2, 5); g.add(2, 3, -1); g.add(1, 3, 3); g.add(3, 4, 0);
        bfr.init(4); spr.init(4);
        assert(!bfr.run(g) && !spr.run(g));
    }
}

// ============ 段 7: Graph 赋值语义 ============

// 整图邻接快照(每点按链序的 v 序列), 供赋值前后比对
template <class G>
static VVI snap(G& g, int n)
{
    VVI s(n + 1);
    for (int u = 1; u <= n; u++)
        for (auto& e : g[u]) s[u].push_back(e.v);
    return s;
}

void test_graph_assign()
{
    // 隐式拷贝/移动赋值是鸭子化的前置契约(算法器会拷贝/移动整图),
    // nothrow 移动赋值编译期钉死
    static_assert(is_nothrow_move_assignable_v<Graph<true>>);
    static_assert(is_nothrow_move_assignable_v<Graph<false, LL>>);
    mt19937 rng(42);
    for (int tc = 0; tc < 300; tc++)
    {
        int n = 1 + rng() % 40, m = rng() % 80;
        Graph<true> a1(n, m);         // 有向 Empty (SCC 形态)
        Graph<true, LL> a2(n, m);     // 有向 LL (SegGraph 形态)
        Graph<false, LL> a3(n, m);    // 无向 LL (带权连通性形态)
        for (int i = 0; i < m; i++)
        {
            int u = 1 + rng() % n, v = 1 + rng() % n;
            LL w = (LL)(rng() % 100);
            a1.add(u, v); a2.add(u, v, w); a3.add(u, v, w);
        }
        VVI s1 = snap(a1, n), s2 = snap(a2, n), s3 = snap(a3, n);
        // 拷贝赋值: 结构逐点等价
        Graph<true> b1; b1 = a1;
        Graph<true, LL> b2; b2 = a2;
        Graph<false, LL> b3; b3 = a3;
        assert(snap(b1, n) == s1);
        assert(snap(b2, n) == s2);
        assert(snap(b3, n) == s3);
        // 深独立: 副本加边/清空, 母本快照纹丝不动
        b1.add(1, 1); b2.add(1, 1, 7); b3.add(1, 1, 7);
        b1.clear(); b2.clear(); b3.clear();
        assert(snap(a1, n) == s1);
        assert(snap(a2, n) == s2);
        assert(snap(a3, n) == s3);
        // 移动赋值: 接管结构(母本进入有效未定态, 不再断言其内容)
        Graph<true> c1; c1 = move(a1);
        Graph<true, LL> c2; c2 = move(a2);
        assert(snap(c1, n) == s1);
        assert(snap(c2, n) == s2);
        assert(c1.edge_cnt() == m && c2.edge_cnt() == m);
        // 拷贝自赋值(经别名): 无操作不损坏
        auto& r1 = c1; c1 = r1;
        assert(snap(c1, n) == s1);
    }
}

int main()
{
    test_lca_engines();
    test_topo_sort();
    test_tree_basic();
    test_centroid_boundaries();
    test_graph_core();
    test_graph_assign();
    test_shortest_path();
    cout << "All tests passed flawlessly!\n";
    return 0;
}
