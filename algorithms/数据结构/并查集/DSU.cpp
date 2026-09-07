// zoi: dsu
#ifndef Z_OI_DSU
#define Z_OI_DSU

#include "../../杂项/utils/utils.cpp"

// 定向合并并查集, fa 存父亲, sz 仅在根处有效; 每点约 8B, 20 万点约 1.6MB
struct DSU
{
    int n;
    VI fa, sz;
    // 分配 max_n 个点, 初始每个点自成集合, 编号为 1..max_n
    // 时间: O(max_n) | 空间: O(max_n)
    DSU(int max_n = 0) :
        n(max_n), fa(max_n + 10), sz(max_n + 10, 1)
    {
        for (int i = 0; i <= n; i++)
            fa[i] = i;
    }
    // 在构造容量内重置为 n 个单点集合, n 可为 0
    // 时间: O(n) | 额外空间: O(1)
    void init(int _n)
    {
        n = _n;
        for (int i = 0; i <= n; i++)
        {
            fa[i] = i;
            sz[i] = 1;
        }
    }
    // 返回 x 的根, 并把沿途各点直接接到根
    // 时间: O(路径长度), 单次最坏 O(n) | 额外空间: O(1)
    int find(int x)
    {
        int r = x;
        while (fa[r] != r) r = fa[r];
        while (x != r)
        {
            int p = fa[x];
            fa[x] = r;
            x = p;
        }
        return r;
    }
    // 把 v 的根挂到 u 的根, 成功返回 true, 已连通返回 false
    // 时间: O(两条查根路径长度), 单次最坏 O(n) | 额外空间: O(1)
    bool merge(int u, int v)
    {
        int fu = find(u);
        int fv = find(v);
        if (fu == fv) return false;
        fa[fv] = fu;
        sz[fu] += sz[fv];
        return true;
    }
    // 返回 u 和 v 是否属于同一个集合
    // 时间: O(两条查根路径长度), 单次最坏 O(n) | 额外空间: O(1)
    bool same(int u, int v) { return find(u) == find(v); }
    // 返回 x 所在集合的点数
    // 时间: O(查根路径长度), 单次最坏 O(n) | 额外空间: O(1)
    int size(int x) { return sz[find(x)]; }
};
#endif

/*
Usage:
#include "dsu.h"
int main()
{
    DSU ds(5);
    ds.merge(1, 2);
    ds.merge(3, 2);              // 原集合 {1, 2} 的根挂到 3
    cout << ds.find(1) << ' ' << ds.size(2) << '\n'; // 3 3
    cout << ds.same(1, 4) << '\n'; // 0
    ds.init(3);                 // 多测复用, 恢复三个单点集合
    cout << ds.size(2) << '\n';  // 1
}
*/
