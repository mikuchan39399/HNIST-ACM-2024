// zoi: wdsu
#ifndef Z_OI_WDSU
#define Z_OI_WDSU

#include "../../杂项/utils/utils.cpp"

// 带权并查集, 1..n; d[x]=x 到父亲的距离. 空间 O(n); h=查根路径长, 最坏 n
// mod=0 为普通距离, mod>0 为模距离; 加减中间值须在 LL 内
struct WDSU
{
    static constexpr LL INF = ::INF;
    LL mod;
    VI fa;
    VLL d;
    // 预留并初始化 n 个单点集合, mod>=0. 时空 O(n)
    WDSU(int n = 0, LL mod_ = 0) : fa(n + 1), d(n + 1)
    {
        init(n, mod_);
    }
    // 重置 n 个点及模数(默认 0), n<=容量. 时间 O(n)
    void init(int n, LL mod_ = 0)
    {
        mod = mod_;
        assert(mod >= 0);
        iota(fa.begin(), fa.begin() + n + 1, 0);
        fill(d.begin(), d.begin() + n + 1, 0);
    }
    // 查根并压缩路径, d[x] 更新为到根的距离. 时间 O(h)
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
    // 加入 x 到 y 距离为 w 的约束; 同集合时忽略, 不检查矛盾. 时间 O(h)
    void merge(int x, int y, LL w)
    {
        int fx = find(x), fy = find(y);
        if (fx == fy) return;
        d[fy] = norm(d[x] - w - d[y]);
        fa[fy] = fx;
    }
    // 是否连通. 时间 O(h)
    bool same(int x, int y) { return find(x) == find(y); }
    // x 到 y 的距离, 不连通返回 INF; 可能与合法距离重合, 先 same. 时间 O(h)
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
