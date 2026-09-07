// zoi: wdsu
#ifndef Z_OI_WDSU
#define Z_OI_WDSU

#include "../../杂项/utils/utils.cpp"

// 定向带权并查集, d[x] 存 x 到 fa[x] 的距离, 根处为 0
// mod=0 存普通距离, mod>0 存 [0, mod) 的余数; 距离加减中间值须在 LL 内
// 每点约 12B, 20 万点约 2.4MB
struct WDSU
{
    static constexpr LL INF = ::INF;
    LL mod;
    VI fa;
    VLL d;
    // 分配 n 个单点集合, 编号为 1..n, mod_ 须非负
    // 时间: O(n) | 空间: O(n)
    WDSU(int n = 0, LL mod_ = 0) : fa(n + 1), d(n + 1)
    {
        init(n, mod_);
    }
    // 在构造容量内重置为 n 个单点集合, 省略 mod_ 时切回普通距离模式
    // 时间: O(n) | 额外空间: O(1)
    void init(int n, LL mod_ = 0)
    {
        mod = mod_;
        assert(mod >= 0);
        iota(fa.begin(), fa.begin() + n + 1, 0);
        fill(d.begin(), d.begin() + n + 1, 0);
    }
    // 返回 x 的根并压缩整条路径, 更新 d[x] 为 x 到根的距离
    // 时间: O(路径长度), 单次最坏 O(n) | 额外空间: O(1)
    int find(int x)
    {
        int r = x;
        LL sum = 0;
        while (fa[r] != r)
        {
            sum = norm(sum + d[r]);
            r = fa[r];
        }
        while (x != r)
        {
            int p = fa[x];
            LL w = d[x];
            fa[x] = r;
            d[x] = sum;
            sum = norm(sum - w);
            x = p;
        }
        return r;
    }
    // 声明 x 到 y 的距离为 w, 把 y 的根挂到 x 的根; 已连通则忽略, 不判矛盾
    // 时间: O(两条查根路径长度), 单次最坏 O(n) | 额外空间: O(1)
    void merge(int x, int y, LL w)
    {
        int fx = find(x), fy = find(y);
        if (fx == fy) return;
        d[fy] = norm(d[x] - w - d[y]);
        fa[fy] = fx;
    }
    // 返回 x 和 y 是否属于同一个集合
    // 时间: O(两条查根路径长度), 单次最坏 O(n) | 额外空间: O(1)
    bool same(int x, int y) { return find(x) == find(y); }
    // 返回 x 到 y 的距离, 未连通返回 INF; 合法距离也可能等于 INF, 用 same 区分
    // 时间: O(两条查根路径长度), 单次最坏 O(n) | 额外空间: O(1)
    LL query(int x, int y)
    {
        if (find(x) != find(y)) return INF;
        return norm(d[x] - d[y]);
    }
private:
    LL norm(LL v) const
    {
        if (!mod) return v;
        v %= mod;
        return v < 0 ? v + mod : v;
    }
};
#endif

/*
Usage:
#include "wdsu.h"
int main()
{
    WDSU ds(5);
    ds.merge(1, 2, 7);
    ds.merge(2, 3, -2);
    if (ds.same(1, 3)) cout << ds.query(1, 3) << '\n'; // 5
    ds.find(3);
    cout << ds.d[3] << '\n';     // -5, 点 3 到当前根 1

    ds.init(3, 3);              // 切换为三类关系
    ds.merge(1, 2, 1);
    ds.merge(2, 3, 1);
    cout << ds.query(1, 3) << '\n'; // 2
    LL w = 0;
    bool conflict = ds.same(1, 3) && ds.query(1, 3) != w;
    cout << conflict << '\n';   // 1, 调用方自行判矛盾
    ds.init(3);                 // 清空并切回普通距离模式
}
*/
