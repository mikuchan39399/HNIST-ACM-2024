// zoi: center
#ifndef Z_OI_TREE_CENTER
#define Z_OI_TREE_CENTER

#include "../../图的存储/Graph.cpp"
#include "../../../杂项/utils/utils.cpp"

// 求最小化最远距离的顶点; n>=1 的无向树, 非负整数边权(无权计 1), 路径和在 LL 内
// ecc 为最远距离, centers 为升序中心, radius 为半径; 零权时可多于两个中心
// end_u/end_v 为直径端点, diameter 为长度
template <class G>
struct TreeCenter
{
    VLL ecc;
    VI centers;
    int end_u, end_v, far;
    LL diameter, radius, far_dis;

    // 重建中心信息并返回半径, 单点为 0. 时空 O(n), 递归深度最坏 n
    LL build(G& g, int n)
    {
        ecc.assign(n + 1, 0);
        centers.clear();
        far_dis = -1;
        dfs(1, 0, 0, g);
        end_u = far;
        far_dis = -1;
        dfs(end_u, 0, 0, g);
        end_v = far;
        diameter = far_dis;
        dfs(end_v, 0, 0, g, true);
        radius = diameter;
        for (int u = 1; u <= n; u++)
        {
            if (ecc[u] < radius) radius = ecc[u], centers = {u};
            else if (ecc[u] == radius) centers.push_back(u);
        }
        return radius;
    }
private:
    void dfs(int u, int p, LL d, G& g, bool merge = false)
    {
        ecc[u] = merge ? max(ecc[u], d) : d;
        if (d > far_dis) far_dis = d, far = u;
        for (auto& e : g[u])
        {
            if (e.v == p) continue;
            LL w = 1;
            if constexpr (!is_same_v<decltype(e.w), Empty>) w = e.w;
            dfs(e.v, u, d + w, g, merge);
        }
    }
};
#endif

/* Usage
Graph<false> g(4, 3);
g.add(1, 2); g.add(2, 3); g.add(3, 4);
TreeCenter<Graph<false>> tc;
cout << tc.build(g, 4) << '\n';       // 半径 2
for (int u : tc.centers) cout << u << ' '; // 中心 2, 3
cout << tc.ecc[1] << '\n';            // 点 1 的最远距离为 3
Graph<false, LL> wg(2, 1);
wg.add(1, 2, 10);
TreeCenter<Graph<false, LL>> wc;
cout << wc.build(wg, 2) << '\n';      // 顶点半径 10, 不返回边内部的中点
*/
