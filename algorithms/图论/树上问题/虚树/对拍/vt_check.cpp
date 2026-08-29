// ============ vt_check 虚树双实现回归 ============
// 覆盖: 二次排序(VirtualTree) / 单调栈(VirtualTreeStack) vs 独立暴力
//       (关键点 LCA 闭包 + 最近粗先代父边, 权=跳数); 双实现互拍;
//       空关键点集 | keys 含根 | 孤立点森林 | 二次排序跨分量 rt 守卫
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

        // 暴力期望虚树: V' = keys ∪ {1} ∪ 两两 lca 闭包; 每点连向 V' 内最近粗先代
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

        // 二次排序(会改写 nodes, 传副本)
        VI nodes1 = keys;
        vt1.build(nodes1, lca);
        assert(collect_edges(vt1.tree) == expect);

        // 单调栈
        VI nodes2 = keys;
        vt2.build(nodes2, lca);
        assert(collect_edges(vt2.tree) == expect);

        // 跨分量专项: 二次排序 rt 守卫 → 空树
        if (isolated_last)
        {
            VI bad = keys;
            bad.push_back(n);
            vt1.build(bad, lca);
            assert(collect_edges(vt1.tree).empty());
        }
    }
    cout << "vt_check passed: VirtualTree x2 all tests ok\n";
    return 0;
}