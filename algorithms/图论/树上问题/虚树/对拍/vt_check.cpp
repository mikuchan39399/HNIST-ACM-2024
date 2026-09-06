// ============ vt_check 虚树双实现回归 ============
// 覆盖: 二次排序(VirtualTree) / 单调栈(VirtualTreeStack) vs 独立暴力
//       (关键点 LCA 闭包 + 最近祖先父边); 双实现互拍
//       300 组原无权测试 + 300 组带权森林 + 确定性边界, mt19937(42)
//       重复点 | 任意 root | 输入不变 | 空集/单点 | 跨分量清空 | clear/build 复用
// 纪律: 改动 二次排序/单调栈/DFN_LCA/Graph, 必重跑本套件
// 跑法: g++ -std=c++20 -Wall -Wextra -O2 vt_check.cpp -o vt_check && ./vt_check
#include <iostream>
#include <vector>
#include <cassert>
#include <algorithm>
#include <set>
#include <tuple>
#include <random>
#include "../二次排序.cpp"
#include "../../最近公共祖先/DFN_LCA.cpp"
#include "../单调栈.cpp"

using namespace std;

using T3 = tuple<int, int, LL>;   // (min(u,v), max(u,v), w) 规范边

// 从 Graph<false, LL> 收集规范边集(每条逻辑边取偶数下标半边)
static vector<T3> collect_edges(Graph<false, LL>& g)
{
    vector<T3> res;
    for (size_t i = 0; i + 1 < g.edges.size(); i += 2)
    {
        int u = g.edges[g.rev(i)].v;
        int v = g.edges[i].v;
        LL w = g.edges[i].w;
        if (u > v) swap(u, v);
        res.push_back({u, v, w});
    }
    sort(res.begin(), res.end());
    return res;
}

// 从原始边表独立建父表, 不使用被测 LCA 的查询或成员
static void check_case(int n, const vector<T3>& edges, const VI& keys, int root)
{
    static LCA lca(64);
    static VirtualTree vt1(64);
    static VirtualTreeStack vt2(64);
    Graph<false, LL> g(n, edges.size());
    vector<vector<pair<int, LL>>> adj(n + 1);
    for (auto [u, v, w] : edges)
    {
        g.add(u, v, w);
        adj[u].push_back({v, w});
        adj[v].push_back({u, w});
    }
    VI par(n + 1), dep(n + 1), rt(n + 1);
    VLL dis(n + 1);
    auto dfs = [&](auto&& self, int u, int p, int r) -> void
    {
        par[u] = p;
        rt[u] = r;
        for (auto [v, w] : adj[u])
            if (v != p)
            {
                dep[v] = dep[u] + 1;
                dis[v] = dis[u] + w;
                self(self, v, u, r);
            }
    };
    for (int u = 1; u <= n; u++)
        if (!rt[u]) dfs(dfs, u, 0, u);
    auto lca_b = [&](int a, int b)
    {
        while (dep[a] > dep[b]) a = par[a];
        while (dep[b] > dep[a]) b = par[b];
        while (a != b) { a = par[a]; b = par[b]; }
        return a;
    };
    set<int> vs;
    bool valid = !keys.empty();
    for (int u : keys) valid = valid && rt[u] == rt[root];
    if (valid)
    {
        vs.insert(root);
        vs.insert(keys.begin(), keys.end());
        VI vv(vs.begin(), vs.end());
        for (int u : vv)
            for (int v : vv) vs.insert(lca_b(u, v));
    }
    vector<T3> expect;
    for (int v : vs)
    {
        int p = par[v];
        while (p && !vs.count(p)) p = par[p];
        if (p) expect.push_back({min(p, v), max(p, v), dis[v] - dis[p]});
    }
    sort(expect.begin(), expect.end());
    lca.init(n);
    lca.build(g);
    auto verify = [&](auto& vt)
    {
        VI input = keys;
        vt.build(input, lca, root);
        assert(input == keys);
        assert(collect_edges(vt.tree) == expect);
        VI used = vt.tree.used;
        sort(used.begin(), used.end());
        VI expected_used;
        if (vs.size() > 1) expected_used.assign(vs.begin(), vs.end());
        assert(used == expected_used);
        vt.clear();
        assert(vt.tree.edges.empty() && vt.tree.used.empty());
        for (int u = 1; u <= n; u++)
            assert(vt.tree.head[u] == -1 && vt.tree.deg[u] == 0);
        vt.build(keys, lca, root); // const 入参, 留给下一组检验 build 自带复位
        assert(collect_edges(vt.tree) == expect);
    };
    verify(vt1);
    verify(vt2);
}

int main()
{
    mt19937 rng(42);
    static LCA lca(20);
    static VirtualTree vt1(24);
    static VirtualTreeStack vt2(24);
    for (int tc = 0; tc < 300; tc++)
    {
        int n = 1 + rng() % 12;
        VI par(n + 1, 0);
        Graph<false, Empty> g(n, n);
        for (int i = 2; i <= n; i++)
        {
            if (i == n && n >= 3 && rng() % 6 == 0) continue;   // 偶发孤立点(森林)
            par[i] = 1 + rng() % (i - 1);
            g.add(i, par[i]);
        }
        lca.init(n);
        lca.build(g);

        // 独立暴力: 深度 + 朴素 lca(爬父链)
        VI dep(n + 1, 0);
        for (int i = 2; i <= n; i++) dep[i] = dep[par[i]] + 1;
        auto lca_b = [&](int a, int b)
        {
            while (dep[a] > dep[b]) a = par[a];
            while (dep[b] > dep[a]) b = par[b];
            while (a != b)
            {
                a = par[a];
                b = par[b];
            }
            return a;
        };

        // 关键点集(只取 root=1 分量; 孤立点留给跨分量专项)
        bool isolated_last = (n >= 2 && par[n] == 0);
        VI keys;
        for (int v = 1; v <= n; v++)
        {
            bool in_root = !(v == n && isolated_last);
            if (in_root && rng() % 10 < 3) keys.push_back(v);
        }

        // 暴力期望虚树: V' = keys ∪ {1} ∪ 两两 lca 闭包; 每点连向 V' 内最近祖先
        set<int> vs;
        vs.insert(1);
        for (int x : keys) vs.insert(x);
        vector<int> vv(vs.begin(), vs.end());
        for (size_t i = 0; i < vv.size(); i++)
            for (size_t j = i + 1; j < vv.size(); j++)
                vs.insert(lca_b(vv[i], vv[j]));
        vector<T3> expect;
        for (int v : vs)
        {
            if (v == 1) continue;
            int p = par[v];
            while (!vs.count(p)) p = par[p];   // 1 ∈ V' 保证终止
            expect.push_back({min(p, v), max(p, v), (LL)(dep[v] - dep[p])});
        }
        sort(expect.begin(), expect.end());

        // 二次排序
        VI nodes1 = keys;
        vt1.build(nodes1, lca);
        assert(nodes1 == keys);
        assert(collect_edges(vt1.tree) == expect);

        // 单调栈
        VI nodes2 = keys;
        vt2.build(nodes2, lca);
        assert(nodes2 == keys);
        assert(collect_edges(vt2.tree) == expect);

        // 跨分量专项: 两版都清空
        if (isolated_last)
        {
            VI bad = keys;
            bad.push_back(n);
            vt1.build(bad, lca);
            assert(collect_edges(vt1.tree).empty());
            vt2.build(bad, lca);
            assert(collect_edges(vt2.tree).empty());
        }
    }

    check_case(2, {{1, 2, 3}}, {2, 2}, 1); // 旧栈版重复点会生成 2-2 自环
    check_case(2, {{1, 2, 3}}, {1}, 2);    // root 非祖先, 旧栈版会生成 1-1 自环
    check_case(4, {{1, 2, 3}, {2, 3, 5}, {2, 4, 7}}, {4, 3, 4}, 4);
    check_case(4, {{1, 2, -3}, {2, 3, 0}, {2, 4, 1000000000000LL}}, {3, 4}, 1);
    check_case(3, {{1, 2, 1}}, {2, 3}, 1); // 跨分量, 清掉上组非空树
    check_case(1, {}, {1, 1}, 1);
    check_case(1, {}, {}, 1);
    vector<T3> chain, star;
    VI all;
    for (int u = 1; u <= 64; u++)
    {
        all.push_back(u);
        if (u > 1)
        {
            chain.push_back({u - 1, u, u % 3 - 1});
            star.push_back({1, u, u});
        }
    }
    check_case(64, chain, {64, 1, 64}, 32);
    check_case(1, {}, {1}, 1);
    check_case(64, star, all, 64); // 大-小-大复用及全点集

    mt19937 weighted_rng(42);
    for (int tc = 0; tc < 300; tc++)
    {
        int n = 1 + weighted_rng() % 24;
        VI labels;
        for (int u = 1; u <= n; u++) labels.push_back(u);
        shuffle(labels.begin(), labels.end(), weighted_rng);
        vector<T3> edges;
        for (int i = 1; i < n; i++)
            if (weighted_rng() % 5)
                edges.push_back({labels[i], labels[weighted_rng() % i], (int)(weighted_rng() % 21) - 10});
        int root = 1 + weighted_rng() % n;
        VI keys;
        int k = weighted_rng() % (2 * n + 1);
        for (int i = 0; i < k; i++) keys.push_back(1 + weighted_rng() % n);
        check_case(n, edges, keys, root);
        // 全部点都在 root 分量, 避免随机跨树把大部分样例变成空树
        VI component{root};
        for (size_t i = 0; i < component.size(); i++)
            for (auto [u, v, w] : edges)
            {
                (void)w;
                int x = component[i] == u ? v : component[i] == v ? u : 0;
                if (x && find(component.begin(), component.end(), x) == component.end()) component.push_back(x);
            }
        keys.clear();
        for (int i = 0; i < k; i++) keys.push_back(component[weighted_rng() % component.size()]);
        check_case(n, edges, keys, root);
    }
    cout << "vt_check passed: VirtualTree x2 all tests ok\n";
    return 0;
}
