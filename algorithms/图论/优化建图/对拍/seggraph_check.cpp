// ============ seggraph_check 线段树优化建图 回归套件 ============
// 覆盖: SegGraph 六型连边(p2p/p2r/r2p/r2r/r2new 手动中继/p2new 单点起链)建图跑 Dijkstra + Empty 同拓扑互拍,
//       对拍朴素全边展开图(README 钦定的暴力参照系); 非负权;
//       static 实例跨轮 build 复用(多测路径, 内部自动清图)
// 纪律: 改动 线段树优化建图 / Dijkstra / Graph 模板, 必重跑本套件
// 跑法: g++ -std=c++20 -Wall -Wextra -O2 seggraph_check.cpp -o seggraph_check && ./seggraph_check
#include <iostream>
#include <vector>
#include <cassert>
#include <random>
#include <algorithm>
#include "../线段树优化建图.cpp"
#include "../../最短路问题/单源最短路径/dijkstra_heap.cpp"

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

int main()
{
    test_seg_graph();
    cout << "seggraph_check passed: SegGraph vs naive-edge Dijkstra all tests ok\n";
    return 0;
}
