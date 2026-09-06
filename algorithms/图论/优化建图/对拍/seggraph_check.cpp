// ============ seggraph_check 线段树优化建图 回归套件 ============
// 保留 300 组旧随机测试, 另用 400 组独立 Floyd、Empty 可达性及 SCC 组装核对六类接口
// 默认执行 20 万点/操作、非对齐区间及少量原点/20 万中继, 大小交替重建
// 负权 DAG 使用独立松弛及闭式答案, 不把它传给 Dijkstra
// 纪律: 改动 线段树优化建图 / Dijkstra / Graph 模板, 必重跑本套件
// 跑法: g++ -std=c++20 -Wall -Wextra -O2 seggraph_check.cpp -o seggraph_check && ./seggraph_check
#include <iostream>
#include <vector>
#include <cassert>
#include <random>
#include <algorithm>
#include "../线段树优化建图.cpp"
#include "../../最短路问题/单源最短路径/dijkstra_heap.cpp"
#include "../../连通性相关/Tarjan_SCC.cpp"

using namespace std;
using LL = long long;

void test_seg_graph()
{
    mt19937 rng(786786);
    static SegGraph<LL> sg{61, 4000};
    static SegGraph<Empty> se{61, 4000};   // 无权实例(偏序用法), 同拓扑互拍
    static Dijkstra d1{254};
    static Dijkstra d2{41};
    static Graph<true, LL> gn{41, 200000};
    for (int tc = 0; tc < 300; tc++)
    {
        int n = 1 + rng() % 40, q = rng() % 60;
        gn.clear();
        sg.build(n);
        se.build(n);
        for (int i = 0; i < q; i++)
        {
            int l = 1 + rng() % n, r = 1 + rng() % n;
            if (l > r) swap(l, r);
            int u = 1 + rng() % n, v = 1 + rng() % n;
            LL w = rng() % 21;
            int op = rng() % 6;
            if (op == 0)
            {
                sg.add_p2p(u, v, w);
                se.add_p2p(u, v);
                gn.add(u, v, w);
            }
            else if (op == 1)
            {
                sg.add_p2r(u, l, r, w);
                se.add_p2r(u, l, r);
                for (int k = l; k <= r; k++) gn.add(u, k, w);
            }
            else if (op == 2)
            {
                sg.add_r2p(l, r, v, w);
                se.add_r2p(l, r, v);
                for (int k = l; k <= r; k++) gn.add(k, v, w);
            }
            else if (op == 3)
            {
                int l2 = 1 + rng() % n, r2 = 1 + rng() % n;
                if (l2 > r2) swap(l2, r2);
                sg.add_r2r(l, r, l2, r2, w);
                se.add_r2r(l, r, l2, r2);
                for (int i = l; i <= r; i++)
                    for (int j = l2; j <= r2; j++) gn.add(i, j, w);
            }
            else if (op == 4)   // 手动中继: r2new 收编(权 w) + 虚点当源 p2r 分发(权 w2), 异权链
            {
                int l2 = 1 + rng() % n, r2 = 1 + rng() % n;
                if (l2 > r2) swap(l2, r2);
                LL w2 = rng() % 21;
                int vp = sg.add_r2new(l, r, w);
                int vp2 = se.add_r2new(l, r);
                assert(vp > n && vp == vp2);
                sg.add_p2r(vp, l2, r2, w2);
                se.add_p2r(vp, l2, r2);
                for (int i = l; i <= r; i++)
                    for (int j = l2; j <= r2; j++) gn.add(i, j, w + w2);
            }
            else   // 单点起链: p2new 收进 u(权 w) + 虚点当源 p2r 分发(权 w2)
            {
                int l2 = 1 + rng() % n, r2 = 1 + rng() % n;
                if (l2 > r2) swap(l2, r2);
                LL w2 = rng() % 21;
                int vp = sg.add_p2new(u, w);
                int vp2 = se.add_p2new(u);
                assert(vp > n && vp == vp2);
                sg.add_p2r(vp, l2, r2, w2);
                se.add_p2r(vp, l2, r2);
                for (int j = l2; j <= r2; j++) gn.add(u, j, w + w2);
            }
        }
        assert(se.tot == sg.tot && se.g.edge_cnt() == sg.g.edge_cnt());   // Empty 与 LL 同拓扑
        int s = 1 + rng() % n;
        d1.init(sg.tot);
        d1.run(s, sg.g);
        d2.init(n);
        d2.run(s, gn);
        for (int i = 1; i <= n; i++)
            assert(d1.dist[i] == d2.dist[i]);   // 原点最短路逐点互拍
    }
}

// 独立矩阵直接展开原点区间, 用户虚点单独编号, 不包含两棵线段树
void test_independent_small()
{
    mt19937 rng(42);
    SegGraph<LL> sg(64, 0);
    SegGraph<Empty> se(64, 0);
    Dijkstra dij(256);
    SCC scc(256);
    for (int tc = 0; tc < 400; tc++)
    {
        int n = 1 + rng() % 16, q = tc < 16 ? 0 : 32;
        sg.build(n); se.build(n);
        assert(sg.tot == 3 * n - 2 && sg.g.edge_cnt() == 4 * n - 4);
        VI id(n + 1); iota(id.begin(), id.end(), 0);
        vector<VLL> d(n + q + 1, VLL(n + q + 1, INF));
        for (int i = 1; i <= n + q; i++) d[i][i] = 0;
        auto edge = [&](int u, int v, LL w) { d[u][v] = min(d[u][v], w); };
        int extra = 0;
        for (int op = 0; op < q; op++)
        {
            int u = 1 + rng() % (id.size() - 1), v = 1 + rng() % (id.size() - 1);
            int l = 1 + rng() % n, r = 1 + rng() % n;
            int a = 1 + rng() % n, b = 1 + rng() % n;
            if (l > r) swap(l, r);
            if (a > b) swap(a, b);
            if (op % 7 == 0) l = 1, r = n;
            if (op % 7 == 1) r = l;
            LL w = op % 3 == 0 ? 0 : op % 3 == 1 ? rng() % 20 : 1000000000000LL;
            int kind = op < 6 ? op : rng() % 6;
            if (kind == 0) { sg.add_p2p(id[u], id[v], w); se.add_p2p(id[u], id[v]); edge(u, v, w); }
            if (kind == 1)
            {
                sg.add_p2r(id[u], l, r, w); se.add_p2r(id[u], l, r);
                for (int j = l; j <= r; j++) edge(u, j, w);
            }
            if (kind == 2)
            {
                sg.add_r2p(l, r, id[v], w); se.add_r2p(l, r, id[v]);
                for (int j = l; j <= r; j++) edge(j, v, w);
            }
            if (kind == 3)
            {
                sg.add_r2r(l, r, a, b, w); se.add_r2r(l, r, a, b); extra++;
                for (int i = l; i <= r; i++) for (int j = a; j <= b; j++) edge(i, j, w);
            }
            if (kind >= 4)
            {
                int p = kind == 4 ? sg.add_r2new(l, r, w) : sg.add_p2new(id[u], w);
                int p2 = kind == 4 ? se.add_r2new(l, r) : se.add_p2new(id[u]);
                assert(p == p2 && p == 3 * n - 2 + ++extra);
                int naive = id.size(); id.push_back(p);
                if (kind == 4) { for (int i = l; i <= r; i++) edge(i, naive, w); }
                else edge(u, naive, w);
            }
            assert(sg.tot == 3 * n - 2 + extra && se.tot == sg.tot);
        }
        assert(sg.g.edges.size() == se.g.edges.size() && sg.g.head == se.g.head);
        for (size_t i = 0; i < sg.g.edges.size(); i++)
            assert(sg.g.edges[i].v == se.g.edges[i].v && sg.g.edges[i].nxt == se.g.edges[i].nxt);
        int k = id.size() - 1;
        for (int mid = 1; mid <= k; mid++) for (int u = 1; u <= k; u++) for (int v = 1; v <= k; v++)
            if (d[u][mid] != INF && d[mid][v] != INF) d[u][v] = min(d[u][v], d[u][mid] + d[mid][v]);
        scc.init(sg.tot); scc.build(sg.g, sg.tot);
        for (int u = 1; u <= k; u++)
        {
            dij.init(sg.tot); dij.run(id[u], sg.g);
            VI seen(se.tot + 1), que{id[u]}; seen[id[u]] = 1;
            for (size_t i = 0; i < que.size(); i++) for (auto& e : se.g[que[i]])
                if (!seen[e.v]) { seen[e.v] = 1; que.push_back(e.v); }
            for (int v = 1; v <= k; v++)
            {
                assert(dij.dist[id[v]] == d[u][v]);
                assert(bool(seen[id[v]]) == (d[u][v] != INF));
                assert((scc.bel[id[u]] == scc.bel[id[v]]) == (d[u][v] != INF && d[v][u] != INF));
            }
        }
    }
    sg.build(3);
    sg.add_p2r(1, 2, 2, INF - 2); sg.add_r2p(2, 2, 3, 1);
    dij.init(sg.tot); dij.run(1, sg.g);
    assert(dij.dist[2] == INF - 2 && dij.dist[3] == INF - 1);
}

// 目标规模: 区间长度接近 n, 朴素展开需 O(nq), 答案由有向星结构直接给出
void test_large()
{
    const int cap = 200000;
    SegGraph<LL> sg(cap, 0);
    Dijkstra dij(4 * cap + 20);
    for (int n : {cap, 1, 2, cap - 1, cap})
    {
        sg.build(n);
        assert(sg.tot == 3 * n - 2 && sg.g.edge_cnt() == 4 * n - 4);
        for (int i = 0; i < cap; i++) sg.add_p2r(1, 1, n, i % 17);
        dij.init(sg.tot); dij.run(1, sg.g);
        for (int u = 1; u <= n; u++) assert(dij.dist[u] == 0);
        dij.init(sg.tot); dij.run(n, sg.g);
        for (int u = 1; u <= n; u++) assert(dij.dist[u] == (u == n ? 0 : INF));
        sg.build(n);
        for (int i = 0; i < cap; i++) sg.add_r2p(1, n, 1, 1000000000000LL + i % 17);
        dij.init(sg.tot); dij.run(n, sg.g);
        for (int u = 1; u <= n; u++)
            assert(dij.dist[u] == (u == n ? 0 : u == 1 ? 1000000000000LL : INF));
        sg.build(n);
        // 各 32 点块内全互达, 块间不连; 检查相互重叠区间和大量中继
        for (int l = 1; l <= n; l += 32) sg.add_r2r(l, min(n, l + 31), l, min(n, l + 31), 0);
        SCC scc(sg.tot); scc.build(sg.g, sg.tot);
        VI owner(scc.scc_cnt + 1, -1);
        for (int u = 1; u <= n; u++)
        {
            int block = (u - 1) / 32;
            assert(scc.bel[u] == scc.bel[block * 32 + 1]);
            if (u % 32 == 1) { assert(owner[scc.bel[u]] == -1); owner[scc.bel[u]] = block; }
        }
        sg.build(n);
        // 非对齐区间使每次分解出多个覆盖结点; p2p 保留直接路径
        for (int i = 0; i < cap; i++)
        {
            int l = 1 + i % n, r = min(n, l + n / 2);
            sg.add_p2r(1, l, r, 1); sg.add_p2p(1, l, 0);
        }
        dij.init(sg.tot); dij.run(1, sg.g);
        for (int u = 1; u <= n; u++) assert(dij.dist[u] == 0);
    }
}

void test_relay_budget()
{
    const int q = 200000;
    SegGraph<LL> sg(2, 0, q);
    SegGraph<Empty> se(2, 0, q);
    Dijkstra dij(q + 20);
    for (int n : {2, 1, 2})
    {
        sg.build(n); se.build(n);
        int previous = 1;
        VI relay;
        for (int i = 0; i < q; i++)
        {
            int id, id2;
            if (i % 3 == 0) { id = sg.add_p2new(previous, 1); id2 = se.add_p2new(previous); }
            else if (i % 3 == 1) { id = sg.add_r2new(1, n, 7); id2 = se.add_r2new(1, n); }
            else { sg.add_r2r(1, n, 1, n, 9); se.add_r2r(1, n, 1, n); continue; }
            assert(id == id2 && id == 3 * n - 2 + i + 1);
            relay.push_back(id); previous = id;
        }
        assert(sg.tot == 3 * n - 2 + q);
        dij.init(sg.tot); dij.run(1, sg.g);
        for (int id : relay)
        {
            int op = id - (3 * n - 2) - 1;
            assert(dij.dist[id] == (op == 0 ? 1 : op % 3 == 0 ? 8 : 7));
        }
        assert(dij.dist[n] == (n == 1 ? 0 : 9));
        VI seen(se.tot + 1), que{1}; seen[1] = 1;
        for (size_t i = 0; i < que.size(); i++) for (auto& e : se.g[que[i]])
            if (!seen[e.v]) { seen[e.v] = 1; que.push_back(e.v); }
        for (int id : relay) assert(seen[id]);
        assert(seen[n]);
    }
    SegGraph<LL> zero(2, 0, 0);
    zero.build(2); zero.add_p2r(1, 1, 2, 1);
    assert(zero.tot == 4);
    SegGraph<LL> legacy(2, 0);
    legacy.build(2); legacy.add_p2new(1); legacy.add_r2new(1, 2);
    assert(legacy.tot == 6);
}

void test_negative_dag()
{
    SegGraph<LL> sg(16, 0, 2); sg.build(16);
    sg.add_p2r(1, 9, 16, -5); sg.add_r2p(1, 8, 16, -7);
    sg.add_r2r(1, 8, 9, 16, -11);
    int relay = sg.add_r2new(1, 8, -3); sg.add_p2r(relay, 9, 16, -13);
    // 此图无环, 按边反复松弛, 不将负权图传给 Dijkstra
    for (int source : {1, 8, 9, relay})
    {
        VLL d(sg.tot + 1, INF); d[source] = 0;
        for (int pass = 0; pass < sg.tot; pass++) for (int u = 1; u <= sg.tot; u++) if (d[u] != INF)
            for (auto& e : sg.g[u]) d[e.v] = min(d[e.v], d[u] + e.w);
        for (int u = 1; u <= 16; u++)
        {
            LL expected = u == source ? 0 : (u >= 9 && source != 9 ? (source == relay ? -13 : -16) : INF);
            assert(d[u] == expected);
        }
    }
}

int main()
{
    test_seg_graph();
    test_independent_small();
    test_large();
    test_relay_budget();
    test_negative_dag();
    cout << "seggraph_check passed\n";
    return 0;
}
