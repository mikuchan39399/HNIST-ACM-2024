// zoi: dsu
#ifndef Z_OI_DSU
#define Z_OI_DSU

#include "../../杂项/utils/utils.cpp"

// 并查集, 1..n; 定向合并, sz 仅根有效. 空间 O(n); h=查根路径长, 最坏 n
struct DSU
{
    int n;
    VI fa, sz;
    // 预留 max_n 个点并初始化. 时空 O(max_n)
    DSU(int max_n = 0) :
        n(max_n), fa(max_n + 10), sz(max_n + 10, 1)
    {
        for (int i = 0; i <= n; i++)
            fa[i] = i;
    }
    // 重置为 n 个单点集合, n<=容量. 时间 O(n)
    void init(int _n)
    {
        n = _n;
        for (int i = 0; i <= n; i++)
        {
            fa[i] = i;
            sz[i] = 1;
        }
    }
    // 查根并压缩路径. 时间 O(h)
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
    // 将 v 的根接到 u 的根, 返回是否发生合并. 时间 O(h)
    bool merge(int u, int v)
    {
        int fu = find(u);
        int fv = find(v);
        if (fu == fv) return false;
        fa[fv] = fu;
        sz[fu] += sz[fv];
        return true;
    }
    // 是否同属一个集合. 时间 O(h)
    bool same(int u, int v) { return find(u) == find(v); }
    // 所在集合大小. 时间 O(h)
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
