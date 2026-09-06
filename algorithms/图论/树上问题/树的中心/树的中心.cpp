// zoi: center
#ifndef Z_OI_TREE_CENTER
#define Z_OI_TREE_CENTER

#include "../../图的存储/Graph.cpp"
#include "../../../杂项/utils/utils.cpp"

// 求到最远点距离最小的全部顶点, 输入为 n >= 1 的无向连通树
// 无权边按 1 计权, 整数边权须非负且路径权和不溢出 LL; 零权下中心可能超过两个
// ecc 为每点的最远距离, centers 按编号升序存中心, radius 为最小最远距离
// end_u/end_v 为一条直径的端点, diameter 为其长度; build 自带复位
// ecc 每点 8 B, 答案每点 4 B; n = 1e6 时有效元素合计至多约 12 MB, 递归栈另计
template <class G>
struct TreeCenter
{
    VLL ecc;
    VI centers;
    int end_u, end_v, far;
    LL diameter, radius, far_dis;

    // 重建 g 的中心信息并返回半径, 全部中心写入 centers, 单点树半径为 0
    // 时间 O(n) | 空间 O(n), 含递归栈
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
