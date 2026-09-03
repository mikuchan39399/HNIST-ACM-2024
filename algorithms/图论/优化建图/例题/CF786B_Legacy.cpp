// CF786B Legacy —— 线段树优化建图组装范例
// 题意: n 点 q 操作, 源 s, 三型连边后问 1..n 各点最短路
//   op1 v u w: v->u        op2 v l r w: v->[l,r]      op3 v l r w: [l,r]->v
// 优化建图: 区间边 O(n) 条压成 O(log n) 条, 总边数 O(n + q log n)
// 内存: 结点 3n-2 + 边 4n + 34q; n=q=1e5 ≈ 67MB
#include <iostream>
#include "../线段树优化建图.cpp"
#include "../../最短路问题/单源最短路径/dijkstra_heap.cpp"

using namespace std;

int main()
{
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n, q, s;
    cin >> n >> q >> s;
    SegGraph sg{n, 8 * n + 34 * q};   // 边预算公式见引擎头注释
    sg.build(n);                      // 两棵骨架树, 树边 0 权
    while (q--)
    {
        int op;
        cin >> op;
        if (op == 1)
        {
            int v, u;
            LL w;
            cin >> v >> u >> w;
            sg.add_edge(v, u, w);     // 普通边, 与骨架无关
        }
        else if (op == 2)
        {
            int v, l, r;
            LL w;
            cin >> v >> l >> r >> w;
            sg.add_p2r(v, l, r, w);   // v -> 区间: 进出树覆盖点顺骨架下漏
        }
        else
        {
            int v, l, r;
            LL w;
            cin >> v >> l >> r >> w;
            sg.add_r2p(l, r, v, w);    // 区间 -> v: 入树叶子汇流上覆盖点再出去
        }
    }
    Dijkstra dij{sg.tot};
    dij.init(sg.tot);                 // dist 池要盖住树内点
    dij.run(s, sg.g);                 // 直接吃 SegGraph 产出的图
    for (int i = 1; i <= n; i++)
        cout << dij.dist[i] << " \n"[i == n];
    return 0;
}
