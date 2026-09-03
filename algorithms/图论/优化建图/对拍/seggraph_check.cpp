// ============ seggraph_check 线段树优化建图 回归套件 ============
// 覆盖: SegGraph 三型连边(add_edge/add_out/add_in)建图跑 Dijkstra,
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
    static SegGraph sg{41, 4000};
    static Dijkstra d1{170};
    static Dijkstra d2{41};
    static Graph<true, LL> gn{41, 4000};
    for (int tc = 0; tc < 300; tc++)
    {
        int n = 1 + rng() % 40, q = rng() % 60;
        gn.clear();
        sg.build(n);
        for (int i = 0; i < q; i++)
        {
            int l = 1 + rng() % n, r = 1 + rng() % n;
            if (l > r) swap(l, r);
            int u = 1 + rng() % n, v = 1 + rng() % n;
            LL w = rng() % 21;
            int op = rng() % 3;
            if (op == 0)
            {
                sg.add_edge(u, v, w);
                gn.add(u, v, w);
            }
            else if (op == 1)
            {
                sg.add_out(u, l, r, w);
                for (int k = l; k <= r; k++) gn.add(u, k, w);
            }
            else
            {
                sg.add_in(l, r, v, w);
                for (int k = l; k <= r; k++) gn.add(k, v, w);
            }
        }
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
